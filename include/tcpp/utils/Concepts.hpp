#pragma once

constexpr bool is_power_of_2(size_t s) {
    uint8_t count = 0;
    while (s > 0) {
        count += s & 1;
        s >>= 1;
    }
    return count == 1;
}

template <size_t S>
concept PowerOfTwo = is_power_of_2(S);

