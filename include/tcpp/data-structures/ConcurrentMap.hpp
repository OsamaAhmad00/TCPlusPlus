#pragma once

#include <map>
#include <mutex>

namespace tcpp {

template<typename Key, typename Value>
class ConcurrentMap {

    std::mutex m;
    // TODO review memory orders
    std::atomic<bool> is_read_only = false;
    std::map<Key, Value> map;

public:

    template <typename... Args>
    auto emplace(Args&&... args) {
        std::unique_lock lock(m);
        // TODO is this acceptable?
        if (is_read_only) return std::make_pair(map.end(), false);
        return map.emplace(std::forward<Args>(args)...);
    }

    auto erase(auto&& key) {
        std::lock_guard lock(m);
        // TODO this prevents other overloads. Make it work with all overloads
        if (is_read_only) return map.end();
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

    auto find_and_perform(auto&& key, auto&& func) {
        std::lock_guard lock(m);
        auto it = map.find(key);
        if (it != map.end())
            func(it->second);
        return it;
    }

    auto clear() {
        std::lock_guard lock(m);
        return map.clear();
    }

    auto begin() {
        std::lock_guard lock(m);
        return map.begin();
    }

    auto end() {
        std::lock_guard lock(m);
        return map.end();
    }

    void set_read_only() {
        // Used before destruction, to ensure that
        // no elements are inserted or removed, and
        // thus, we can iterate without holding the lock
        is_read_only = true;
    }

    ~ConcurrentMap() noexcept {
        std::lock_guard lock(m);
        map.clear();
    }
};

}