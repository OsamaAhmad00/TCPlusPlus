#pragma once

#include <span>
#include <tcpp/utils/Checksum.hpp>

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
        auto& derived = static_cast<Derived&>(self);
        auto begin = ptr + derived.payload_offset();
        auto end = begin + payload_size;
        return std::span { begin, end };
    }

    template <typename Self>
    auto payload(this Self& self) requires PayloadHolderWithSizeInfo<T> {
        using Derived = CopyConstness<Self, T>;
        auto& derived = static_cast<Derived&>(self);
        return self.payload(derived.payload_size());
    }

    Checksum16BE header_and_payload_checksum(size_t payload_size) const requires PayloadHolder<T> {
        auto& derived = static_cast<const T&>(*this);
        return checksum16_be(reinterpret_cast<const uint8_t*>(this), derived.payload_offset() + payload_size);
    }

    Checksum16BE header_and_payload_checksum() const requires PayloadHolderWithSizeInfo<T> {
        auto& derived = static_cast<const T&>(*this);
        return header_and_payload_checksum(derived.payload_size());
    }

    /*
     * Ensure that there is enough space for this data in the buffer, because this function won't.
     * Also, insure to update size-related fields because this function won't update them as well.
     */
    void set_payload(const std::span<const uint8_t> payload) requires PayloadHolder<T> {
        auto& derived = static_cast<const T&>(*this);
        auto offset = derived.payload_offset();
        auto buffer = reinterpret_cast<uint8_t*>(this);
        for (size_t i = 0; i < payload.size(); i++) {
            buffer[offset + i] = payload[i];
        }
    }

    /*
     * Ensure that there is enough space for this data in the buffer, because this function won't.
     * Also, insure to update size-related fields because this function won't update them as well.
     */
    void set_payload(const std::string& payload) requires PayloadHolder<T> {
        const auto buffer = reinterpret_cast<const uint8_t*>(payload.c_str());
        set_payload(std::span { buffer , payload.size() });
    }
};

}
