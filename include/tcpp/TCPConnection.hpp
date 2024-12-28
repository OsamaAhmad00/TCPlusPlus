#pragma once

#include <algorithm>

#include <tcpp/structs/IPv4.hpp>
#include <tcpp/structs/TCP.hpp>
#include <tcpp/data-structures/MPSCBoundedQueue.hpp>
#include <tcpp/allocators/ReusableSlabAllocator.hpp>

namespace tcpp {

template <size_t ConnectionBufferSize>
class TCPConnection {
    /*
     *  RFC 9293 - Section 3.3.1 - Figure 3
     *
     *             1         2          3          4
     *        ----------|----------|----------|----------
     *               SND.UNA    SND.NXT    SND.UNA
     *                                    +SND.WND
     *
     *  1 - old sequence numbers that have been acknowledged
     *  2 - sequence numbers of unacknowledged data
     *  3 - sequence numbers allowed for new data transmission
     *  4 - future sequence numbers that are not yet allowed
     *
     *  The send window is the portion of the sequence space labeled 3
     */
    struct SendSequenceSpace {
        uint32_t una;  // send unacknowledged
        uint32_t nxt;  // send next
        uint16_t wnd;  // send window
        uint8_t  up;   // send urgent pointer
        uint32_t wl1;  // segment sequence number used for last window update
        uint32_t wl2;  // segment acknowledgment number used for last window update
        uint32_t iss;  // initial send sequence number
    };

    /*
     *  RFC 9293 - Section 3.3.1 - Figure 4
     *
     *                 1          2          3
     *             ----------|----------|----------
     *                    RCV.NXT    RCV.NXT
     *                              +RCV.WND
     *
     *  1 - old sequence numbers that have been acknowledged
     *  2 - sequence numbers allowed for new reception
     *  3 - future sequence numbers that are not yet allowed
     *
     *  The receive window is the portion of the sequence space labeled 2
     */
    struct ReceiveSequenceSpace {
        uint32_t nxt;  // receive next
        uint32_t wnd;  // receive window
        uint8_t  up;   // receive urgent pointer
        uint32_t irs;  // initial receive sequence number
    };

    enum class State {
        New,
        SynRcvd,  // Syn Received
        SynSent,  // Syn Sent
        Estab,    // Connection Established

        WaitingForDataAck,  // TODO Delete this
        WaitingForFinAck,   // TODO Delete this
        FinAcked,           // TODO Delete this
    };

    void send_packet(structs::IPv4& ip) {
        auto& tcp = ip.tcp_payload();
        tcp.set_seq_num(send.nxt);
        tcp.set_ack_num(receive.nxt);
        tcp.set_window_size(send.wnd);
        if (!swapped) {
            swapped = true;
            std::swap(ip.source_addr_n, ip.dest_addr_n);
            std::swap(tcp.source_port_n, tcp.dest_port_n);
        }
        ip.compute_and_set_ip_tcp_checksums();
        ReusableAllocator alloc;
        auto buffer = alloc.allocate();
        std::copy_n(reinterpret_cast<uint8_t*>(&ip), ip.total_len(), buffer);
        send_queue.push(buffer);
        const auto seq_increase =
            ip.total_len() - ip.payload_offset() - tcp.payload_offset() +  // TCP payload size
            (tcp.syn | tcp.fin);
        send.nxt += seq_increase;
    }

    void process_syn(structs::IPv4& ip) {
        auto& tcp = ip.tcp_payload();

        send.iss = 0;
        send.una = send.iss;
        send.nxt = send.iss;
        send.wnd = static_cast<uint16_t>((1 << 16) - 1);

        receive.irs = tcp.seq_num();
        receive.nxt = receive.irs + 1;
        receive.wnd = tcp.window_size();

        // Remove any options
        const auto old_offset = tcp.data_offset;
        tcp.data_offset = 5;
        ip.set_total_len(ip.total_len() - (old_offset - 5) * 4);

        tcp.ack = true;
        send_packet(ip);

        state = State::SynRcvd;
    }

