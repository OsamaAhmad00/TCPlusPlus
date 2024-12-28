#pragma once

#include <iostream>

#include <tcpp/TCPInterface.hpp>
#include <tcpp/utils/IPv4.hpp>

using tcpp::operator ""_nip;

void example() {
    auto tun = tcpp::TunBuilder("TunDevice0")
        .set_ip4("10.0.0.1")
        .set_netmask("255.0.0.0")
        .build();

    auto tcp = tcpp::TCPInterface { (std::move(tun)) };
    auto& listener = tcp.bind({ "10.0.0.5"_nip, 4000 });
    auto& connection = listener.accept();
    std::array<uint8_t, 2048> buffer { };
    while (!connection.connection_closed) {
        auto n = connection.read(buffer);
        buffer[n] = '\0';
        std::cout << buffer.data();
    }
}
