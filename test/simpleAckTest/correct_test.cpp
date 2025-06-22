#include <print>
#include <string>
#include <thread>

struct Message {
    explicit Message() : str(""), isRead(true) {}
    explicit Message(std::string str) : str(str), isRead(false) {}
    std::string str;
    bool isRead;
};

struct Ack {
    explicit Ack(bool ackowledge): acknowledge(ackowledge) {}
    bool acknowledge;
};

class Anne;

class Nicholas {
public:
    explicit Nicholas() : msg(Message{}), anne(nullptr) {}



    Message& getMessage() {
        return msg;
    }

    void recieveAnne(Anne* anne) {
        this->anne = anne;
    }

    void receiveMessage();

    Message msg;
    Anne* anne;
};


class Anne {
public:
    explicit Anne(Nicholas* nicholas) : nicholas(nicholas), ack(false) {}

    void sendMessage(const std::string& content);

    Nicholas* nicholas; 
    Ack ack;
};


void Nicholas::receiveMessage(){
    while (msg.isRead) {
        // Wait for a new message
    }
    msg.isRead=true;
    
    std::println("Nicholas: Received Message -> {}", msg.str);
    anne->ack = Ack{true};
    std::println("Nicholas Sent Acknowledgement");   
}

void Anne::sendMessage(const std::string& content) {
    nicholas->msg = Message{content};// Explicitly update msg in Nicholas
    std::println("Anne Sent Message {}", content);
    while(!ack.acknowledge){
        //spinwait
    } 
    std::println("Anne recieved Acknowledgement");
}

int main() {
    // Create participants
    Nicholas nicholas;
    Anne anne(&nicholas); // Pass a pointer to Nicholas

    nicholas.recieveAnne(&anne);

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