#include <string>
#include <print>
#include <thread>
#include <vector>
#include <chrono>

struct Message {
    Message(const std::string& msgString) : msgString(msgString) {}
    std::string msgString;
};

class Sender {
public:
    Sender(int id) : id(id), msg(nullptr) {}

    void addMessage(Message& msg) {
        this->msg = &msg;
    }

    void sendThenReceive(Sender* next, Message& msg) {
        // Send the message to the next sender
        std::println("Sender {} sent Message -> {}", id, msg.msgString);
        next->addMessage(msg);

        // Wait to receive a message
        while (!this->msg) {
            // Spin-wait
        }
        std::println("Sender {} received Message -> {}", id, this->msg->msgString);

        // Clear the message after processing
        this->msg = nullptr;
    }

    void receiveThenSend(Sender* next) {
        // Wait to receive a message
        while (!this->msg) {
            // Spin-wait
        }
        std::println("Sender {} received Message -> {}", id, this->msg->msgString);

        // Send the received message to the next sender
        std::println("Sender {} sent Message -> {}", id, this->msg->msgString);
        next->addMessage(*(this->msg));

        // Clear the message after processing
        this->msg = nullptr;
    }

private:
    int id;
    Message* msg;
};

int main() {
    // Create senders
    Sender sender1(1);
    Sender sender2(2);
    Sender sender3(3);

    // Create the initial message
    Message initialMessage("Hello from Sender 1");

    // Run the protocol in a circular manner using threads
    std::thread t1([&]() {
        sender1.sendThenReceive(&sender2, initialMessage);
    });

    std::thread t2([&]() {
        sender2.receiveThenSend(&sender3);
    });

    std::thread t3([&]() {
        sender3.receiveThenSend(&sender1);
    });

    // Join threads
    t1.join();
    t2.join();
    t3.join();

    return 0;
}