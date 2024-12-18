#pragma once

#include <tcpp/TypeDefs.hpp>
#include <tcpp/utils/Connections.hpp>
#include <tcpp/TCPConnection.hpp>
#include <tcpp/data-structures/SPSCBoundedWaitFreeQueue.hpp>

namespace tcpp {

template <size_t ConnectionQueueCapacity>
class TCPListener {
    template <size_t ConnectionBufferSize>
    requires PowerOfTwo<ConnectionBufferSize>
    friend class TCPInterface;

    const Port port;
    MPSCQueue<ConnectionQueueCapacity>& send_queue;
    ConnectionQueues<ConnectionQueueCapacity>& connection_queues;

    // TODO change this
    SPSCBoundedWaitFreeQueue<ConnectionID, 64> pending_connections;

public:

    // TODO remove the port from the interface when this is destroyed

    // TODO make it private
    TCPListener(const Port port, MPSCQueue<ConnectionQueueCapacity>& send_queue,
        ConnectionQueues<ConnectionQueueCapacity>& connection_queues)
        : port(port), send_queue(send_queue), connection_queues(connection_queues)
    { }

    // TODO same size by default?
    template <size_t ConnectionBufferSize = ConnectionQueueCapacity>
    void accept(std::deque<TCPConnection<ConnectionBufferSize>>& container) {
        // TODO stop spinning
        auto id = pending_connections.pop();
        while (!id.has_value()) {
            id = pending_connections.pop();
        }
        auto& receive_queue = connection_queues[id.value()];
        container.emplace_back(id.value(), connection_queues, receive_queue, send_queue);
    }
};

};