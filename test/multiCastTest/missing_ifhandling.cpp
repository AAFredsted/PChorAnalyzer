#include <string>
#include <print>
#include <vector>
#include <thread>

struct Order{
    std::string item;
    unsigned quantity;
    bool fullfilled; 

    Order(): item(""), quantity(), fullfilled(true) {}
    Order(const std::string& s, unsigned q): item(s), quantity(q) {}

};

class Worker {
public:

Worker(): order(), done(false) {}

void receiveOrder(Order neworder){
    order = neworder;
}

void run() {
    thread = std::thread([this]() {
        processOrder();
    });
}

void processOrder() {
    while(order.fullfilled){
        //spinwait
    }
    std::println("Order {} with quantity {}", order.item, order.quantity);
    done = true;
}

Order order;
bool done;
std::thread thread;

};

class BroadCaster {
public:


explicit BroadCaster(std::vector<Worker>& workerArr): workerArr(workerArr) {}


void run(Order* order) {
    thread = std::thread([this, order]() {
        broadCast5(*order);
    });
}
void broadCast(Order order) {
    for(Worker& worker : workerArr){
        worker.receiveOrder(order);
    }
}

void broadCast5(Order order) {
    if(workerArr.size() >= 5){
        workerArr[0].receiveOrder(order);
        workerArr[1].receiveOrder(order);
        workerArr[2].receiveOrder(order);
        workerArr[3].receiveOrder(order);
        workerArr[4].receiveOrder(order);
        workerArr[5].receiveOrder(order);
    }

}

std::vector<Worker>& workerArr;
std::thread thread;
};


bool isDone(const std::vector<Worker>& workerArr) {
    for(const Worker& worker : workerArr){
        if(!worker.done){
            return false;
        }
    }
    return true;
}
int main(int argc, char const *argv[])
{
    std::vector<Worker> workerArr{5};

    BroadCaster bCast(workerArr);

    for(Worker& worker : workerArr){
        worker.run();
    }

    Order order{"Payamas", 11};

    bCast.run(&order);


    while(!isDone(workerArr)){
        //spinwait
    }
    
    for(Worker& worker : workerArr){
        worker.thread.join();
    }
    bCast.thread.join();

    /* code */
    return 0;
}
