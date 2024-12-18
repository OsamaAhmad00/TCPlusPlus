#pragma once

#include <tcpp/data-structures/SPSCBoundedWaitFreeQueue.hpp>

namespace tcpp {

template <typename T, size_t Capacity, typename Alloc = std::allocator<T>>
requires PowerOfTwo<Capacity>
class MPSCBoundedQueue : public SPSCBoundedWaitFreeQueue<T, Capacity, Alloc> {

    std::mutex m;

public:

    template <typename... Args>
    bool push(Args&&... args) {
        std::lock_guard lock(m);
        return SPSCBoundedWaitFreeQueue<T, Capacity, Alloc>::push(std::forward<Args>(args)...);
    }
};

}