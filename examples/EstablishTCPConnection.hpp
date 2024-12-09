#pragma once

#include <map>
#include <cstdint>
#include <thread>

#include <tcpp/TunDevice.hpp>
#include <tcpp/structs/IPv4.hpp>
#include <tcpp/structs/TCP.hpp>

struct ConnectionID {
    // Our IP address is always the same, no need to store it.
    uint32_t ip = 0xFFFFFFFF;  // IP address of the other end
    uint16_t source_port = 0xFFFF;
    uint16_t dest_port = 0xFFFF;

    operator uint64_t() const {
        return static_cast<uint64_t>(ip) << 32 |
               static_cast<uint64_t>(source_port) << 16 |
               static_cast<uint64_t>(dest_port);
    }
};

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

struct TCB {

    SendSequenceSpace send { };
    ReceiveSequenceSpace receive { };

    void send_packet(tcpp::structs::IPv4& ip, const tcpp::TunDevice& tun) {
        auto& tcp = ip.tcp_payload();
        tcp.set_seq_num(send.nxt + 1);
        tcp.set_window_size(send.wnd);
        ip.compute_and_set_ip_tcp_checksums();
        auto buffer = reinterpret_cast<uint8_t*>(&ip);
        (void)tun.send(std::span { buffer, ip.total_len() });
        const auto seq_increase =
            ip.total_len() - ip.payload_offset() - tcp.payload_offset() +  // TCP payload size
            (tcp.syn | tcp.fin);  // TODO only syn and fin?
        send.nxt += seq_increase;
    }

    void process_syn(const tcpp::structs::TCP& tcp) {
        send.iss = 0;
        send.una = send.iss;
        send.nxt = send.iss;
        send.wnd = 10;

        receive.irs = tcp.seq_num();
        receive.nxt = receive.irs + 1;
        receive.wnd = tcp.window_size();
    }

    void process_packet(tcpp::structs::IPv4& ip, const tcpp::TunDevice& tun) {
        auto& tcp = ip.tcp_payload();

        if (tcp.syn != true) return;

        process_syn(tcp);

        // Remove any options
        const auto old_offset = tcp.data_offset;
        tcp.data_offset = 5;
        ip.set_total_len(ip.total_len() - (old_offset - 5) * 4);

        std::swap(ip.source_addr_n, ip.dest_addr_n);
        std::swap(tcp.source_port_n, tcp.dest_port_n);
        tcp.ack = true;
        tcp.set_ack_num(receive.nxt);
        send_packet(ip, tun);

        // Wait for ack
        // TODO Process an ack instead of just sleeping
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        // Send message
        tcp.syn = false;
        ip.set_tcp_payload("Hello World!\n");
        send_packet(ip, tun);

        // Wait for ack
        // TODO Process an ack instead of just sleeping
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        // Send fin
        ip.set_tcp_payload("");
        tcp.ack = false;
        tcp.fin = true;
        send_packet(ip, tun);

        // Wait for fin
        // TODO Process a fin instead of just sleeping. This relies on
        //  the client to finish the connection about before 3 seconds
        std::this_thread::sleep_for(std::chrono::milliseconds(3000));

        // Send ack
        tcp.set_ack_num(receive.nxt + 1);
        tcp.ack = true;
        tcp.fin = false;
        send_packet(ip, tun);
    }
};

[[noreturn]] void example() {
    auto tun = tcpp::TunBuilder("TunDevice0")
        .set_ip4("10.0.0.1")
        .set_netmask("255.0.0.0")
        .build();

    std::map<ConnectionID, TCB> connections;
    std::array<uint8_t, 2048> buffer { };
    while (true) {
        // TODO consider SYN-flood attacks
        (void)tun.receive(buffer);
        auto& ip = tcpp::structs::IPv4::from_ptr(buffer.data());
        if (ip.version != 4 || ip.protocol != tcpp::structs::IPv4::IPPROTOCOL_TCP) continue;
        const auto& tcp = ip.tcp_payload();
        ConnectionID id { ip.source_addr_n, tcp.source_port_n, tcp.dest_port_n };
        connections[id].process_packet(ip, tun);
    }
}
