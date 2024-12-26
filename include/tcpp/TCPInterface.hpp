#pragma once

#include <map>
#include <thread>

#include <tcpp/TypeDefs.hpp>
#include <tcpp/TunDevice.hpp>
#include <tcpp/TCPListener.hpp>
#include <tcpp/utils/Connections.hpp>
#include <tcpp/data-structures/ConcurrentMap.hpp>
#include <tcpp/allocators/ReusableSlabAllocator.hpp>

namespace tcpp {

template <size_t ConnectionBufferSize = (1 << 20)>
requires PowerOfTwo<ConnectionBufferSize>
class TCPInterface {

public:

    explicit TCPInterface(TunDevice interface)
        : interface(std::move(interface)),
          listener_thread(&TCPInterface::listener, this),
          sender_thread(&TCPInterface::sender, this),
          handler_thread(&TCPInterface::packets_handler, this)
    { }

    // TODO accept the size argument as a template parameter when the map is replaced
    TCPListener<ConnectionBufferSize>& bind(const Endpoint endpoint) {
        auto[it, inserted] = port_listeners.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(endpoint),
            std::forward_as_tuple(endpoint, send_queue, port_listeners)
        );
        assert(inserted);
        return it->second;
    }

private:

    void listener(std::stop_token token) {
        ReusableAllocator allocator;
        while (!token.stop_requested()) {
            auto buffer = allocator.allocate();
            // TODO get rid of latency incurred by copying
            // TODO if a stop is requested, this call should return. Change blocking somehow.
            (void)interface.receive({ buffer, PacketBufferSize });

            auto& ip = structs::IPv4::from_ptr(buffer);
            if (ip.version != 4 || ip.protocol != structs::IPv4::IPPROTOCOL_TCP) {
                continue;
            }

            auto& tcp = ip.tcp_payload();
            Port port = tcp.dest_port();
            IpAddress address = ip.dest_addr_n;
            Endpoint endpoint { address, port };
            auto listener = port_listeners.find(endpoint);
            if (listener == port_listeners.end()) {
                continue;
            }

            auto id = ip.connection_id();
            auto connection_it = connections.find(id);

            if (connection_it == connections.end()) {
                // there is only a single listener (this thread). there can be multiple threads accessing
                //  the listener, but they can only increment the counter. this means that if the counter
                //  is > 0, then we can proceed without worrying about other threads decrementing the counter
                //  and having us stuck in the CAS loop down there.
                if (tcp.syn && listener->second.acceptable_connections > 0) {
                    // New connection
                    // TODO memory order
                    // TODO insure that this is a valid new connection
                    auto [new_connection, inserted] = connections.emplace(
                        std::piecewise_construct,
                        std::forward_as_tuple(id),
                        std::forward_as_tuple(id, send_queue)
                    );
                    assert(inserted);
                    received_packets.push(buffer);
                    // TODO will there ever be a situation in which multiple
                    //  threads are accessing this value concurrently?
                    // TODO memory orders
                    listener->second.acceptable_connections--;
                    auto& to_return = listener->second.connection_to_return;
                    assert(to_return == nullptr);
                    to_return = &new_connection->second;
                    // TODO Optimal? the listener has to have a very low latency.
                    to_return.notify_one();
                } else {
                    // Not waiting for a new connection, drop the packet.
                }
            } else {
                received_packets.push(buffer);
            }
        }
    }

    void sender(std::stop_token token) {
        ReusableAllocator allocator;
        while (!token.stop_requested()) {
            // TODO don't spin
            auto packet = send_queue.pop();
            while (!packet.has_value()) {
                if (token.stop_requested()) return;
                packet = send_queue.pop();
            }
            auto buffer = packet.value();
            auto& ip = structs::IPv4::from_ptr(buffer);
            (void)interface.send({ buffer, ip.total_len() });
            allocator.deallocate(buffer);
        }
    }

    void packets_handler(std::stop_token token) {
        while (!token.stop_requested()) {
            // TODO stop spinning?
            auto packet = received_packets.pop();
            if (!packet.has_value()) continue;
            auto& ip = structs::IPv4::from_ptr(packet.value());
            auto id = ip.connection_id();
            auto connection_it = connections.find(id);
            assert(connection_it != connections.end());
            connection_it->second.process_packet(packet.value());
            if (connection_it->second.connection_closed) {
                // TODO erase when actually appropriate
                connections.erase(id);
            }
        }
    }

    TunDevice interface;
    // TODO change this to SPMC
    SPSCBoundedWaitFreeQueue<uint8_t*, ConnectionBufferSize> received_packets;
    // TODO this needs to change
    MPSCBoundedQueue<PacketBuffer, ConnectionBufferSize> send_queue;
    // TODO have different argument for queue capacity
    ConcurrentMap<Endpoint, TCPListener<ConnectionBufferSize>> port_listeners;
    ConcurrentMap<ConnectionID, TCPConnection<ConnectionBufferSize>> connections;
    std::jthread listener_thread;
    std::jthread sender_thread;
    std::jthread handler_thread;
};

}
