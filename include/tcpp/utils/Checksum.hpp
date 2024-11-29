#pragma once

#include <cstdint>
#include <string>

namespace tcpp {

uint16_t checksum16_be(const uint16_t* data, size_t size, uint16_t initial_value = 0);

}