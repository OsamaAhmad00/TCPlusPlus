#pragma once

#include <map>
#include <thread>

#include <tcpp/TypeDefs.hpp>
#include <tcpp/TunDevice.hpp>
#include <tcpp/TCPListener.hpp>
#include <tcpp/utils/Connections.hpp>
#include <tcpp/data-structures/ConcurrentMap.hpp>

namespace tcpp {

template <size_t ConnectionBufferSize = (1 << 20)>
requires PowerOfTwo<ConnectionBufferSize>
class TCPInterface {

public:

    explicit TCPInterface(TunDevice interface)
        : interface(std::move(interface)),
          listen_thread(&TCPInterface::listener, this),
          send_thread(&TCPInterface::sender, this)
    { }

    // TODO accept the size argument as a template parameter when the map is replaced
    TCPListener<ConnectionBufferSize>& bind(const Endpoint endpoint) {
        auto[it, inserted] = port_listeners.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(endpoint),
            std::forward_as_tuple(endpoint, send_queue, connection_queues, port_listeners)
        );
        assert(inserted);
        return it->second;
    }

private:

    void listener(std::stop_token token) {
        while (!token.stop_requested()) {
            auto buffer = std::make_unique<ByteArr>();
            // TODO get rid of latency incurred by copying
            // TODO if a stop is requested, this call should return. Change blocking somehow.
            (void)interface.receive(*buffer);

            auto& ip = structs::IPv4::from_ptr(buffer->data());
            if (ip.version != 4 || ip.protocol != structs::IPv4::IPPROTOCOL_TCP) {
                continue;
            }

            auto& tcp = ip.tcp_payload();
            Port port = tcp.dest_port();
            IpAddress address = ip.dest_addr_n;
            Endpoint endpoint { address, port };
            if (!port_listeners.contains(endpoint)) {
                continue;
            }

            const auto id = ip.connection_id();

            if (tcp.syn == true) {
                // TODO Change this...
                // TODO have the size be customizable
                connection_queues.create_connection_queue(id);
                auto it = port_listeners.find(endpoint);
                assert(it != port_listeners.end());
                it->second.pending_connections.push(id);
            }

            // TODO Change this...
            const auto it = connection_queues.find(id);
            if (it != connection_queues.end()) {
                it->second.push(std::move(buffer));
            }
        }
    }

    void sender(std::stop_token token) {
        while (!token.stop_requested()) {
            // TODO don't spin
            auto packet = send_queue.pop();
            while (!packet.has_value()) {
                if (token.stop_requested()) return;
                packet = send_queue.pop();
            }
            auto& buffer = *packet.value();
            auto& ip = structs::IPv4::from_ptr(buffer.data());
            (void)interface.send({ buffer.data(), ip.total_len() });
        }
    }

    TunDevice interface;
    // TODO have different argument for queue capacity
    ConnectionQueues<ConnectionBufferSize> connection_queues;
    // TODO this needs to change
    MPSCQueue<ConnectionBufferSize> send_queue;
    // TODO have different argument for queue capacity
    ConcurrentMap<Endpoint, TCPListener<ConnectionBufferSize>> port_listeners;
    std::jthread listen_thread;
    std::jthread send_thread;
};

}