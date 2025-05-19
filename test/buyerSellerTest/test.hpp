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
struct PartialQuote {
    Quote* quote;
    size_t partialQuote;
    PartialQuote(Quote* quote, size_t partialQuote): quote(quote), partialQuote(partialQuote) {}
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
    bool isdone() {
        return done;
    }
    void InitiateSale(Buyer1* b1, Buyer2* b2);
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

    bool isdone() {
        return done;
    }
    void sale1();

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
    void addQ(Quote& q);
    void addPQ(PartialQuote& q);
    void addDate(Date& d);

    bool isdone() {
        return done;
    }

    void sale2();

private:
    Buyer1* b1;
    Seller* s;
    Quote* q1;
    PartialQuote* q2;
    Date* d;
    bool done;
};
