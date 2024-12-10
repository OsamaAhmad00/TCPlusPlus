#pragma once

#include <cstdint>
#include <cstddef>

namespace tcpp {

struct Checksum16BE {
    uint32_t sum = 0;
    Checksum16BE() = default;
    explicit Checksum16BE(const uint16_t initial_value) : sum(initial_value) { }
    Checksum16BE& add(uint8_t  x);
    Checksum16BE& add(uint16_t x);
    Checksum16BE& add(uint32_t x);
    Checksum16BE& add_be(uint16_t x);
    Checksum16BE& add_be(uint32_t x);
    [[nodiscard]] uint16_t get() const;
    operator uint16_t() const { return get(); }
};

Checksum16BE checksum16_be(const uint16_t* data, size_t size, uint16_t initial_value = 0);

Checksum16BE checksum16_be(const uint8_t* data, size_t size, uint16_t initial_value = 0);

}