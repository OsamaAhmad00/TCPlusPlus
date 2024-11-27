#pragma once

#include <cstdint>
#include <netinet/in.h>

#include <tcpp/structs/Base.hpp>

namespace tcpp::structs {

struct UDP : Base<UDP> {
    // _n = network byte order
    uint16_t source_port_n;
    uint16_t dest_port_n;
    uint16_t length_n;
    uint16_t checksum_n;

    [[nodiscard]] uint16_t source_port() const { return ntohs(source_port_n); }

    [[nodiscard]] uint16_t dest_port() const { return ntohs(dest_port_n); }

    [[nodiscard]] constexpr size_t payload_offset() const { return sizeof(*this); }

    [[nodiscard]] constexpr size_t payload_size() const { return ntohs(length_n) - payload_offset(); }
};

}