    void process_fin(structs::IPv4& ip) {
        // Send ack as a response to the fin
        auto& tcp = ip.tcp_payload();
        tcp.ack = true;
        tcp.fin = false;
        send_packet(ip);
        // TODO change this?
        connection_closed = true;
        connection_closed.notify_all();
    }

    void process_ack(structs::IPv4& ip) {
        auto& tcp = ip.tcp_payload();
        if (state == State::SynRcvd) {
            // Send message
            ip.set_tcp_payload("Hello World!\n");
            send_packet(ip);
            state = State::WaitingForDataAck;
        } else if (state == State::WaitingForDataAck) {
            // Send fin
            ip.set_tcp_payload("");
            bool fin = tcp.fin;
            tcp.fin = true;
            send_packet(ip);
            tcp.fin = fin;
            state = State::WaitingForFinAck;
        } else if (state == State::WaitingForFinAck) {
            state = State::FinAcked;
        } else if (state == State::FinAcked) {
            tcp.psh = false;
            ip.set_tcp_payload("");
            // Send ack
            send_packet(ip);
        }
    }

    void process_new(structs::IPv4& ip) {
        auto& tcp = ip.tcp_payload();
        // Ignore any non-syn packet for new connections
        if (tcp.syn != true) return;
        process_syn(ip);
    }

public:

    const ConnectionID id;

    TCPConnection(TCPConnection&) = delete;
    TCPConnection(TCPConnection&&) = delete;
    TCPConnection& operator=(TCPConnection&) = delete;
    TCPConnection& operator=(TCPConnection&&) = delete;

    friend auto operator<=>(const TCPConnection& lhs, const TCPConnection& rhs) {
        return lhs.id <=> rhs.id;
    }

    explicit TCPConnection(
        const ConnectionID id,
        MPSCBoundedQueue<PacketBuffer, ConnectionBufferSize>& send_queue
    ) : id(id), send_queue(send_queue)
    { }

    void process_packet(uint8_t* packet) {
        auto& ip = structs::IPv4::from_ptr(packet);

        swapped = false;

        auto& tcp = ip.tcp_payload();
        // TODO perform the actual checks
        if (tcp.ack == true && (tcp.ack_num() < send.nxt || tcp.seq_num() < receive.nxt)) {
            return;
        }

        if (state == State::New) {
            return process_new(ip);
        }

        // TODO use receive and send windows
        auto offset = ip.payload_offset() + tcp.payload_offset();
        auto payload_len = ip.total_len() - offset;
        auto payload = &ip.extract<uint8_t>(offset);
        for (size_t i = 0; i < payload_len; i++) {
            auto pushed = receive_buffer.push(payload[i]);
            assert(pushed);
        }

        receive.nxt += payload_len + (tcp.syn | tcp.fin);

        if (tcp.ack == true) {
            process_ack(ip);
        }

        if (tcp.fin == true) {
            process_fin(ip);
        }

        ReusableAllocator{}.deallocate(packet);
    }

    [[nodiscard]] size_t read(std::span<uint8_t> buffer) {
        // TODO change this
        size_t bytes_read = 0;
        for (uint8_t& b : buffer) {
            auto val = receive_buffer.pop();
            if (!val.has_value()) break;
            b = val.value();
            bytes_read++;
        }
        return bytes_read;
    }

    void close() {
        // TODO change this?
        connection_closed.wait(false);
    }

    ~TCPConnection() noexcept {
        close();
    }

private:

    State state = State::New;
    SendSequenceSpace send { };
    ReceiveSequenceSpace receive { };

    // TODO have a separate arg for the queue capacity
    MPSCBoundedQueue<PacketBuffer, ConnectionBufferSize>& send_queue;

    // TODO delete this
    bool swapped = false;

    // TODO change this
    static constexpr std::size_t BufferSize =  1 << 16;
    SPSCBoundedWaitFreeQueue<uint8_t, BufferSize> receive_buffer;

public:

    // TODO delete this?
    std::atomic<bool> connection_closed = false;
};

}
