#pragma once

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
        auto& ip = tcpp::structs::IPv4::from_ptr(buffer.data());

        if (ip.version != 4 || ip.protocol != tcpp::structs::IPv4::IPPROTOCOL_UDP) continue;

        auto& udp = ip.udp_payload();

        std::swap(ip.source_addr_n, ip.dest_addr_n);
        std::swap(udp.source_port_n, udp.dest_port_n);

        (void)tun.send(std::span { buffer.data(), n });
    }
}