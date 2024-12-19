#pragma once

#include <map>
#include <mutex>

namespace tcpp {

template<typename Key, typename Value>
class ConcurrentMap {

    std::mutex m;
    std::map<Key, Value> map;

public:

    template <typename... Args>
    auto emplace(Args&&... args) {
        std::unique_lock lock(m);
        return map.emplace(std::forward<Args>(args)...);
    }

    auto erase(auto&& key) {
        std::lock_guard lock(m);
        return map.erase(key);
    }

    auto contains(auto&& key) {
        std::lock_guard lock(m);
        return map.contains(key);
    }

    auto find(auto&& key) {
        std::lock_guard lock(m);
        return map.find(key);
    }

    auto end() {
        std::lock_guard lock(m);
        return map.end();
    }
};

}