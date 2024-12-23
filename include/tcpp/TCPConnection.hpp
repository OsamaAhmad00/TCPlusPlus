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
        Closed,             // TODO Delete this
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
        connection_closed = true;
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
            state = State::Closed;
        }
    }

    void process_new(structs::IPv4& ip) {
        auto& tcp = ip.tcp_payload();
        // Ignore any non-syn packet for new connections
        if (tcp.syn != true) return;
        process_syn(ip);
    }

    void process_packet(structs::IPv4& ip) {
        auto& tcp = ip.tcp_payload();
        // TODO perform the actual checks
        if (tcp.ack == true && (tcp.ack_num() < send.nxt || tcp.seq_num() < receive.nxt)) {
            return;
        }

        if (state == State::New) {
            return process_new(ip);
        }

        receive.nxt += tcp.syn | tcp.fin;

        if (tcp.ack == true) {
            process_ack(ip);
        }

        if (tcp.fin == true) {
            process_fin(ip);
        }
    }

    void handler(std::stop_token token) {
        ReusableAllocator alloc;
        while (!token.stop_requested() && !connection_closed) {
            // TODO stop spinning
            auto packet = receive_queue.pop();
            if (packet.has_value()) {
                swapped = false;
                process_packet(structs::IPv4::from_ptr(packet.value()));
                // TODO don't deallocate and don't copy when sending
                alloc.deallocate(packet.value());
            }
        }
        connection_queues.remove_connection_queue(id);
    }

    // TODO use the state instead
    bool connection_closed = false;

    State state = State::New;
    SendSequenceSpace send { };
    ReceiveSequenceSpace receive { };

    ConnectionID id;
    // TODO have a separate arg for the queue capacity
    ConnectionQueues<ConnectionBufferSize>& connection_queues;
    SPSCBoundedWaitFreeQueue<PacketBuffer, ConnectionBufferSize>& receive_queue;
    MPSCBoundedQueue<PacketBuffer, ConnectionBufferSize>& send_queue;
    std::jthread packet_processor;

    // TODO delete this
    bool swapped = false;

public:

    TCPConnection(TCPConnection&) = delete;
    TCPConnection(TCPConnection&&) = delete;
    TCPConnection& operator=(TCPConnection&) = delete;
    TCPConnection& operator=(TCPConnection&&) = delete;

    explicit TCPConnection(
        const ConnectionID id,
        ConnectionQueues<ConnectionBufferSize>& connection_queues,
        SPSCBoundedWaitFreeQueue<PacketBuffer, ConnectionBufferSize>& receive_queue,
        MPSCBoundedQueue<PacketBuffer, ConnectionBufferSize>& send_queue
    ) : id(id),
        connection_queues(connection_queues),
        receive_queue(receive_queue),
        send_queue(send_queue),
        packet_processor(&TCPConnection::handler, this)
    { }
};

}
