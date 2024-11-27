#pragma once

#include <span>

namespace tcpp::structs {

template <typename T>
concept PayloadHolder = requires(T t) {
    { t.payload_offset() } -> std::convertible_to<size_t>;
};

template <typename T>
concept PayloadHolderWithSizeInfo = requires(T t) {
    requires PayloadHolder<T>;
    { t.payload_size() } -> std::convertible_to<size_t>;
};

template <typename T>
struct Base {
    template <typename U>
    static T& from_ptr(U* ptr) {
        // Check for alignment?
        return *reinterpret_cast<T*>(ptr);
    }

    template <typename U>
    U& extract(const size_t offset) {
        auto self = reinterpret_cast<uint8_t*>(this);
        return *reinterpret_cast<U*>(self + offset);
    }

    std::span<uint8_t> payload(const size_t payload_size) requires PayloadHolder<T> {
        auto ptr = reinterpret_cast<uint8_t*>(this);
        auto& self = *reinterpret_cast<T*>(this);
        auto begin = ptr + self.payload_offset();
        auto end = begin + payload_size;
        return std::span { begin, end };
    }

    std::span<uint8_t> payload() requires PayloadHolderWithSizeInfo<T> {
        auto& self = *reinterpret_cast<T*>(this);
        return payload(self.payload_size());
    }
};

}
