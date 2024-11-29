#include <gtest/gtest.h>

#include <chrono>

#include <tcpp/TunDevice.hpp>
#include <tcpp/structs/IPv4.hpp>
#include <tcpp/structs/UDP.hpp>

// UDP 10.0.0.1:52716 -> 10.0.0.2:4000
// IP checksum: 0xc927
// UDP checksum: 0xea66
// Payload: "Hello"
uint8_t udp_packet[] = { 0x45, 0x0, 0x0, 0x22, 0x5d, 0xa1, 0x40, 0x0, 0x40, 0x11, 0xc9, 0x27, 0xa, 0x0,
    0x0, 0x1, 0xa, 0x0, 0x0, 0x2, 0xcd, 0xec, 0xf, 0xa0, 0x0, 0xe, 0xea, 0x66, 0x48, 0x65, 0x6c, 0x6c, 0x6f, 0xa };

TEST(checksum, IPv4HeaderChecksum) {
    auto& ip = tcpp::structs::IPv4::from_ptr(udp_packet);
    ASSERT_TRUE(ip.has_valid_checksum());
    const auto old = ip.checksum();
    ASSERT_EQ(old, 0xc927);
    ip.compute_and_set_checksum();
    ASSERT_EQ(ip.checksum(), 0xc927);
}

TEST(checksum, UDPHeaderChecksum) {
    auto& ip = tcpp::structs::IPv4::from_ptr(udp_packet);
    auto& udp = ip.udp_payload();
    ASSERT_TRUE(ip.has_valid_udp_checksum());
    const auto old = udp.checksum();
    ASSERT_EQ(old, 0xea66);
    ip.compute_and_set_udp_checksum();
    ASSERT_EQ(udp.checksum(), 0xea66);
}
