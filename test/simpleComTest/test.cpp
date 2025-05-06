#include <print>
#include <string>
#include <vector>

struct Message {
    explicit Message(): str("") {}
    explicit Message(std::string str): str(str) {}
    std::string str;
};

class Anne {
public:
    Message* msg;
    
    explicit Anne(): msg() {}
    
    Message getMessage(){
        return *msg;
    }

    void dosomething() {
        msg->str = "hello";
    }

};

class Nicholas {
public:
    const Message* msg;

    explicit Nicholas(): msg() {}

    Message getMessage(){
        Message a{};
        return *msg;
    }


};


int main(){
    std::println("Hello Worlds");
}