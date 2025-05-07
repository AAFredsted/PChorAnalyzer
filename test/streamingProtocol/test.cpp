#include <string>
#include <queue>
#include <print>
#include <thread>

struct Data {
    std::string data;
    Data(const std::string& data) : data(data) {}
};

struct Key {
    const std::string key;
    Key(const std::string& key) : key(key) {}
};

class Kernel {
public:
    Kernel() : dataQueue(), keyQueue(), run(false) {}

    void stopRun() {
        run = false;
    }

    void sendData(std::queue<Data>* consumerQueue) {
        run = true;
        while (run) {
            if (!dataQueue.empty() && !keyQueue.empty()) {
                // Get the front elements of the queues
                Data data = dataQueue.front();
                Key key = keyQueue.front();

                std::println("Kernel received key and data -> Key: {}, Data: {}", key.key, data.data);

                // Pop the elements from the queues
                dataQueue.pop();
                keyQueue.pop();
                std::println("Kernel popped key and data");

                // Push the data to the consumer queue
                std::println("Kernel: Sent Data to Consumer -> {}", data.data);
                consumerQueue->push(data);
            }
        }
    }

    std::queue<Data>* getDataQueue() {
        return &dataQueue;
    }

    std::queue<Key>* getKeyQueue() {
        return &keyQueue;
    }

private:
    std::queue<Data> dataQueue;
    std::queue<Key> keyQueue;
    bool run;
};

class DataProducer {
public:
    DataProducer(std::vector<Data> data) : data(data) {}

    void sendData(std::queue<Data>* dataQueuePtr) {
        for (Data& d : data) {
            std::println("DataProducer: Sent Data -> {}", d.data);
            dataQueuePtr->push(d);
        }
    }

private:
    std::vector<Data> data;
};

class KeyProducer {
public:
    KeyProducer(std::vector<Key> keys) : keys(keys) {}

    void sendKeys(std::queue<Key>* keyQueuePtr) {
        for (Key& k : keys) {
            std::println("KeyProducer: Sent Key -> {}", k.key);
            keyQueuePtr->push(k);
        }
    }

private:
    std::vector<Key> keys;
};

class Consumer {
public:
    Consumer() : consumerQueue(), run(false) {}

    std::queue<Data>* getPtrToQueue() {
        return &consumerQueue;
    }

    void stopRun() {
        run = false;
    }

    void receiveData() {
        run = true;
        while (run) {
            if (!consumerQueue.empty()) {
                Data data = consumerQueue.front();
                consumerQueue.pop();
                std::println("Consumer: Received Data -> {}", data.data);
            }
        }
    }

private:
    std::queue<Data> consumerQueue;
    bool run;
};

int main() {
    // Shared queues
    Kernel kernel;
    Consumer consumer;
    DataProducer dataProducer({Data("Data1"), Data("Data2"), Data("Data3")});
    KeyProducer keyProducer({Key("Key1"), Key("Key2"), Key("Key3")});

    // Threads for each participant
    std::thread dataProducerThread([&]() {
        dataProducer.sendData(kernel.getDataQueue());
    });

    std::thread keyProducerThread([&]() {
        keyProducer.sendKeys(kernel.getKeyQueue());
    });

    std::thread kernelThread([&]() {
        kernel.sendData(consumer.getPtrToQueue());
    });

    std::thread consumerThread([&]() {
        consumer.receiveData();
    });

    // Join producer threads
    dataProducerThread.join();
    keyProducerThread.join();

    // Wait for the kernel to process all data and keys
    while (!kernel.getDataQueue()->empty() || !kernel.getKeyQueue()->empty()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // Stop the kernel
    kernel.stopRun();

    // Wait for the consumer to process all data
    while (!consumer.getPtrToQueue()->empty()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // Stop the consumer
    consumer.stopRun();

    // Join kernel and consumer threads
    kernelThread.join();
    consumerThread.join();

    return 0;
}