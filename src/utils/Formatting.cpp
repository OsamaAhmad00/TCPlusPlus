#include <tcpp/utils/Formatting.hpp>

namespace tcpp {

std::string network_ip_to_string(uint32_t ip) {
    std::string result;
    result.reserve(3 * 4 + 4);

    // TODO validate input
    auto bytes = reinterpret_cast<uint8_t *>(&ip);
    for (int i = 0; i < 4; i++) {
        result += std::to_string(bytes[i]);
        result.push_back('.');
    }
    result.pop_back();

    return result;
}

}