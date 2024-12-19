#pragma once

#include <cstdint>
#include <string>
#include <tcpp/TypeDefs.hpp>

namespace tcpp {

namespace structs { struct IPv4; }

std::string packet_info(const structs::IPv4& ip);

std::string network_ip_to_string(uint32_t ip);

IpAddress string_to_network_ip(const std::string& ip);

// Convert IPv4 string to a network byte order 32-bit unsigned integer
IpAddress operator""_nip(const char* ip, size_t size);

}
