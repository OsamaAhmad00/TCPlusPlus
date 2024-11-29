#include <tcpp/structs/IPv4.hpp>
#include <tcpp/structs/UDP.hpp>

namespace tcpp::structs {

uint16_t udp_checksum(const IPv4& ip) {
    auto& udp = ip.udp_payload();
    auto udp_len = ip.payload_size();
    auto checksum = udp.header_and_payload_checksum(udp_len - udp.payload_offset());
    // Add pseudo-header
    checksum.add(ip.source_addr_n)
        .add(ip.dest_addr_n)
        .add(htons(IPv4::IPPROTOCOL_UDP))
        .add(htons(static_cast<uint16_t>(udp_len)));
    return checksum;
}

void IPv4::compute_and_set_udp_checksum() {
    auto& udp = udp_payload();
    udp.checksum_n = 0;
    udp.checksum_n = udp_checksum(*this);
}

bool IPv4::has_valid_udp_checksum() const {
    return udp_checksum(*this) == 0;
}

}