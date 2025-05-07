#include <string>
#include <print>

// Message types
struct Title {
    std::string title;
    Title(const std::string& t) : title(t) {}
};

struct Quote {
    size_t quote;
    Quote(size_t q) : quote(q) {}
};

struct Address {
    std::string address;
    Address(const std::string& a) : address(a) {}
};

struct Date {
    std::string date;
    Date(const std::string& d) : date(d) {}
};

// Forward declarations
class Buyer1;
class Buyer2;

class Seller {
public:
    Seller(): title(nullptr), address(nullptr), done(false) {}
    void addTitle(Title& t);
    void addAddress(Address& a);
    void InitiateSale(Buyer1* b1, Buyer2* b2);
    bool isdone() {
        return done;
    }
private:
    Title* title;
    Address* address;
    bool done;
};

class Buyer1 {
public:
    Buyer1(): b2(nullptr), s(nullptr), q(nullptr), done(false) {}
    void addB2(Buyer2& b2);
    void addSeller(Seller& s);
    void addQ(Quote& q);
    void sale1();
    bool isdone() {
        return done;
    }

private:
    Buyer2* b2;
    Seller* s;
    Quote* q;
    bool done;
};

class Buyer2 {
public:
    Buyer2(): b1(nullptr), s(nullptr), q1(nullptr), q2(nullptr), d(nullptr), done(false) {}
    void addB1(Buyer1& b1);
    void addSeller(Seller& s);
    void addQ1(Quote& q);
    void addQ2(Quote& q);
    void addDate(Date& d);
    void sale2();
    bool isdone() {
        return done;
    }

private:
    Buyer1* b1;
    Seller* s;
    Quote* q1;
    Quote* q2;
    Date* d;
    bool done;
};
