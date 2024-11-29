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

template <typename T, typename U>
using CopyConstness = std::conditional_t<std::is_const_v<std::remove_reference_t<T>>, const U, U>;

template <typename T>
struct Base {
    template <typename U>
    static T& from_ptr(U* ptr) {
        // Check for alignment?
        return *reinterpret_cast<T*>(ptr);
    }

    template <typename U, typename Self>
    auto& extract(this Self& self, const size_t offset) {
        using Bytes = CopyConstness<Self, uint8_t>;
        using Result = CopyConstness<Self, U>;
        auto ptr = reinterpret_cast<Bytes*>(&self);
        return *reinterpret_cast<Result*>(ptr + offset);
    }

    template <typename Self>
    auto payload(this Self& self, const size_t payload_size) requires PayloadHolder<T> {
        using Derived = CopyConstness<Self, T>;
        using Bytes = CopyConstness<Self, uint8_t>;
        auto ptr = reinterpret_cast<Bytes*>(&self);
        auto& derived = *reinterpret_cast<Derived*>(&self);
        auto begin = ptr + derived.payload_offset();
        auto end = begin + payload_size;
        return std::span { begin, end };
    }

    template <typename Self>
    auto payload(this Self& self) requires PayloadHolderWithSizeInfo<T> {
        using Derived = CopyConstness<Self, T>;
        auto& derived = *reinterpret_cast<Derived*>(&self);
        return self.payload(derived.payload_size());
    }
};

}
