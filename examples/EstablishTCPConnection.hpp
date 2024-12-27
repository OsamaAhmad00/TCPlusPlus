#pragma once

#include <vector>

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
    // TODO Remove the hardcoded number
    std::vector<tcpp::TCPConnection<1 << 20>*> connections;
    int allowed_connections = 5;
    while (allowed_connections--) {
        auto& connection = listener.accept();
        connections.push_back(&connection);
    }
    // TODO close connections;
    listener.close();
}
