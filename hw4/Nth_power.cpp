#include <algorithm>
#include <vector>
#include <iostream>
using namespace std;

struct Nth_Power {
	unsigned operator()(int x) {
		int result = 1;
		for (unsigned i = 1; i <= n; i++)
			result *= x;
		return result;
	}
	unsigned n;
};

int main() {
	vector v = { 1, 2, 3, 4, 5 };
	Nth_Power cube{ 3 };
	cout << cube(7) << endl; // prints 343
	// print first five cubes
	transform(v.begin(), v.end(), 
		      ostream_iterator<int>(cout, ", "), cube);
}