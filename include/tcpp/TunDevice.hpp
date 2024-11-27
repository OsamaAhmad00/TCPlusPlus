#pragma once

#include <span>
#include <string>
#include <cstdint>

#include <tcpp/utils/FileDescriptor.hpp>

namespace tcpp {

class TunDevice {
public:

    [[nodiscard]] size_t send(std::span<const uint8_t> buffer) const;

    [[nodiscard]] size_t receive(std::span<uint8_t> buffer) const;

private:

    friend class TunBuilder;
    // Can only be created through a TunBuilder
    TunDevice(FileDescriptor fd_, std::string name_) : fd(std::move(fd_)), name(std::move(name_)) { }

    FileDescriptor fd;
    std::string name;
};

class TunBuilder {
public:

    explicit TunBuilder(std::string name_) : name(std::move(name_)) { }

    TunBuilder& set_ip4(std::string ip4_) { ip4 = std::move(ip4_); return *this; }

    TunBuilder& set_netmask(std::string netmask_) { netmask = std::move(netmask_); return *this; }

    TunDevice build();

private:

    void allocate_tun();

    FileDescriptor fd;
    std::string name;
    std::string ip4;
    std::string netmask;
};

};