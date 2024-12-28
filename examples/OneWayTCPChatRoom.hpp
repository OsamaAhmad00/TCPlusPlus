#pragma once

#include <iostream>

#include <tcpp/TCPInterface.hpp>
#include <tcpp/utils/IPv4.hpp>

using tcpp::operator ""_nip;

static std::string id_str(tcpp::ConnectionID id) {
    return tcpp::network_ip_to_string(id.source_ip) +  ':' + std::to_string(ntohs(id.source_port));
}

[[noreturn]] void example() {
    auto tun = tcpp::TunBuilder("TunDevice0")
        .set_ip4("10.0.0.1")
        .set_netmask("255.0.0.0")
        .build();

    auto tcp = tcpp::TCPInterface { (std::move(tun)) };
    auto& listener = tcp.bind({ "10.0.0.5"_nip, 4000 });

    std::mutex m;
    // TODO remove the hardcoded number
    std::vector<tcpp::TCPConnection<1 << 20>*> connections;

    std::jthread printer([&] {
        std::array<uint8_t, 2048> buffer { };
        std::unique_lock lock(m, std::defer_lock);
        while (true) {
            lock.lock();

            for (size_t i = 0; i < connections.size(); ++i) {
                auto& connection = *connections[i];
                auto id = id_str(connection.id);

                if (connection.connection_closed) {
                    std::cout << "(*) " << id << " left the room.\n";
                    connections[i] = connections.back();
                    connections.pop_back();
                }

                auto n = connection.read(buffer);
                if (n > 0) {
                    buffer[n] = '\0';
                    std::cout << id << ": ";
                    std::cout << buffer.data();
                    if (buffer[n - 1] != '\n')
                        std::cout << '\n';
                }
            }

            lock.unlock();
        }
    });

    while (true) {
        auto& connection = listener.accept();
        std::lock_guard lock(m);
        std::cout << "(*) " << id_str(connection.id) << " joined the room.\n";
        connections.push_back(&connection);
    }
}
