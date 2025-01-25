#include <iostream>

struct PrintOnDestruct {
    PrintOnDestruct(const char* msg) : message(msg) {}
    ~PrintOnDestruct() { std::cout << message; }
    const char* message;
};

struct PrintOnConstruct {
    PrintOnConstruct(const char* msg) : message(msg) { std::cout << message; }
    const char* message;
};

static PrintOnDestruct world{", world!\n"};
static PrintOnConstruct hello{"Hello"};

struct EnsureOrder {
    EnsureOrder() { 
        (void)&hello; 
        (void)&world;
    }
};

static EnsureOrder order;

int main() {
    return 0;
}