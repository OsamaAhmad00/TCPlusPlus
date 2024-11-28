#pragma once

#include <cstdint>
#include <cassert>
#include <endian.h>
#include <netinet/in.h>

#include <tcpp/structs/Base.hpp>
#include <tcpp/utils/Formatting.hpp>

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

    UDP& udp_payload() { assert(protocol == IPPROTOCOL_UDP); return extract<UDP>(payload_offset()); }

    TCP& tcp_payload() { assert(protocol == IPPROTOCOL_TCP); return extract<TCP>(payload_offset()); }

    [[nodiscard]] std::string source_ip() const { return network_ip_to_string(source_addr_n); }

    [[nodiscard]] std::string dest_ip() const { return network_ip_to_string(dest_addr_n); }

    [[nodiscard]] uint16_t total_len() const { return htons(total_len_n); }

    [[nodiscard]] constexpr size_t payload_offset() const { return ihl * 4; };

    [[nodiscard]] constexpr size_t payload_size() const { return total_len() - payload_offset(); };

    constexpr static uint8_t IPPROTOCOL_UDP = 0x11;
    constexpr static uint8_t IPPROTOCOL_TCP = 0x06;
};

}