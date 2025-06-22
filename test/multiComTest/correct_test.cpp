#include <string>
#include <print>
#include <thread>

struct Query {
    Query(const std::string& endpoint, bool isProcessed) : endpoint(endpoint), isProcessed(isProcessed) {}
    std::string endpoint;
    bool isProcessed;

};

struct Ack {
    Ack(bool valid) : valid(valid) {}
    bool valid;
};

struct Response {
    Response(const std::string& data) : data(data) {}
    std::string data;
};

class Nicholas;

class Anne {
public:
    Anne() : ack(Ack(false)), resp(Response("")) {}

    void recieveAck(const Ack& acknowledgement) {
        ack = acknowledgement;
    }

    void recieveResp(const Response& response) {
        resp = response;
    }

    void run(Nicholas* nicholas) {
        thread = std::thread([this, nicholas]() {
            sendQuery(nicholas, "Request from Anne");
        });
    }

    void join() {
        if (thread.joinable()) {
            thread.join();
        }
    }

    void sendQuery(Nicholas* nicholas, const std::string& endpoint);

private:
    Ack ack;
    Response resp;
    std::thread thread;
};

class Nicholas {
public:
    Nicholas() : query("", true) {}

    void recieveQuery(Query& newQuery) {
        query = newQuery;
    }

    void run(Anne* anne) {
        thread = std::thread([this, anne]() {
            respondToQuery(anne);
        });
    }

    void join() {
        if (thread.joinable()) {
            thread.join();
        }
    }

    void respondToQuery(Anne* anne);

private:
    Query query;
    std::thread thread;
};

void Anne::sendQuery( Nicholas* nicholas, const std::string& endpoint) {
    // Send the query to Nicholas
    Query q{endpoint, false};
    nicholas->recieveQuery(q);

    std::print("Anne: Sent Query -> {}\n", q.endpoint);

    // Wait for Ack
    while (!ack.valid) {
        std::print("Anne: Waiting for Ack...\n");
    }
    std::print("Anne: Received Ack -> {}\n", ack.valid ? "Valid" : "Invalid");

    // Wait for Response
    while (resp.data.empty()) {
        std::print("Anne: Waiting for Response...\n");
    }
    std::print("Anne: Received Response -> {}\n", resp.data);
}

void Nicholas::respondToQuery(Anne* anne) {
    while (!query.isProcessed || query.endpoint.empty()) {
        std::print("Nicholas: Waiting for Query...\n");
    }
    std::print("Nicholas: Received Query -> {}\n", query.endpoint);

    anne->recieveAck(Ack(true));
    std::print("Nicholas: Sent Ack -> Valid\n");

    Response resp = Response("Response from Nicholas");
    anne->recieveResp(resp);
    std::print("Nicholas: Sent Response -> {}\n", resp.data);
}

int main() {
    Anne anne;
    Nicholas nicholas;

    // Run threads
    anne.run(&nicholas);
    nicholas.run(&anne);

    // Join threads
    anne.join();
    nicholas.join();

    return 0;
}