#pragma once

#include <deque>
#include <tcpp/TCPInterface.hpp>

void example() {
    auto tun = tcpp::TunBuilder("TunDevice0")
        .set_ip4("10.0.0.1")
        .set_netmask("255.0.0.0")
        .build();

    auto tcp = tcpp::TCPInterface { (std::move(tun)) };
    auto& listener = tcp.bind(4000);
    // TODO Remove the hardcoded number
    std::deque<tcpp::TCPConnection<1 << 20>> connections;
    int allowed_connections = 5;
    while (allowed_connections--) {
        listener.accept(connections);
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    listener.close();
}
