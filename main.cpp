#include <iostream>

#include <tcpp/TunDevice.hpp>

[[noreturn]] int main() {
    auto tun = tcpp::TunBuilder("TunDevice0")
        .set_ip4("10.0.0.1")
        .set_netmask("255.0.0.0")
        .build();

    std::array<uint8_t, 2048> buffer { };
    while (true) {
        const size_t n = tun.receive(buffer);
        std::cout << "Received " << std::dec << n << " bytes from TunDevice0:\n";
        for (size_t i = 0; i < n; i++)
            std::cout << std::hex << +buffer[i] << ' ';
        std::cout << "\n\n";
    }
}