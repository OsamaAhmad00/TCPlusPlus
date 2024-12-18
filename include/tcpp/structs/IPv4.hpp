#pragma once

#include <cstdint>
#include <cassert>
#include <endian.h>
#include <string>

#include <tcpp/structs/Base.hpp>
#include <tcpp/utils/Connections.hpp>

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

    void compute_and_set_checksum();

    void compute_and_set_udp_checksum();

    void compute_and_set_tcp_checksum();

    void compute_and_set_ip_tcp_checksums();

    void compute_and_set_ip_udp_checksums();

    void set_source_ip(uint32_t value);

    void set_dest_ip(uint32_t value);

    void set_total_len(uint16_t value);

    void set_tcp_payload(std::span<const uint8_t> payload);

    void set_tcp_payload(const std::string& payload);

    [[nodiscard]] bool has_valid_checksum() const;

    [[nodiscard]] bool has_valid_udp_checksum() const;

    [[nodiscard]] bool has_valid_tcp_checksum() const;

    [[nodiscard]] uint16_t checksum() const;

    [[nodiscard]] std::string source_ip() const;

    [[nodiscard]] std::string dest_ip() const;

    [[nodiscard]] uint16_t total_len() const;

    [[nodiscard]] ConnectionID connection_id() const;

    [[nodiscard]] constexpr size_t payload_offset() const { return ihl * 4; }

    [[nodiscard]] constexpr size_t payload_size() const { return total_len() - payload_offset(); }

    [[nodiscard]] std::string info() const;

    constexpr static uint8_t IPPROTOCOL_UDP = 0x11;
    constexpr static uint8_t IPPROTOCOL_TCP = 0x06;
};

}
