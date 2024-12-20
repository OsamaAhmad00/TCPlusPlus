#include <string>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <iostream>

#include <tcpp/TunDevice.hpp>
#include <tcpp/utils/FileDescriptor.hpp>

namespace tcpp {

size_t TunDevice::receive(std::span<uint8_t> buffer) const {
    const ssize_t n = read(fd, buffer.data(), buffer.size());
    if (n < 0) throw std::runtime_error("Reading from TUN device failed");
    return static_cast<size_t>(n);
}

size_t TunDevice::send(std::span<const uint8_t> buffer) const {
    const ssize_t n = write(fd, buffer.data(), buffer.size());
    if (n < 0) throw std::runtime_error("Writing to TUN device failed");
    return static_cast<size_t>(n);
}

void TunBuilder::allocate_tun() {
    if (name.size() > IFNAMSIZ) {
        throw std::invalid_argument("Device name is too long");
    }

    fd = open("/dev/net/tun", O_RDWR);
    if (fd < 0) throw std::runtime_error("Opening /dev/net/tun");

    ifreq ifr { };
    ifr.ifr_flags = IFF_TUN | IFF_NO_PI;
    // TODO correct?
    if (!name.empty()) std::strncpy(ifr.ifr_name, name.data(), name.size() + 1);

    if (ioctl(fd, TUNSETIFF, &ifr) < 0) {
        throw std::runtime_error("ioctl(TUNSETIFF)");
    }

    if (name.empty()) name = ifr.ifr_name;
}

struct TunBuilderHelper {

    TunBuilderHelper(const std::string& name_, const std::string& ip4_, const std::string& netmask_)
        : name(name_), ip4(ip4_), netmask(netmask_) {}

    void build() {
        sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
        if (sock_fd < 0) throw std::runtime_error("Opening socket failed");

        // TODO correct?
        std::strncpy(ifr.ifr_name, name.data(), name.size() + 1);

        addr = reinterpret_cast<sockaddr_in*>(&ifr.ifr_addr);

        set_ip4();
        set_netmask();
        bring_up();
    }

    void set_ip4() {
        if (ip4.empty()) return;

        addr->sin_family = AF_INET;
        if (inet_pton(AF_INET, ip4.data(), &addr->sin_addr) <= 0)
            throw std::runtime_error("inet_pton for IP failed");

        if (ioctl(sock_fd, SIOCSIFADDR, &ifr) < 0)
            throw std::runtime_error("ioctl(SIOCSIFADDR) failed");
    }

    void set_netmask() {
        if (netmask.empty()) return;

        if (inet_pton(AF_INET, netmask.data(), &addr->sin_addr) <= 0)
            throw std::runtime_error("inet_pton for netmask failed");

        if (ioctl(sock_fd, SIOCSIFNETMASK, &ifr) < 0)
            throw std::runtime_error("ioctl(SIOCSIFNETMASK) failed");
    }

    void bring_up() {
        if (ioctl(sock_fd, SIOCGIFFLAGS, &ifr) < 0)
            throw std::runtime_error("ioctl(SIOCGIFFLAGS) failed");

        ifr.ifr_flags |= IFF_UP | IFF_RUNNING;
        if (ioctl(sock_fd, SIOCSIFFLAGS, &ifr) < 0)
            throw std::runtime_error("ioctl(SIOCSIFFLAGS) failed");
    }

    const std::string& name;
    const std::string& ip4;
    const std::string& netmask;

    FileDescriptor sock_fd { };
    ifreq ifr { };
    sockaddr_in* addr { };
};

TunDevice TunBuilder::build() {
    allocate_tun();
    TunBuilderHelper helper { name, ip4, netmask };
    helper.build();
    return TunDevice { std::move(fd), helper.ifr.ifr_name };
}

}