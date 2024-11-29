#include <iostream>
#include <ostream>
#include <tcpp/structs/IPv4.hpp>
#include <tcpp/structs/UDP.hpp>
#include <tcpp/structs/TCP.hpp>

namespace tcpp::structs {

template <typename T>
uint16_t checksum_with_pseudo_header(const IPv4& ip, const T& t) {
    auto total_size = ip.payload_size();
    auto payload_size = total_size - t.payload_offset();
    auto checksum = t.header_and_payload_checksum(payload_size);
    // Add pseudo-header
    checksum.add_be(ip.source_addr_n)
        .add_be(ip.dest_addr_n)
        .add(ip.protocol)
        .add(static_cast<uint16_t>(total_size));
    return checksum;
}

uint16_t udp_checksum(const IPv4& ip) {
    return checksum_with_pseudo_header(ip, ip.udp_payload());
}

uint16_t tcp_checksum(const IPv4& ip) {
    return checksum_with_pseudo_header(ip, ip.tcp_payload());
}

void IPv4::compute_and_set_udp_checksum() {
    auto& udp = udp_payload();
    udp.checksum_n = 0;
    udp.checksum_n = udp_checksum(*this);
}

void IPv4::compute_and_set_tcp_checksum() {
    auto& tcp = tcp_payload();
    tcp.checksum_n = 0;
    tcp.checksum_n = tcp_checksum(*this);
}

bool IPv4::has_valid_udp_checksum() const {
    return udp_checksum(*this) == 0;
}

bool IPv4::has_valid_tcp_checksum() const {
    return tcp_checksum(*this) == 0;
}

}