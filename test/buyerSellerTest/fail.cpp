#include "test.hpp"
#include <thread>
#include <chrono>


// Seller methods
void Seller::addTitle(Title& t) {
    title = &t;
}

void Seller::addAddress(Address& a) {
    address = &a;
}

void Seller::InitiateSale(Buyer1* b1, Buyer2* b2) {
    std::println("Seller: Thread Started");
    b1->addSeller(*this);
    b2->addSeller(*this);
    b1->addB2(*b2);
    b2->addB1(*b1);
    std::println("Seller: Buyer 1 and 2 informed");

    while (!title) {
        // Spin-wait
    }
    std::println("Seller: Received title -> {}", title->title);

    Quote q{100};
    std::println("Seller: Sent quote -> {}", q.quote);
    b1->addQ(q);
    b2->addQ(q);

    while (!address) {
        // Spin-wait
    }
    std::println("Seller: Received address -> {}", address->address);

    Date d{"2025-05-07"};
    std::println("Seller: Sent delivery date -> {}", d.date);
    b2->addDate(d);
    done = true;
}

// Buyer1 methods
void Buyer1::addB2(Buyer2& b2) {
    this->b2 = &b2;
}

void Buyer1::addSeller(Seller& s) {
    this->s = &s;
}

void Buyer1::addQ(Quote& q) {
    this->q = &q;
}

void Buyer1::sale1() {
    std::println("Buyer1: thread started");
    while (!s) {
        // Spin-wait
    }
    while(!b2) {
        //spinwait
    }
    std::println("Buyer1: recieved reference to Buyer2 and Seller");

    Title t{"The Vampire Diaries"};
    std::println("Buyer1: Sent title -> {}", t.title);
    s->addTitle(t);

    while (!q) {
        // Spin-wait
    }
    std::println("Buyer1: Received quote -> {}", q->quote);

    PartialQuote q2(q, q->quote/2);
    std::println("Buyer1: Forwarded quote -> {}", q2.partialQuote);
    b2->addPQ(q2);
    done = true;
}

// Buyer2 methods
void Buyer2::addB1(Buyer1& b1) {
    this->b1 = &b1;
}

void Buyer2::addSeller(Seller& s) {
    this->s = &s;
}

void Buyer2::addQ(Quote& q) {
    b1->addQ(q);
}

void Buyer2::addPQ(PartialQuote& q) {
    //q2 = &q;
    q1 = q.quote;
    
}

void Buyer2::addDate(Date& d) {
    this->d = &d;
}

void Buyer2::sale2() {

    std::println("Buyer2: thread started");
    while (!s) {
        //spin-wait
    }

    while(!b1) {
        //spinwait
    }
    std::println("Buyer2: recieved reference to Buyer1 and Seller");
    while (!q1) {
        // Spin-wait
    }
    std::println("Buyer2: Received quote 1-> {}", q1->quote);
    while(!q2){
        //wait for second quote
    }
    std::println("Buyer2: recieved forwarded quote2 -> {}", q2->partialQuote);

    Address addr{"Rue Langgads Vej 2..."};
    std::println("Buyer2: Sent address -> {}", addr.address);
    s->addAddress(addr);

    while (!d) {
        // Spin-wait
    }
    std::println("Buyer2: Received delivery date -> {}", d->date);
    done = true;
}

int main() {
    // Participants
    Seller seller{};
    Buyer1 buyer1{};
    Buyer2 buyer2{};

    // Threads for communication
    std::thread sellerThread([&]() { seller.InitiateSale(&buyer1, &buyer2); });
    std::thread buyer1Thread([&]() { buyer1.sale1(); });
    std::thread buyer2Thread([&]() { buyer2.sale2(); });

    while(!seller.isdone() || !buyer1.isdone() || !buyer2.isdone()) {
        //wait
    }
    // Join threads
    sellerThread.join();
    buyer1Thread.join();
    buyer2Thread.join();

    return 0;
}