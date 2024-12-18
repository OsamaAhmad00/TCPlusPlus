#pragma once

#include <cassert>
#include <map>
#include <mutex>

#include <tcpp/TypeDefs.hpp>

namespace tcpp {

struct ConnectionID {
    // Our IP address is always the same, no need to store it.
    IpAddress ip = 0xFFFFFFFF;  // IP address of the other end
    Port source_port = 0xFFFF;
    Port dest_port = 0xFFFF;

    operator uint64_t() const {
        return static_cast<uint64_t>(ip) << 32 |
               static_cast<uint64_t>(source_port) << 16 |
               static_cast<uint64_t>(dest_port);
    }
};

template <size_t QueueCapacity>
class ConnectionQueues {
    // TODO get rid of the mutex
    std::mutex mutex;
    // TODO change std::map, but make sure you have stable references
    std::map<ConnectionID, SPSCQueue<QueueCapacity>> queues;

public:

    void create_connection_queue(const ConnectionID id) {
        std::lock_guard lock(mutex);
        assert(!queues.contains(id));
        queues[id];
    }

    void remove_connection_queue(const ConnectionID id) {
        std::lock_guard lock(mutex);
        queues.erase(id);
    }

    auto find(const ConnectionID id) { return queues.find(id); }

    auto end() { return queues.end(); }

    auto& operator[](const ConnectionID id) {
        std::lock_guard lock(mutex);
        return queues[id];
    }
};

}