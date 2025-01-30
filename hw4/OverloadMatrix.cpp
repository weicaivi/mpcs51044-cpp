#include "OverloadMatrix.h"
#include <iostream>
#include <chrono>
using namespace mpcs51044;
using namespace std;

int main()
{
	auto start = chrono::system_clock::now();
	Matrix<double, 3, 3> m = {
			{ 1, 2, 3, },
			{ 4, 5, 6, },
			{ 7, 8, 9, }
	};
	static double total;
	for (int i = 0; i < 100'000'000; i++) {
		m(1, 1) = i;
		total += m.determinant();
	}
	cout << m;
	cout << chrono::duration<double>(chrono::system_clock::now() - start).count() << " seconds\n";
}