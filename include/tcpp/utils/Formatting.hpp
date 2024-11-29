#pragma once

#include <cstdint>
#include <string>

namespace tcpp {

namespace structs { struct IPv4; }

std::string packet_info(const structs::IPv4& ip);

std::string network_ip_to_string(uint32_t ip);

}