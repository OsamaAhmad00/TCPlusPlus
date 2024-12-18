#pragma once

#include <atomic>
#include <memory>
#include <optional>

#include <tcpp/utils/Concepts.hpp>

constexpr int CACHE_LINE_SIZE = 64;
//constexpr int CACHE_LINE_SIZE = std::hardware_destructive_interference_size;

template <typename T, size_t capacity, typename Alloc = std::allocator<T>>
requires PowerOfTwo<capacity>
class SPSCBoundedWaitFreeQueue : protected Alloc {

public:

    using value_type = T;
    using size_type = decltype(capacity);
    using allocator_traits = std::allocator_traits<Alloc>;

    SPSCBoundedWaitFreeQueue() : buffer(allocator_traits::allocate(*this, capacity)) { }

    // TODO review memory orders in push and pop

    template <typename... Args>
    bool push(Args&&... args) {
        auto push_val = push_ptr.load(std::memory_order::acquire);
        if (!can_push(push_val)) return false;
        allocator_traits::construct(*this, element_ptr(push_val), std::forward<Args>(args)...);
        push_ptr.store(push_val + 1, std::memory_order::release);
        return true;
    }

    std::optional<value_type> pop() {
        auto pop_val = pop_ptr.load(std::memory_order::acquire);
        if (!can_pop(pop_val)) return std::nullopt;
        auto ptr = element_ptr(pop_val);
        std::optional<value_type> result = std::move(*ptr);
        allocator_traits::destroy(*this, ptr);
        pop_ptr.store(pop_val + 1, std::memory_order::release);
        return result;
    }

    ~SPSCBoundedWaitFreeQueue() {
        // TODO relaxed then acquire?
        size_type pop_index = pop_ptr.load(std::memory_order::relaxed);
        size_type push_index = push_ptr.load(std::memory_order::acquire);
        for (size_type i = pop_index; i < push_index; i++)
            allocator_traits::destroy(*this, element_ptr(i));
        allocator_traits::deallocate(*this, buffer, capacity);
    }

protected:

    bool can_push(size_type push_ptr_) {
        if (is_full(push_ptr_, cached_pop_ptr)) {
            cached_pop_ptr = pop_ptr.load(std::memory_order::acquire);
            if (is_full(push_ptr_, cached_pop_ptr)) {
                return false;
            }
        }
        return true;
    }

    bool can_pop(size_type pop_ptr_) {
        if (is_empty(cached_push_ptr, pop_ptr_)) {
            cached_push_ptr = push_ptr.load(std::memory_order::acquire);
            if (is_empty(cached_push_ptr, pop_ptr_)) {
                return false;
            }
        }
        return true;
    }

    bool is_full(size_type push, size_type pop) { return (push - pop) == capacity; }

    bool is_empty(size_type push, size_type pop) { return push == pop; }

    value_type* element_ptr(size_type ptr) { return &buffer[ptr & (capacity - 1)]; }

    using cursor_type = std::atomic<size_type>;
    static_assert(cursor_type::is_always_lock_free);

    value_type* buffer = nullptr;

    // The push and pop pointers keep increasing forever. This assumes that
    //  the system restarts, and they'll begin at 0 again soon before they
    //  overflow. Given that they're 64-bits, it's very unlikely that any
    //  of them will overflow. This is done to distinguish between the
    //  empty and the full case with just a simple subtraction.

    // Loaded and stored by the push thread, loaded only by the pop thread
    alignas(CACHE_LINE_SIZE) cursor_type push_ptr { };
    // Exclusive to the push thread
    alignas(CACHE_LINE_SIZE) size_type   cached_pop_ptr { };
    // Loaded and stored by the pop thread, loaded only by the push thread
    alignas(CACHE_LINE_SIZE) cursor_type pop_ptr { };
    // Exclusive to the pop thread
    alignas(CACHE_LINE_SIZE) size_type   cached_push_ptr { };

    // To avoid false sharing with any adjacent data
    char padding_[CACHE_LINE_SIZE - sizeof(cached_push_ptr)] { };
};