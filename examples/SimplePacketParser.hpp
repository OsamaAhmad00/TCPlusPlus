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
        std::cout << ip.info() << "\n\n";
    }
}