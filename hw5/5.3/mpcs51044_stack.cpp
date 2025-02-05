#include <iostream>
#include <thread>
#include "mpcs51044_stack.h"

int main() {
    mpcs51044::stack<int> s1 = {1, 2, 3};
    std::cout << s1.pop() << std::endl;
    
    mpcs51044::stack<int> s2 = s1;
    std::cout << s2.pop() << std::endl;
    
    mpcs51044::stack<int> s3;
    s3 = s1;
    std::cout << s3.pop() << std::endl;
    
    return 0;
}