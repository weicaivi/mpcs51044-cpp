#include <iostream>
#include <thread>
#include "mpcs51044_stack.h"

int main() {
    // Create a stack with initializer list
    mpcs51044::ThreadSafeStack<int> stack = {1, 2, 3, 4, 5};
    std::cout << "Initial stack size: " << stack.size() << "\n";

    stack.push(6);
    stack.push(7);
    
    int value;
    while (stack.pop(value)) {
        std::cout << "Popped: " << value << "\n";
    }

    std::cout << "Stack is empty: " << (stack.empty() ? "yes" : "no") << "\n";
    return 0;
}