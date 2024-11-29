#include <sstream>
#include <tcpp/utils/Formatting.hpp>
#include <tcpp/structs/IPv4.hpp>
#include <tcpp/structs/TCP.hpp>
#include <tcpp/structs/UDP.hpp>

namespace tcpp {

std::string packet_info(const structs::IPv4& ip) {
    if (ip.version != 4) return "Not an IPv4 packet";

    std::stringstream ss;

    ss << "From " << ip.source_ip() << " to " << ip.dest_ip() << '\n';
    ss << "IP checksum: " << ip.checksum() << '\n';

    if (ip.protocol == structs::IPv4::IPPROTOCOL_TCP) {
        auto& tcp = ip.tcp_payload();
        auto payload_size = ip.payload_size() - tcp.payload_offset();
        ss << "Protocol: TCP (" << +ip.protocol << ")\n";
        ss << "Ports " << tcp.source_port() << " -> " << tcp.dest_port() << '\n';
        ss << "Sequence number: " << tcp.seq_num() << '\n';
        ss << "Acknowledgement number: " << tcp.ack_num() << '\n';
        ss << "urg: " << +tcp.urg << ", ack: " << +tcp.ack << ", psh: " << +tcp.psh <<
            ", rst: " << +tcp.rst << ", syn: " << tcp.syn << ", fin: " << tcp.fin << '\n';
        ss << "Window size: " << tcp.window_size() << '\n';
        ss << "Checksum: " << tcp.checksum() << '\n';
        ss << "Urgent Pointer: " << tcp.urgent_ptr() << '\n';
        ss << "Size: " << payload_size << '\n';
        ss << "Data:\n\"\n";
        for (auto r : tcp.payload(payload_size)) ss << r;
        ss << "\"";
    } else if (ip.protocol == structs::IPv4::IPPROTOCOL_UDP) {
        auto& udp = ip.udp_payload();
        ss << "Protocol: UDP (" << +ip.protocol << ")\n";
        ss << "Ports " << udp.source_port() << " -> " << udp.dest_port() << '\n';
        ss << "Checksum: " << udp.checksum() << '\n';
        ss << "Size: " << udp.payload_size() << '\n';
        ss << "Data:\n\"\n";
        for (auto r : udp.payload()) ss << r;
        ss << "\"";
    }

    return ss.str();
}

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