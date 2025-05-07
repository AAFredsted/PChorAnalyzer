#include <print>
#include <string>
#include <thread>

struct Message {
    explicit Message() : str(""), isRead(true) {}
    explicit Message(std::string str) : str(str), isRead(false) {}
    std::string str;
    bool isRead;
};

class Anne {
public:
    explicit Anne(Message* NicholasMsg) : NicholasMsg(NicholasMsg) {}

    void sendMessage(const std::string& content) {
        Message msg(content);
        *NicholasMsg = msg;
        std::println("Anne: Sent Message -> {}", msg.str);
    }

private:
    Message* NicholasMsg;
};

class Nicholas {
public:
    explicit Nicholas() : msg(Message{}) {}

    Message* getMessagePointer() {
        return &msg;
    }

    void receiveMessage() {
        while (msg.isRead) {
            // Wait for a new message
        }
        std::println("Nicholas: Received Message -> {}", msg.str);
        msg.isRead = true;
    }
private:
    Message msg;
};

int main() {
    // Shared message object
    Message sharedMessage;

    // Create participants
    Nicholas nicholas;
    Anne anne(nicholas.getMessagePointer());

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