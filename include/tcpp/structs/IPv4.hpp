#pragma once

#include <cstdint>
#include <cassert>
#include <endian.h>
#include <netinet/in.h>

#include <tcpp/structs/Base.hpp>
#include <tcpp/utils/Formatting.hpp>
#include <tcpp/utils/Checksum.hpp>

namespace tcpp::structs {

struct UDP;
struct TCP;

struct IPv4 : Base<IPv4> {
    // _n = network byte order
#if BYTE_ORDER == LITTLE_ENDIAN
    unsigned int ihl : 4;
    unsigned int version : 4;
#else
    unsigned int version : 4;
    unsigned int ihl : 4;
#endif
    uint8_t tos;
    uint16_t total_len_n;
    uint16_t id_n;
    uint16_t fragment_offset_n; // includes the 3 flag bits
    uint8_t ttl;
    uint8_t protocol;
    uint16_t checksum_n;
    uint32_t source_addr_n;
    uint32_t dest_addr_n;

    // Options go here

    template <typename Self>
    auto& udp_payload(this Self& self) { assert(self.protocol == IPPROTOCOL_UDP); return self.template extract<UDP>(self.payload_offset()); }

    template <typename Self>
    auto& tcp_payload(this Self& self) { assert(self.protocol == IPPROTOCOL_TCP); return self.template extract<TCP>(self.payload_offset()); }

    void compute_and_set_checksum() { checksum_n = 0; checksum_n = checksum16_be(reinterpret_cast<uint16_t*>(this), payload_offset() / 2); }

    void compute_and_set_udp_checksum();

    void compute_and_set_tcp_checksum();

    // Computing the checksum with the checksum field not zeroed should result in 0
    [[nodiscard]] bool has_valid_checksum() { return checksum16_be(reinterpret_cast<uint16_t*>(this), payload_offset() / 2) == 0; }

    [[nodiscard]] bool has_valid_udp_checksum() const;

    [[nodiscard]] bool has_valid_tcp_checksum() const;

    [[nodiscard]] uint16_t checksum() const { return htons(checksum_n); }

    [[nodiscard]] std::string source_ip() const { return network_ip_to_string(source_addr_n); }

    [[nodiscard]] std::string dest_ip() const { return network_ip_to_string(dest_addr_n); }

    [[nodiscard]] uint16_t total_len() const { return htons(total_len_n); }

    [[nodiscard]] constexpr size_t payload_offset() const { return ihl * 4; };

    [[nodiscard]] constexpr size_t payload_size() const { return total_len() - payload_offset(); };

    [[nodiscard]] std::string info() const { return packet_info(*this); }

    constexpr static uint8_t IPPROTOCOL_UDP = 0x11;
    constexpr static uint8_t IPPROTOCOL_TCP = 0x06;
};

}
