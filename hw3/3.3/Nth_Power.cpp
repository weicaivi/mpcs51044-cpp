#include <iostream>
#include <vector>
#include <algorithm>
#include <iterator>
using namespace std;

class Nth_Power {
    int n;
public:
    Nth_Power(int power) : n(power) {}
    
    int operator()(int x) const {
        int result = 1;
        for(int i = 0; i < n; i++) {
            result *= x;
        }
        return result;
    }
};

int main() {
    Nth_Power cube(3);
    cout << cube(4) << endl;  // prints 64

    vector<int> v = {1, 2, 3, 4, 5};
    transform(v.begin(), v.end(), 
             ostream_iterator<int>(cout, " "), 
             cube);
    return 0;
}