#pragma once

#include <set>
#include <mutex>

namespace tcpp {

template<typename Key>
class ConcurrentSet {

    std::mutex m;
    std::set<Key> set;

public:

    template <typename... Args>
    auto emplace(Args&&... args) {
        std::unique_lock lock(m);
        return set.emplace(std::forward<Args>(args)...);
    }

    auto erase(auto&& key) {
        std::lock_guard lock(m);
        return set.erase(key);
    }

    auto contains(auto&& key) {
        std::lock_guard lock(m);
        return set.contains(key);
    }

    auto find(auto&& key) {
        std::lock_guard lock(m);
        return set.find(key);
    }

    auto end() {
        std::lock_guard lock(m);
        return set.end();
    }
};

}