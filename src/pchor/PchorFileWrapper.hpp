#pragma once 
#include <vector>
#include <fcntl.h>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include <print>
#include "cstring"
#include <charconv>
#include "pchor.hpp"

namespace PchorAST {
class PchorFileWrapper {
    int fd;
public:
    explicit PchorFileWrapper(const std::string& filePath) {
        fd = open(filePath.c_str(), O_RDONLY);
        if(fd == -1){
            std::println("Failed to open file {}", strerror(errno));
        }
    }
    ~PchorFileWrapper() {
        if(fd != 1) {
            close(fd);
        }
    }

    PchorFileWrapper(const PchorFileWrapper& other) = delete;
    PchorFileWrapper(PchorFileWrapper&& other) noexcept : fd(other.fd) {
        other.fd = -1;
    }

    PchorFileWrapper& operator=(const PchorFileWrapper& other) = delete;
    PchorFileWrapper& operator=(PchorFileWrapper&& other) noexcept {
        if(this != &other) {
            //close current filewrapper
            if(fd != -1) {
                close(fd);
            }
            fd = other.fd;
            other.fd = -1;
        }
        return *this;
    }

    char readChar() {
        char c;
        if(read(fd, &c, 1) <= 0){
            throw std::runtime_error("Failed to read character from file");
        }
        return c;
    }
    bool isValid() const {
        return fd != -1;
    }

};
} //namespace PchorAST