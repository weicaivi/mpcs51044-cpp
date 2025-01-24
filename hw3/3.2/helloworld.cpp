#include <iostream>

struct PrintOnDestruct {
    ~PrintOnDestruct() { std::cout << message; }
    const char* message;
};

struct PrintOnConstruct {
    PrintOnConstruct() { std::cout << message; }
    const char* message;
};

static PrintOnDestruct world{", world!\n"};
static PrintOnConstruct hello{"Hello"};

struct EnsureOrder {
    EnsureOrder() { 
        // Force hello to be constructed before world
        (void)&hello; 
        // Prevent optimization of world
        (void)&world;
    }
};

static EnsureOrder order;

int main() {
    return 0;
}