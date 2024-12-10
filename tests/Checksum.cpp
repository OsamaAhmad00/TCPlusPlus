#include <gtest/gtest.h>

#include <tcpp/structs/IPv4.hpp>
#include <tcpp/structs/UDP.hpp>
#include <tcpp/structs/TCP.hpp>

/*
 * 10.0.0.1:52716 -> 10.0.0.2:4000
 * IP checksum: 0xc927
 * UDP checksum: 0xea66
 * Payload: "Hello\n"
*/
uint8_t udp_packet[] = { 0x45, 0x0, 0x0, 0x22, 0x5d, 0xa1, 0x40, 0x0, 0x40, 0x11, 0xc9, 0x27, 0xa, 0x0,
    0x0, 0x1, 0xa, 0x0, 0x0, 0x2, 0xcd, 0xec, 0xf, 0xa0, 0x0, 0xe, 0xea, 0x66, 0x48, 0x65, 0x6c, 0x6c, 0x6f, 0xa };

/*
 * Internet Protocol Version 4, Src: 127.0.0.1, Dst: 127.0.0.1
 *     0100 .... = Version: 4
 *     .... 0101 = Header Length: 20 bytes (5)
 *     Differentiated Services Field: 0x00 (DSCP: CS0, ECN: Not-ECT)
 *         0000 00.. = Differentiated Services Codepoint: Default (0)
 *         .... ..00 = Explicit Congestion Notification: Not ECN-Capable Transport (0)
 *     Total Length: 58
 *     Identification: 0x8f85 (36741)
 *     010. .... = Flags: 0x2, Don't fragment
 *     ...0 0000 0000 0000 = Fragment Offset: 0
 *     Time to Live: 64
 *     Protocol: TCP (6)
 *     Header Checksum: 0xad36 [correct]
 *     [Header checksum status: Good]
 *     [Calculated Checksum: 0xad36]
 *     Source Address: 127.0.0.1
 *     Destination Address: 127.0.0.1
 *     [Stream index: 0]
 * Transmission Control Protocol, Src Port: 53428, Dst Port: 5000, Seq: 1, Ack: 1, Len: 6
 *     Source Port: 53428
 *     Destination Port: 5000
 *     [Stream index: 2]
 *     [Conversation completeness: Complete, WITH_DATA (31)]
 *     [TCP Segment Len: 6]
 *     Sequence Number: 1    (relative sequence number)
 *     Sequence Number (raw): 2461807680
 *     [Next Sequence Number: 7    (relative sequence number)]
 *     Acknowledgment Number: 1    (relative ack number)
 *     Acknowledgment number (raw): 1850889381
 *     1000 .... = Header Length: 32 bytes (8)
 *     Flags: 0x018 (PSH, ACK)
 *     Window: 512
 *     [Calculated window size: 65536]
 *     [Window size scaling factor: 128]
 *     ==================== I MANUALLY MODIFIED THE CHECKSUM TO THE CORRECT VALUE (0x1768) ====================
 *     Checksum: 0x1768
 *     // Checksum: 0xfe2e [correct] (matches partial checksum, not 0x1768, likely caused by "TCP checksum offload")
 *     [Checksum Status: Good]
 *     Urgent Pointer: 0
 *     Options: (12 bytes), No-Operation (NOP), No-Operation (NOP), Timestamps
 *         TCP Option - No-Operation (NOP)
 *         TCP Option - No-Operation (NOP)
 *         TCP Option - Timestamps: TSval 2781432681, TSecr 2781427259
 *     [Timestamps]
 *     [SEQ/ACK analysis]
 *     TCP payload (6 bytes)
 * Data (6 bytes)
 *     Data: 48656c6c6f0a ("Hello\n")
 *     [Length: 6]
*/
uint8_t tcp_packet[] = { 0x45, 0x0, 0x0, 0x3a, 0x8f, 0x85, 0x40, 0x0, 0x40, 0x6, 0xad, 0x36, 0x7f, 0x0, 0x0, 0x1,
    0x7f, 0x0, 0x0, 0x1, 0xd0, 0xb4, 0x13, 0x88, 0x92, 0xbc, 0x34, 0x40, 0x6e, 0x52, 0x54, 0xa5, 0x80, 0x18, 0x2,
    0x0, 0x17, 0x68, 0x0, 0x0, 0x1, 0x1, 0x8, 0xa, 0xa5, 0xc9, 0x4b, 0x69, 0xa5, 0xc9, 0x36, 0x3b, 0x48, 0x65,
    0x6c, 0x6c, 0x6f, 0xa };

