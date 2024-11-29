#include <tcpp/utils/Checksum.hpp>

namespace tcpp {

static uint16_t reverse_bytes(const uint16_t x) { return static_cast<uint16_t>((x >> 8) + ((x & 0xFF) << 8)); }

uint16_t checksum16_be(const uint16_t *data, const size_t size, uint16_t initial_value) {
    uint32_t sum = initial_value;

    for (size_t i = 0; i < size; i++) {
        sum += reverse_bytes(data[i]);
    }

    // End-around carry will happen at most twice
    sum = (sum & 0xFFFF) + (sum >> 16);
    sum = (sum & 0xFFFF) + (sum >> 16);

    const uint16_t ones_complement = ~static_cast<uint16_t>(sum);

    return reverse_bytes(ones_complement);
}

}