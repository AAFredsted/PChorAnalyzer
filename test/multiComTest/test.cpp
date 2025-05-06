#include <string>
#include <print>
#include <thread>

struct Query {
    Query(const std::string& endpoint) : endpoint(endpoint) {}
    std::string endpoint;
};

struct Ack {
    Ack(bool valid) : valid(valid) {}
    bool valid;
};

struct Response {
    Response(const std::string& data) : data(data) {}
    std::string data;
};

class Anne {
public:
    Anne() : ack(nullptr), resp(nullptr) {}

    void receiveAck(Ack* ackPtr) {
        ack = ackPtr;
    }

    void receiveResponse(Response* respPtr) {
        resp = respPtr;
    }

    void sendQuery(Query q, Query* nicholasQueue) {
        // Send the query to Nicholas
        *nicholasQueue = q;
        std::print("Anne: Sent Query -> {}\n", q.endpoint);

        // Wait for Ack
        while (ack == nullptr) {
            std::print("Anne: Waiting for Ack...\n");
        }
        std::print("Anne: Received Ack -> {}\n", ack->valid ? "Valid" : "Invalid");

        // Wait for Response
        while (resp == nullptr) {
            std::print("Anne: Waiting for Response...\n");
        }
        std::print("Anne: Received Response -> {}\n", resp->data);
    }

private:
    Ack* ack;
    Response* resp;
};

class Nicholas {
public:
    Nicholas() : query(nullptr) {}

    void waitForQuery() {
        while (query == nullptr) {
            std::print("Nicholas: Waiting for Query...\n");
        }
        std::print("Nicholas: Received Query -> {}\n", query->endpoint);
    }

    void sendAck(Ack* ackPtr, Anne* anne) {
        *ackPtr = Ack(true);
        anne->receiveAck(ackPtr);
        std::print("Nicholas: Sent Ack -> Valid\n");
    }

    void sendResponse(Response* respPtr, Anne* anne) {
        *respPtr = Response("Response from Nicholas");
        anne->receiveResponse(respPtr);
        std::print("Nicholas: Sent Response -> {}\n", respPtr->data);
    }

    void receiveQuery(Query* queryPtr) {
        query = queryPtr;
    }

private:
    Query* query;
};

int main() {
    Anne anne;
    Nicholas nicholas;

    Query query("Request from Anne");
    Query nicholasQueue("");

    Ack ack(false);
    Response response("");

    // Simulate Anne sending a query
    std::thread anneThread([&]() {
        anne.sendQuery(query, &nicholasQueue);
    });

    // Simulate Nicholas receiving the query and responding
    std::thread nicholasThread([&]() {
        nicholas.receiveQuery(&nicholasQueue);
        nicholas.waitForQuery();
        nicholas.sendAck(&ack, &anne);
        nicholas.sendResponse(&response, &anne);
    });

    // Join threads
    anneThread.join();
    nicholasThread.join();

    return 0;
}