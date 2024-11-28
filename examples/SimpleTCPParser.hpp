#pragma once

#include <iostream>

#include <tcpp/TunDevice.hpp>
#include <tcpp/structs/IPv4.hpp>
#include <tcpp/structs/TCP.hpp>

[[noreturn]] void example() {
    auto tun = tcpp::TunBuilder("TunDevice0")
        .set_ip4("10.0.0.1")
        .set_netmask("255.0.0.0")
        .build();

    std::array<uint8_t, 2048> buffer { };
    while (true) {
        const size_t n = tun.receive(buffer);
        std::cout << "Received " << n << " bytes from TunDevice0:\n";

        auto& ip = tcpp::structs::IPv4::from_ptr(buffer.data());
        if (ip.version != 4) { std::cout << '\n'; continue; }

        std::cout << "From " << ip.source_ip() << " to " << ip.dest_ip() << '\n';
        std::cout << "Protocol: " << +ip.protocol << '\n';

        if (ip.protocol != tcpp::structs::IPv4::IPPROTOCOL_TCP) { std::cout << '\n'; continue; }

        auto& tcp = ip.tcp_payload();
        auto payload_size = ip.payload_size() - tcp.payload_offset();
        std::cout << "Ports " << tcp.source_port() << " -> " << tcp.dest_port() << '\n';
        std::cout << "Sequence number: " << tcp.seq_num() << '\n';
        std::cout << "Acknowledgement number: " << tcp.ack_num() << '\n';
        std::cout << "urg: " << +tcp.urg << ", ack: " << +tcp.ack << ", psh: " << +tcp.psh <<
            ", rst: " << +tcp.rst << ", syn: " << tcp.syn << ", fin: " << tcp.fin << '\n';
        std::cout << "Window size: " << tcp.window_size() << '\n';
        std::cout << "Checksum: " << tcp.checksum() << '\n';
        std::cout << "Urgent Pointer: " << tcp.urgent_ptr() << '\n';
        std::cout << "Size: " << payload_size << '\n';
        std::cout << "Data:\n\"\n";
        for (auto r : tcp.payload(payload_size)) std::cout << r;
        std::cout << "\"\n\n";
    }
}