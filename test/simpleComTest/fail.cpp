#include <print>
#include <string>
#include <thread>

struct Message {
    explicit Message() : str(""), isRead(true) {}
    explicit Message(std::string str) : str(str), isRead(false) {}
    std::string str;
    bool isRead;
};

class Nicholas {
public:
    explicit Nicholas() : msg(Message{}) {}

    Message& getMessage() {
        return msg;
    }

    void receiveMessage() {
        while (msg.isRead) {
            // Wait for a new message
        }
        std::println("Nicholas: Received Message -> {}", msg.str);
    }

    Message msg;
};


class Anne {
public:
    explicit Anne(Nicholas* nicholas) : nicholas(nicholas) {}

    void sendMessage(const std::string& content) {
        thismsg = Message{content};// Explicitly update msg in Nicholas

    }

    Nicholas* nicholas; 
    Message thismsg;
};

int main() {
    // Create participants
    Nicholas nicholas;
    Anne anne(&nicholas); // Pass a pointer to Nicholas

    // Simulate the choreography
    std::thread anneThread([&]() {
        anne.sendMessage("Hello, Nicholas!");
    });

    std::thread nicholasThread([&]() {
        nicholas.receiveMessage();
    });

    // Join threads
    anneThread.join();
    nicholasThread.join();

    return 0;
}