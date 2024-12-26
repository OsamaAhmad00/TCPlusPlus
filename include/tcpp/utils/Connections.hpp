#pragma once

#include <tuple>

#include <tcpp/TypeDefs.hpp>

namespace tcpp {

struct ConnectionID {
    IpAddress source_ip = 0xFFFFFFFF;
    IpAddress dest_ip = 0xFFFFFFFF;
    Port source_port = 0xFFFF;
    Port dest_port = 0xFFFF;

    // operator __uint128_t() const {
    //     return static_cast<__uint128_t>(source_ip)   << 64 |
    //            static_cast<__uint128_t>(dest_ip)     << 32 |
    //            static_cast<__uint128_t>(source_port) << 16 |
    //            static_cast<__uint128_t>(dest_port);
    // }

    friend auto operator<=>(const ConnectionID& lhs, const ConnectionID& rhs) {
        return std::tie(lhs.source_ip, lhs.dest_ip, lhs.source_port, lhs.dest_port) <=>
               std::tie(rhs.source_ip, rhs.dest_ip, rhs.source_port, rhs.dest_port);
    }
};

struct Endpoint {
    IpAddress ip = 0xFFFFFFFF;
    Port port = 0xFFFF;

    operator uint64_t() const {
        return static_cast<uint64_t>(ip) << 16 |
               static_cast<uint64_t>(port);
    }
};

}