#pragma once

#include <span>
#include <string>
#include <cstdint>

#include <tcpp/utils/FileDescriptor.hpp>

namespace tcpp {

class TunDevice {
public:

    [[nodiscard]] ssize_t send(std::span<const uint8_t> buffer) const;

    [[nodiscard]] ssize_t receive(std::span<uint8_t> buffer) const;

    void close();

    TunDevice(const TunDevice&) = delete;

    TunDevice& operator=(const TunDevice&) = delete;

    TunDevice(TunDevice&&) = default;

    TunDevice& operator=(TunDevice&&) = default;

    ~TunDevice();

private:

    friend class TunBuilder;
    // Can only be created through a TunBuilder
    TunDevice(FileDescriptor fd_, std::string name_) : fd(std::move(fd_)), name(std::move(name_)) { }

    FileDescriptor fd;
    std::string name;

    // This is an alternative to have an atomic fd. This is
    // written to only from the close() function and read
    // in the destructor. As long as these don't happen
    // concurrently, and as long as nothing will try to
    // send or receive after closing (this is not checked
    // in the functions), then  no data race should occur.
    bool already_closed = false;
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