#include <netinet/in.h>

#include <tcpp/structs/IPv4.hpp>
#include <tcpp/structs/UDP.hpp>
#include <tcpp/structs/TCP.hpp>
#include <tcpp/utils/Checksum.hpp>
#include <tcpp/utils/Formatting.hpp>

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

void IPv4::compute_and_set_checksum() {
    checksum_n = 0;
    checksum_n = checksum16_be(reinterpret_cast<uint16_t*>(this), payload_offset() / 2);
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

void IPv4::compute_and_set_ip_tcp_checksums() {
    compute_and_set_tcp_checksum();
    compute_and_set_checksum();
}

void IPv4::compute_and_set_ip_udp_checksums() {
    compute_and_set_udp_checksum();
    compute_and_set_checksum();
}

void IPv4::set_source_ip(const uint32_t value) {
    source_addr_n = htonl(value);
}

void IPv4::set_dest_ip(const uint32_t value) {
    dest_addr_n = htonl(value);
}

void IPv4::set_total_len(const uint16_t value) {
    total_len_n = htons(value);
}

template <typename T>
static void _set_tcp_payload(IPv4& ip, const T& payload) {
    auto& tcp = ip.tcp_payload();
    tcp.set_payload(payload);
    const auto len = static_cast<uint16_t>(ip.payload_offset() + tcp.payload_offset() + payload.size());
    ip.set_total_len(len);
}

void IPv4::set_tcp_payload(const std::span<const uint8_t> payload) {
    _set_tcp_payload(*this, payload);
}

void IPv4::set_tcp_payload(const std::string& payload) {
    _set_tcp_payload(*this, payload);
}

// Computing the checksum with the checksum field not zeroed should result in 0

bool IPv4::has_valid_checksum() const {
    return checksum16_be(reinterpret_cast<const uint16_t*>(this), payload_offset() / 2) == 0;
}

bool IPv4::has_valid_udp_checksum() const {
    return udp_checksum(*this) == 0;
}

bool IPv4::has_valid_tcp_checksum() const {
    return tcp_checksum(*this) == 0;
}

uint16_t IPv4::checksum() const {
    return htons(checksum_n);
}

std::string IPv4::source_ip() const {
    return network_ip_to_string(source_addr_n);
}

std::string IPv4::dest_ip() const {
    return network_ip_to_string(dest_addr_n);
}

uint16_t IPv4::total_len() const {
    return htons(total_len_n);
}

ConnectionID IPv4::connection_id() const {
    auto& tcp = tcp_payload();
    return { source_addr_n, tcp.source_port(), tcp.dest_port() };
}

std::string IPv4::info() const {
    return packet_info(*this);
}

}