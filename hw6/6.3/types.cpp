#include <iostream>
#include <string.h>

// Forward declarations
double floatHelper(float& f);
void simpleFunc();

class A {
public:
    A() : data(42) {}

    int processPointer(int* ptr) {
        return *ptr + data;
    }

    int processFunction(double (*fn)(float&)) {
        float f = 3.14f;
        return static_cast<int>(fn(f));
    }

    int data;
};

// Helper functions
double floatHelper(float& f) {
    return f * 2.0;
}

void simpleFunc() {
    std::cout << "Simple function called" << std::endl;
}

int getValue() {
    return 42;
}

char* concatenate(char* dest, char* src) {
    return strcat(dest, src);
}

int main() {
    // 1. int *
    std::cout << "1. int * :\n";
    int value1 = 42;
    int* ptr1 = &value1;
    std::cout << "   Value: " << *ptr1 << "\n\n";

    // 2. int &
    std::cout << "2. int & :\n";
    int value2 = 100;
    int& ref2 = value2;
    std::cout << "   Value: " << ref2 << "\n\n";

    // 3. double
    std::cout << "3. double :\n";
    double value3 = 3.14159;
    std::cout << "   Value: " << value3 << "\n\n";

    // 4. A *
    std::cout << "4. A * :\n";
    A* ptr4 = new A();
    std::cout << "   Value: " << ptr4->data << "\n";
    delete ptr4;
    std::cout << "\n";

    // 5. char const *
    std::cout << "5. char const * :\n";
    const char* ptr5 = "Hello";
    std::cout << "   Value: " << ptr5 << "\n\n";

    // 6. char const &
    std::cout << "6. char const & :\n";
    const char c6 = 'A';
    const char& ref6 = c6;
    std::cout << "   Value: " << ref6 << "\n\n";

    // 7. long[7]
    std::cout << "7. long[7] :\n";
    long arr7[7] = {1L, 2L, 3L, 4L, 5L, 6L, 7L};
    std::cout << "   Value[3]: " << arr7[3] << "\n\n";

    // 8. int **
    std::cout << "8. int ** :\n";
    int value8 = 200;
    int* ptr8a = &value8;
    int** ptr8b = &ptr8a;
    std::cout << "   Value: " << **ptr8b << "\n\n";

    // 9. int *&
    std::cout << "9. int *& :\n";
    int value9 = 300;
    int* ptr9 = &value9;
    int*& ref9 = ptr9;
    std::cout << "   Value: " << *ref9 << "\n\n";

    // 10. float &
    std::cout << "10. float & :\n";
    float value10 = 3.14f;
    float& ref10 = value10;
    std::cout << "    Value: " << ref10 << "\n\n";

    // 11. int (*)()
    std::cout << "11. int (*)() :\n";
    int (*ptr11)() = getValue;
    std::cout << "    Value: " << ptr11() << "\n\n";

    // 12. int (*&)()
    std::cout << "12. int (*&)() :\n";
    int (*&ref12)() = ptr11;
    std::cout << "    Value: " << ref12() << "\n\n";

    // 13. char *(*)(char *, char *)
    std::cout << "13. char *(*)(char *, char *) :\n";
    char *(*ptr13)(char*, char*) = concatenate;
    char str13a[20] = "Hello, ";
    char str13b[] = "World!";
    std::cout << "    Value: " << ptr13(str13a, str13b) << "\n\n";

    // 14. int A::*
    std::cout << "14. int A::* :\n";
    int A::* ptr14 = &A::data;
    A obj14;
    std::cout << "    Value: " << obj14.*ptr14 << "\n\n";

    // 15. int (A::*)(int *)
    std::cout << "15. int (A::*)(int *) :\n";
    int (A::*ptr15)(int*) = &A::processPointer;
    A obj15;
    int testVal15 = 10;
    std::cout << "    Value: " << (obj15.*ptr15)(&testVal15) << "\n\n";

    // 16. int (A::**)(int *)
    std::cout << "16. int (A::**)(int *) :\n";
    int (A::**ptr16)(int*) = new (int (A::*)(int*));
    *ptr16 = &A::processPointer;
    A obj16;
    int testVal16 = 10;
    std::cout << "    Value: " << (obj16.**ptr16)(&testVal16) << "\n\n";
    delete ptr16;

    // 17. int (A::*&)(int *)
    std::cout << "17. int (A::*&)(int *) :\n";
    int (A::*ptr17)(int*) = &A::processPointer;
    int (A::*&ref17)(int*) = ptr17;
    A obj17;
    int testVal17 = 10;
    std::cout << "    Value: " << (obj17.*ref17)(&testVal17) << "\n\n";

    // 18. int (A::*)(double (*)(float &))
    std::cout << "18. int (A::*)(double (*)(float &)) :\n";
    int (A::*ptr18)(double (*)(float&)) = &A::processFunction;
    A obj18;
    std::cout << "    Value: " << (obj18.*ptr18)(floatHelper) << "\n\n";

    // 19. void (*p[10])(void (*)())
    std::cout << "19. void (*p[10])(void (*)()) :\n";
    void (*arr19[10])(void (*)());
    arr19[0] = [](void (*fn)()) { 
        std::cout << "    ";
        fn(); 
    };
    arr19[0](simpleFunc);

    return 0;
}