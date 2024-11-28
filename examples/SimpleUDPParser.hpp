#pragma once

#include <iostream>

#include <tcpp/TunDevice.hpp>
#include <tcpp/structs/IPv4.hpp>
#include <tcpp/structs/UDP.hpp>

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

        if (ip.protocol != tcpp::structs::IPv4::IPPROTOCOL_UDP) { std::cout << '\n'; continue; }

        auto& udp = ip.udp_payload();
        std::cout << "Ports " << udp.source_port() << " -> " << udp.dest_port() << '\n';
        std::cout << "Size: " << udp.payload_size() << '\n';
        std::cout << "Data:\n\"\n";
        for (auto r : udp.payload()) std::cout << r;
        std::cout << "\"\n\n";
    }
}