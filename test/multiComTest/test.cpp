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
    Anne() : ack(Ack(false)), resp(Response("")), nicholasQuery(nullptr) {}

    void setNicholasQuery(Query* query) {
        nicholasQuery = query;
    }

    void recieveAck(const Ack& acknowledgement) {
        ack = acknowledgement;
    }

    void recieveResp(const Response& response) {
        resp = response;
    }

    void run() {
        thread = std::thread([this]() {
            sendQuery("Request from Anne");
        });
    }

    void join() {
        if (thread.joinable()) {
            thread.join();
        }
    }

    void sendQuery(const std::string& endpoint) {
        // Send the query to Nicholas
        *nicholasQuery = Query{endpoint};
        std::print("Anne: Sent Query -> {}\n", nicholasQuery->endpoint);

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

private:
    Ack ack;
    Response resp;
    Query* nicholasQuery;
    std::thread thread;
};

class Nicholas {
public:
    Nicholas() : query(nullptr) {}

    void setQuery(Query* queryPtr) {
        query = queryPtr;
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

    void respondToQuery(Anne* anne) {
        while (!query || query->endpoint.empty()) {
            std::print("Nicholas: Waiting for Query...\n");
        }
        std::print("Nicholas: Received Query -> {}\n", query->endpoint);

        anne->recieveAck(Ack(true));
        std::print("Nicholas: Sent Ack -> Valid\n");

        Response resp = Response("Response from Nicholas");
        anne->recieveResp(resp);
        std::print("Nicholas: Sent Response -> {}\n", resp.data);
    }

private:
    Query* query;
    std::thread thread;
};

int main() {
    Anne anne;
    Nicholas nicholas;

    Query sharedQuery("");

    // Set up shared query pointer
    anne.setNicholasQuery(&sharedQuery);
    nicholas.setQuery(&sharedQuery);

    // Run threads
    anne.run();
    nicholas.run(&anne);

    // Join threads
    anne.join();
    nicholas.join();

    return 0;
}