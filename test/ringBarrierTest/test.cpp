#include <string>
#include <thread>
#include <chrono>
#include <vector>
#include <thread>
#include <print>

#include <mutex>
std::mutex print_mutex;


struct Ping {
    bool isPinged;
    Ping(): isPinged(true) {}
    Ping(bool isPinged): isPinged(isPinged) {}
};


class Process {
public:
    Process(size_t pID): pID(pID), p(true) {}

    void pingMe(){
        p = Ping{};
    }
    void setNext(Process* nextProcess){
        this->nextProcess = nextProcess;
    }

    bool hasReceived() {
        return p.isPinged;
    }

    void receiveThenSend(){
        while(!p.isPinged){
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        {
            std::lock_guard<std::mutex> lock(print_mutex);
            std::println("Process {} received ping", pID);
        }
        nextProcess->pingMe();
        {
            std::lock_guard<std::mutex> lock(print_mutex);
            std::println("Process {} send ping", pID);
        }
    }
    void sendThenReceive(){
        {
            std::lock_guard<std::mutex> lock(print_mutex);
            std::println("Process {} send ping", pID);
        }
        nextProcess->pingMe();

        while(!p.isPinged){
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        {
            std::lock_guard<std::mutex> lock(print_mutex);
            std::println("Process {} received ping", pID);
        }
    }

private:
    size_t pID;
    Ping p;
    Process* nextProcess;
};



int main(int argc, char const *argv[])
{
    if(argc != 2) {
        return 1;
    }
    size_t N = std::stoul(argv[1]);

    std::vector<Process> processes{};
    processes.reserve(N);

    for(size_t i = 0; i < N; ++i) {
        processes.emplace_back(Process{i});
    }

    for(size_t i = 0; i < (N-1); ++i) {
        processes[i].setNext(&processes[i+1]);
    }
    processes[N-1].setNext(&processes[0]);


    std::vector<std::thread> threads;
    threads.reserve(N);

    // Start threads for processes 1 to N-1
    for(size_t i = 1; i < N; ++i) {
        threads.emplace_back([&processes, i]() {
            processes[i].receiveThenSend();
        });
    }

    // Start thread for process 0
    threads.emplace_back([&processes]() {
        processes[0].sendThenReceive();
    });

    while(!processes[0].hasReceived()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // Join all threads
    for(auto& t : threads) t.join();
    
    return 0;
}
