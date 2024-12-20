#pragma once

#include <tcpp/data-structures/MPSCBoundedQueue.hpp>

namespace tcpp {

template <typename T, size_t Capacity, typename Alloc = std::allocator<T>>
requires PowerOfTwo<Capacity>
class MPMCBoundedQueue : public MPSCBoundedQueue<T, Capacity, Alloc> {

public:

    std::optional<T> pop() {
        std::lock_guard lock(this->m);
        return SPSCBoundedWaitFreeQueue<T, Capacity, Alloc>::pop();
    }
};

}