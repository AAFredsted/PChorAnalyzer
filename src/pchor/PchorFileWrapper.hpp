#pragma once
#include <fcntl.h>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include <stdexcept>
#include <cstring>
#include <cctype>

namespace PchorAST {
class PchorFileWrapper {
    std::string buffer;

public:
    explicit PchorFileWrapper(const std::string& filePath) {
        int fd = open(filePath.c_str(), O_RDONLY);
        if (fd == -1) {
            throw std::runtime_error("Failed to open file: " + std::string(strerror(errno)));
        }

        // Get the file size
        off_t fileSize = lseek(fd, 0, SEEK_END);
        if (fileSize == -1) {
            close(fd);
            throw std::runtime_error("Failed to determine file size: " + std::string(strerror(errno)));
        }
        lseek(fd, 0, SEEK_SET); // Reset file descriptor to the beginning

        // Read the entire file into a temporary buffer
        std::string tempBuffer(fileSize, '\0');
        ssize_t bytesRead = read(fd, tempBuffer.data(), fileSize);
        if (bytesRead < 0) {
            close(fd);
            throw std::runtime_error("Failed to read file: " + std::string(strerror(errno)));
        }
        close(fd);

        // Process the buffer to preserve spaces between tokens and handle comments
        bool inComment = false;
        bool lastWasSpace = false;
        for (size_t i = 0; i < tempBuffer.size(); ++i) {
            char c = tempBuffer[i];

            if (inComment) {
                if (c == '\n') {
                    inComment = false;
                    buffer.push_back(c);
                }
                continue;
            }

            if (c == '/' && i + 1 < tempBuffer.size() && tempBuffer[i + 1] == '/') {
                inComment = true;
                ++i;
                continue;
            }

            if (std::isspace(c)) {
                if (c == '\n') {
                    buffer.push_back(c);
                    lastWasSpace = false;
                } else if (!lastWasSpace) {
                    buffer.push_back(' ');
                    lastWasSpace = true;
                }
            } else {
                buffer.push_back(c);
                lastWasSpace = false;
            }
        }
        buffer.push_back(' ');
    }

    const std::string& getBuffer() const {
        return buffer;
    }
};
} // namespace PchorAST