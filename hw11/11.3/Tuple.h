#ifndef TUPLE_H
#  define TUPLE_H
#include<utility>
#include<type_traits>
#include"variadic_examples.h"
#include<tuple>
namespace mpcs {
// First try at tuple
template<typename ...Ts> struct Tuple1;

template<> struct Tuple1<> {};

template<typename T, typename ...Ts>
struct Tuple1<T, Ts...>
{
	Tuple1(T const &t, Ts const &... ts) : val(t), restOfVals(ts...) {}
	T val;
	Tuple1<Ts...> restOfVals;
};

// Indirect through a class because can't partially specialize template function

template<int i, typename ...Ts>
struct Getter1;

template<typename T,  typename ...Ts>
struct Getter1<0, T, Ts...> {
	static auto &get(Tuple1<T, Ts...> &tup) {
		return tup.val;
	}
};

template<int i, typename T, typename ...Ts>
struct Getter1<i, T, Ts...> {
	static auto &get(Tuple1<T, Ts...> &tup) {
		return Getter1<i-1, Ts...>::get(tup.restOfVals);
	}
};

template<int i, typename ...Ts>
auto &
get(Tuple1<Ts...> &tup) {
	return Getter1<i, Ts...>::get(tup);
}

/*
// Here is an example of what the template instantiations generate
struct Tuple1<int, float> {
	int val;
	Tuple1<float> restofVals;
};

struct Tuple1<float> {
	float val;
	Tuple1<> restOfVals;
};

*/
// Tuple2 is almost identical to tuple1 except it uses the empty base optimization
// for better space utilization as demonstrated in Tuple.cpp
template<typename ...Ts> struct Tuple2;

template<> struct Tuple2<> {};

template<typename T, typename ...Ts>
struct Tuple2<T, Ts...> : public Tuple2<Ts...>
{
	Tuple2(T const &t, Ts const &... ts) : Tuple2<Ts...>(ts...), val(t) {}
	T val;
};

// Indirect through a class because can't partially specialize template arguments
// 
// auto Getter2<0, int, float>::get(Tuple2<int, float> &t2) { return t2.val }
// auto Getter2<1, int, float>::get(Tuple2<int, float> &t2) { 
//   Tuple2<float> &restOfVals = t2;
//   return Getter2<0>(restOfVals);
// }
template<int i, typename ...Ts>
struct Getter2;

template<typename T, typename ...Ts>
struct Getter2<0, T, Ts...> {
	static auto &get(Tuple2<T, Ts...> &tup) {
		return tup.val;
	}
};

template<int i, typename T, typename ...Ts>
struct Getter2<i, T, Ts...> {
	static auto &get(Tuple2<T, Ts...> &tup) {
		Tuple2<Ts...> &restOfVals = tup;
		return Getter2<i - 1, Ts...>::get(restOfVals);
	}
	
	static auto &get(Tuple2<T, Ts...> const &tup) {
		Tuple2<Ts...> const &restOfVals = tup;
		return Getter2<i - 1, Ts...>::get(restOfVals);
	}
};

template<int i, typename ...Ts>
auto &
get(Tuple2<Ts...> &tup) {
	return Getter2<i, Ts...>::get(tup);
}

// HW implementation: Get elements by type
// Helper to find the index of a type in a parameter pack
template<typename T, typename... Ts>
struct TypeIndex;

// Base case: type found at index 0
template<typename T, typename... Ts>
struct TypeIndex<T, T, Ts...> {
    static constexpr int value = 0;
};

// Recursive case: keep searching
template<typename T, typename U, typename... Ts>
struct TypeIndex<T, U, Ts...> {
    static constexpr int value = 1 + TypeIndex<T, Ts...>::value;
};

// Handle error case (type not found) by static_assert
template<typename T>
struct TypeIndex<T> {
    static_assert(sizeof(T) == 0, "The requested type is not in the tuple");
};

// Type-based getter implementation
template<typename T, typename... Ts>
auto& get(Tuple2<Ts...>& tup) {
    // Check if type exists exactly once
    constexpr int count = (std::is_same<T, Ts>::value + ...);
    static_assert(count > 0, "The requested type is not in the tuple");
    static_assert(count == 1, "The requested type appears multiple times in the tuple");
    
    // Use the index-based getter with the calculated index
    constexpr int index = TypeIndex<T, Ts...>::value;
    return get<index>(tup);
}

}

#endif