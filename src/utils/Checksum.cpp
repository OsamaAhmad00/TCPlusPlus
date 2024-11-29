#include <tcpp/utils/Checksum.hpp>

namespace tcpp {

static uint16_t reverse_bytes(const uint16_t x) { return static_cast<uint16_t>((x >> 8) + ((x & 0xFF) << 8)); }

Checksum16BE& Checksum16BE::add(const uint8_t x) {
    return add(static_cast<uint16_t>(x));
}

Checksum16BE& Checksum16BE::add(const uint16_t x) {
    sum += x;
    return *this;
}

Checksum16BE& Checksum16BE::add(const uint32_t x) {
    return add(static_cast<uint16_t>(x >> 16)).add(static_cast<uint16_t>(x & 0xFFFF));
}

Checksum16BE& Checksum16BE::add_be(const uint16_t x) {
    return add(reverse_bytes(x));
}

Checksum16BE& Checksum16BE::add_be(const uint32_t x) {
    return add_be(static_cast<uint16_t>(x >> 16)).add_be(static_cast<uint16_t>(x & 0xFFFF));
}

uint16_t Checksum16BE::get() const {
    // End-around carry will happen at most twice
    uint16_t result = (sum & 0xFFFF) + (sum >> 16);
    result = (result & 0xFFFF) + (result >> 16);

    // Take one's complement then reverse back to big-endian
    return reverse_bytes(~result);
}

Checksum16BE checksum16_be(const uint16_t *data, const size_t size, uint16_t initial_value) {
    Checksum16BE checksum { initial_value };
    for (size_t i = 0; i < size; i++)
        checksum.add_be(data[i]);
    return checksum;
}

}