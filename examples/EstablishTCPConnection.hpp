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

struct TCB {
    void process_packet(tcpp::structs::IPv4& ip, tcpp::TunDevice& tun) {
        auto& tcp = ip.tcp_payload();
        if (tcp.syn != true) return;

        std::swap(ip.source_addr_n, ip.dest_addr_n);
        std::swap(tcp.source_port_n, tcp.dest_port_n);
        tcp.ack = true;
        const auto ack_num = tcp.seq_num() + 1;
        tcp.set_ack_num(ack_num);
        tcp.set_seq_num(0);

        auto buffer = reinterpret_cast<uint8_t*>(&ip);

        ip.compute_and_set_ip_tcp_checksums();
        (void)tun.send(std::span { buffer, ip.total_len() });

        // Wait for ack
        // TODO Process an ack instead of just sleeping
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        // Send message
        tcp.set_seq_num(1);
        tcp.syn = false;
        // tcp.ack = false;
        uint8_t message[] = "Hello World!\n";
        auto size = static_cast<uint16_t>(sizeof(message));  // Ignore the null-terminator
        auto data_offset = ip.total_len();  // This is an empty packet
        for (uint16_t i = 0; i < size; i++)
            buffer[data_offset + i] = message[i];
        ip.set_total_len(ip.total_len() + size);

        ip.compute_and_set_ip_tcp_checksums();
        (void)tun.send(std::span { buffer, ip.total_len() });
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
        // TODO consider the SYN-flood attack
        (void)tun.receive(buffer);
        auto& ip = tcpp::structs::IPv4::from_ptr(buffer.data());
        if (ip.version != 4 || ip.protocol != tcpp::structs::IPv4::IPPROTOCOL_TCP) continue;
        const auto& tcp = ip.tcp_payload();
        ConnectionID id { ip.source_addr_n, tcp.source_port_n, tcp.dest_port_n };
        connections[id].process_packet(ip, tun);
    }
}
