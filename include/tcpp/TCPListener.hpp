#pragma once

#include <tcpp/TypeDefs.hpp>
#include <tcpp/utils/Connections.hpp>
#include <tcpp/TCPConnection.hpp>
#include <tcpp/data-structures/SPSCBoundedWaitFreeQueue.hpp>
#include <tcpp/data-structures/ConcurrentMap.hpp>

namespace tcpp {

template <size_t ConnectionQueueCapacity>
class TCPListener {
    template <size_t ConnectionBufferSize>
    requires PowerOfTwo<ConnectionBufferSize>
    friend class TCPInterface;

    const Endpoint endpoint;
    MPSCBoundedQueue<PacketBuffer, ConnectionQueueCapacity>& send_queue;

    std::atomic<int> acceptable_connections = 0;
    std::atomic<TCPConnection<ConnectionQueueCapacity>*> connection_to_return;

    // TODO specify the size specifically
    ConcurrentMap<Endpoint, TCPListener>& listeners;

public:

    // TODO delete this
    std::atomic<bool> under_usage = false;

    // TODO make it private
    TCPListener(
        const Endpoint endpoint,
        MPSCBoundedQueue<PacketBuffer, ConnectionQueueCapacity>& send_queue,
        ConcurrentMap<Endpoint, TCPListener>& listeners
    ) : endpoint(endpoint),
        send_queue(send_queue),
        listeners(listeners)
    { }

    // TODO same size by default?
    TCPConnection<ConnectionQueueCapacity>& accept() {
        acceptable_connections++;  // TODO memory order
        // TODO stop spinning
        // TODO memory orders
        connection_to_return.wait(nullptr);
        auto connection = connection_to_return.load();
        connection_to_return = nullptr;
        return *connection;
    }

    // TODO delete all connections from the connections map in the interface
    void close() {
        auto it = listeners.find(endpoint);
        listeners.erase(it);
    }

    ~TCPListener() noexcept {
        // TODO delete this
        while (under_usage.load()) {}
    }
};

};