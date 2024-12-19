#pragma once

#include <cstdint>

using Port = uint16_t;
using IpAddress = uint32_t;


// TODO remove these --------
#include <memory>

#include <tcpp/data-structures/MPSCBoundedQueue.hpp>
#include <tcpp/utils/Concepts.hpp>

template <typename T, size_t Capacity, typename Alloc>
requires PowerOfTwo<Capacity>
class SPSCBoundedWaitFreeQueue;

// TODO this needs to change
using ByteArr = std::array<uint8_t, 2048>;
using ByteArrPtr = std::unique_ptr<ByteArr>;

template <size_t Capacity, typename Alloc = std::allocator<ByteArrPtr>>
using SPSCQueue = SPSCBoundedWaitFreeQueue<ByteArrPtr, Capacity, Alloc>;

template <size_t Capacity, typename Alloc = std::allocator<ByteArrPtr>>
using MPSCQueue = tcpp::MPSCBoundedQueue<ByteArrPtr, Capacity, Alloc>;