/*
 * Internet Protocol Version 4, Src: 10.0.0.2, Dst: 10.0.0.1
 *     0100 .... = Version: 4
 *     .... 0101 = Header Length: 20 bytes (5)
 *     Differentiated Services Field: 0x00 (DSCP: CS0, ECN: Not-ECT)
 *     Total Length: 73  <=====================================================  Odd packet size
 *     Identification: 0xb3d5 (46037)
 *     010. .... = Flags: 0x2, Don't fragment
 *     ...0 0000 0000 0000 = Fragment Offset: 0
 *     Time to Live: 64
 *     Protocol: TCP (6)
 *     Header Checksum: 0x72d7 [correct]
 *     [Header checksum status: Good]
 *     [Calculated Checksum: 0x72d7]
 *     Source Address: 10.0.0.2
 *     Destination Address: 10.0.0.1
 *     [Stream index: 3]
 * Transmission Control Protocol, Src Port: 4000, Dst Port: 52656, Seq: 1, Ack: 1, Len: 13
 *     Source Port: 4000
 *     Destination Port: 52656
 *     [Stream index: 0]
 *     [Conversation completeness: Complete, WITH_DATA (31)]
 *     [TCP Segment Len: 13]
 *     Sequence Number: 1    (relative sequence number)
 *     Sequence Number (raw): 1
 *     [Next Sequence Number: 14    (relative sequence number)]
 *     Acknowledgment Number: 1    (relative ack number)
 *     Acknowledgment number (raw): 2576583289
 *     1010 .... = Header Length: 40 bytes (10)
 *     Flags: 0x010 (ACK)
 *     Window: 64240
 *     [Calculated window size: 8222720]
 *     [Window size scaling factor: 128]
 *     Checksum: 0xc9b3 [correct]
 *         [Calculated Checksum: 0xc9b3]
 *     [Checksum Status: Good]
 *     Urgent Pointer: 0
 *     Options: (20 bytes), Maximum segment size, SACK permitted, Timestamps, No-Operation (NOP), Window scale
 *         TCP Option - Maximum segment size: 1460 bytes
 *             [Expert Info (Warning/Protocol): The non-SYN packet does contain a MSS option]
 *             Kind: Maximum Segment Size (2)
 *             Length: 4
 *             MSS Value: 1460
 *         TCP Option - SACK permitted
 *             [Expert Info (Warning/Protocol): The non-SYN packet does contain a SACK PERM option]
 *             Kind: SACK Permitted (4)
 *             Length: 2
 *         TCP Option - Timestamps: TSval 4071497537, TSecr 0
 *             Kind: Time Stamp Option (8)
 *             Length: 10
 *             Timestamp value: 4071497537
 *             Timestamp echo reply: 0
 *         TCP Option - No-Operation (NOP)
 *         TCP Option - Window scale: 7 (multiply by 128)
 *     [Timestamps]
 *     [SEQ/ACK analysis]
 *     TCP payload (13 bytes)
 * Data (13 bytes)
 *     Data: 48656c6c6f20576f726c64210a ("Hello World!\n")
 *     [Length: 13]
 */
uint8_t odd_size_tcp_packet[] = { 0x45, 0x0, 0x0, 0x49, 0xb3, 0xd5, 0x40, 0x0, 0x40, 0x6, 0x72, 0xd7, 0xa, 0x0, 0x0,
    0x2, 0xa, 0x0, 0x0, 0x1, 0xf, 0xa0, 0xcd, 0xb0, 0x0, 0x0, 0x0, 0x1, 0x99, 0x93, 0x8a, 0x79, 0xa0, 0x10, 0xfa, 0xf0,
    0xc9, 0xb3, 0x0, 0x0, 0x2, 0x4, 0x5, 0xb4, 0x4, 0x2, 0x8, 0xa, 0xf2, 0xae, 0x1f, 0x41, 0x0, 0x0, 0x0, 0x0, 0x1, 0x3,
    0x3, 0x7, 0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x20, 0x57, 0x6f, 0x72, 0x6c, 0x64, 0x21, 0xa };

TEST(checksum, IPv4HeaderChecksum1) {
    auto& ip = tcpp::structs::IPv4::from_ptr(udp_packet);
    ASSERT_TRUE(ip.has_valid_checksum());
    const auto old = ip.checksum();
    ASSERT_EQ(old, 0xc927);
    ip.compute_and_set_checksum();
    ASSERT_EQ(ip.checksum(), 0xc927);
}

TEST(checksum, IPv4HeaderChecksum2) {
    auto& ip = tcpp::structs::IPv4::from_ptr(tcp_packet);
    ASSERT_TRUE(ip.has_valid_checksum());
    const auto old = ip.checksum();
    ASSERT_EQ(old, 0xad36);
    ip.compute_and_set_checksum();
    ASSERT_EQ(ip.checksum(), 0xad36);
}

TEST(checksum, UDPHeaderChecksum) {
    auto& ip = tcpp::structs::IPv4::from_ptr(udp_packet);
    auto& udp = ip.udp_payload();
    ASSERT_TRUE(ip.has_valid_udp_checksum());
    const auto old = udp.checksum();
    ASSERT_EQ(old, 0xea66);
    ip.compute_and_set_udp_checksum();
    ASSERT_EQ(udp.checksum(), 0xea66);
}

TEST(checksum, TCPHeaderChecksum) {
    auto& ip = tcpp::structs::IPv4::from_ptr(tcp_packet);
    auto& tcp = ip.tcp_payload();
    ASSERT_TRUE(ip.has_valid_tcp_checksum());
    const auto old = tcp.checksum();
    ASSERT_EQ(old, 0x1768);
    ip.compute_and_set_tcp_checksum();
    ASSERT_EQ(tcp.checksum(), 0x1768);
}

TEST(checksum, OddSizeTCPHeaderChecksum) {
    auto& ip = tcpp::structs::IPv4::from_ptr(odd_size_tcp_packet);
    auto& tcp = ip.tcp_payload();
    ASSERT_TRUE(ip.has_valid_tcp_checksum());
    const auto old = tcp.checksum();
    ASSERT_EQ(old, 0xc9b3);
    ip.compute_and_set_tcp_checksum();
    ASSERT_EQ(tcp.checksum(), 0xc9b3);
}
