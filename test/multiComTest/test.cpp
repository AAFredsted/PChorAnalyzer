
#include <string>
struct Query {
    Query(const std::string& endpoint) :  endpoint(endpoint) {}
    std::string endpoint;
};
struct Ack {
    Ack(bool valid): valid(valid) {}
    bool valid;

};
struct Response {
    Response(const std::string& data): data(data) {}
    std::string data;
};

class Anne {
public:

private:

};

class Nicholas {
public:

private:

};

int main() {

    return 0;
}