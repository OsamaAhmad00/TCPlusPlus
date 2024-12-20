#pragma once

#include <cstdint>
#include <tcpp/utils/Concepts.hpp>

namespace tcpp {

using Port = uint16_t;
using IpAddress = uint32_t;

template <typename T, size_t SlabSize, size_t SlabsCount>
requires PowerOfTwo<SlabsCount>
class ReusableSlabSingletonAllocator;


constexpr int PacketBufferSize = 2048;

constexpr int AllocatablePacketsCount = 1024;

using PacketBuffer = uint8_t*;

using ReusableAllocator = ReusableSlabSingletonAllocator<uint8_t, PacketBufferSize, AllocatablePacketsCount>;

}
