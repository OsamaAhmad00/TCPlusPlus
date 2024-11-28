#pragma once

#include <cstdint>
#include <netinet/in.h>

#include <tcpp/structs/Base.hpp>

namespace tcpp::structs {

struct TCP : Base<TCP> {
    // _n = network byte order
    uint16_t source_port_n;
    uint16_t dest_port_n;
    uint32_t seq_num_n;
    uint32_t ack_num_n;
#if BYTE_ORDER == LITTLE_ENDIAN
    uint16_t
        reserved : 4,
        data_offset : 4,
        fin : 1,
        syn : 1,
        rst : 1,
        psh : 1,
        ack : 1,
        urg : 1,
        ece : 1,
        cwr : 1;
#else
    uint16_t
        data_offset_n : 4,
        reserved : 4,
        cwr : 1,
        ece : 1,
        urg : 1,
        ack : 1,
        psh : 1,
        rst : 1,
        syn : 1,
        fin : 1;
#endif
    uint16_t window_size_n;
    uint16_t checksum_n;
    uint16_t urgent_ptr_n;

    [[nodiscard]] uint16_t source_port() const { return ntohs(source_port_n); }

    [[nodiscard]] uint16_t dest_port() const { return ntohs(dest_port_n); }

    [[nodiscard]] uint32_t seq_num() const { return ntohl(seq_num_n); }

    [[nodiscard]] uint32_t ack_num() const { return ntohl(ack_num_n); }

    [[nodiscard]] uint16_t window_size() const { return ntohs(window_size_n); }

    [[nodiscard]] uint16_t checksum() const { return ntohs(checksum_n); }

    [[nodiscard]] uint16_t urgent_ptr() const { return ntohs(urgent_ptr_n); }

    [[nodiscard]] constexpr size_t payload_offset() const { return data_offset * 4; }
};

}