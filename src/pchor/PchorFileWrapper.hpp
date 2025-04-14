#pragma once 
#include <vector>
#include <fcntl.h>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include <print>
#include "cstring"
#include <charconv>

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

    void readChar(char& c) {
        if(read(fd, &c, 1) <= 0){
            throw std::runtime_error("Failed to read character from file");
        }
    }

    char readChar() {
        char c;
        if(read(fd, &c, 1) <= 0){
            throw std::runtime_error("Failed to read character from file");
        }
        return c;
    }
    
    char peekChar() {
        char c = readChar();
        seekBack();
        return c;
    }

    void seekBack(){
        if(lseek(fd, -1, SEEK_CUR) == -1){
            throw std::runtime_error("Failed to rollback during lexing of tokens" + std::string(strerror(errno)));
        }
    }

    std::string readDecl() {
        char buffer[20];
        size_t i = 0;

        readChar(buffer[i]);
        while(buffer[i] != ' ' || buffer[i] != '(' || buffer[i] != ':'){
            i++;
            if(i == 20){
                throw std::runtime_error("Maximum namespace length exceeded");
            }
            readChar(buffer[i]);
        }
        buffer[i] = '\0';
        return std::string(buffer);
    }
    std::string readLiteral(){
        char buffer[20];
        size_t i = 0;
        while(isValid() && std::isdigit(peekChar())) {
            if(i == 20){
                throw std::runtime_error("Maximum literal length exceeded");
            }
             buffer[i] = readChar();
             i++;
        }
        buffer[i] = '\0';
        return std::string(buffer);
    }

    bool isValid() const {
        return fd != -1;
    }

};
} //namespace PchorAST