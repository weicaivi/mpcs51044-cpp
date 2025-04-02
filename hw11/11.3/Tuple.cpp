#include<iostream>
#include"Tuple.h"
using namespace std;
using namespace mpcs;

Tuple1<int, double> t1id(1, 2.3);
Tuple2<int, double> t2id(4, 5.6);
int main()
{
	cout << "Tuple1<int> is " << sizeof(Tuple1<int>) << " bytes"
		 << " and sizeof(int) is " << sizeof(int) << " bytes" << endl;
	cout << get<1>(t1id) << endl;

	cout << "Tuple2<int> is " << sizeof(Tuple2<int>) << " bytes"
		<< " and sizeof(int) is " << sizeof(int) << " bytes" << endl;
	cout << get<1>(t2id) << endl;
    
	cout << "get<1>(t2id) =  " << get<1>(t2id) << endl;
    cout << "get<int>(t2id) = " << get<int>(t2id) << endl;

 	return 0;
}