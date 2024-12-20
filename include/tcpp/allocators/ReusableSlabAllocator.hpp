#pragma once

#include <utility>
#include <tcpp/data-structures/MPMCBoundedQueue.hpp>

namespace tcpp {

// SlabSize: The size of each slab in multiples of sizeof(T), not in bytes
// Total allocated buffer = (sizeof(T) * SlabSize * SlabsCount) bytes
// The slabs are not cleared before reuse
template <typename T, size_t SlabSize, size_t SlabsCount>
requires PowerOfTwo<SlabsCount>
class ReusableSlabAllocator {
public:

    using value_type = T;

    ReusableSlabAllocator() : buffer(allocate_buffer()) {
        // We have two options here:
        //   1 - Keep track of the amount of slabs inserted into the queue from the buffer
        //   and only insert new slabs into the queue if we need one and the queue is empty
        //   2 - Insert all the slabs into the queue, and don't keep track of anything
        // Here, number 2 is implemented for faster allocations and deallocations
        for (size_t i = 0; i < SlabsCount; i++) {
            available_slabs.push(buffer + i * SlabSize);
        }
    }

    ReusableSlabAllocator(ReusableSlabAllocator&& other) noexcept
        : buffer(std::exchange(other.buffer, nullptr)) { }

    ReusableSlabAllocator(const ReusableSlabAllocator& other) = delete;

    ~ReusableSlabAllocator() noexcept {
        std::allocator<T> alloc;
        std::allocator_traits<std::allocator<T>>::deallocate(alloc, buffer, SlabSize * SlabsCount);
    }

    T* allocate(std::size_t n = 1) {
        (void)n;
        auto slab = available_slabs.pop();
        if (!slab.has_value())
            throw std::runtime_error("The slab allocator ran out of slabs");
        return slab.value();
    }

    void deallocate(T* p, std::size_t n = 1) noexcept {
        (void)n;
        available_slabs.push(p);
    }

    template <typename U, typename... Args>
    void construct(U* p, Args&&... args) {
        ::new((void*)p) U(std::forward<Args>(args)...);
    }

    template <typename U>
    void destroy(U* p) noexcept {
        p->~U();
    }

private:

    T* allocate_buffer() {
        std::allocator<T> alloc;
        return std::allocator_traits<std::allocator<T>>::allocate(alloc, SlabSize * SlabsCount);
    }

    T* buffer;
    MPMCBoundedQueue<T*, SlabsCount> available_slabs;
};


// SlabSize: The size of each slab in multiples of sizeof(T), not in bytes
// Total allocated buffer = (sizeof(T) * SlabSize * SlabsCount) bytes
// The slabs are not cleared before reuse
template <typename T, size_t SlabSize, size_t SlabsCount>
requires PowerOfTwo<SlabsCount>
class ReusableSlabSingletonAllocator {

    inline static ReusableSlabAllocator<T, SlabSize, SlabsCount> instance;

public:

    using value_type = ReusableSlabAllocator<T, SlabSize, SlabsCount>::value_type;

    T* allocate(std::size_t n = 1) {
        return instance.allocate(n);
    }

    void deallocate(T* p, std::size_t n = 1) noexcept {
        return instance.deallocate(p, n);
    }

    template <typename U, typename... Args>
    void construct(U* p, Args&&... args) {
        return instance.construct(p, std::forward<Args>(args)...);
    }

    template <typename U>
    void destroy(U* p) noexcept {
        return instance.destroy(p);
    }
};

}