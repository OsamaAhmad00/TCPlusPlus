#pragma once

#include <utility>
#include <unistd.h>

namespace tcpp {

struct FileDescriptor {
    FileDescriptor() : fd(-1) { }
    FileDescriptor(int fd_) : fd(fd_) { }

    FileDescriptor(const FileDescriptor&) = delete;
    FileDescriptor& operator=(const FileDescriptor&) = delete;

    FileDescriptor(FileDescriptor&& other) noexcept { this->fd = std::exchange(other.fd, -1); }
    FileDescriptor& operator=(FileDescriptor&& other) noexcept { std::swap(fd, other.fd); return *this; }

    FileDescriptor& operator=(int fd_) { (void)~FileDescriptor(); fd = fd_; return *this; }

    operator int() const { return fd; }

    ~FileDescriptor() { if (fd >= 0) close(fd); }

private:
    int fd;
};

};