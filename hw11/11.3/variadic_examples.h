#ifndef VARIADIC_EXAMPLES_H
#  define VARIADIC_EXAMPLES_H
#include <tuple>
using std::tuple;

namespace mpcs {

// Declaration of Length template that calculates the number of parameters in a tuple
template<typename T> struct Length;

// Define specialized implementation for zero length tuples
template<>
struct Length<tuple<>>
{
	static int constexpr value = 0;
};

// Define specialization for tuples of any length
// This will only be matched for tuples with at least one parameter
// because the specialization above is a better match for tuples with no parameters
template<class T, typename... Us>
struct Length<tuple<T, Us...> > 
{
	// We could just use sizeof...(Us),
	// but let’s see how to do ourselves
	static int constexpr value = 1 + Length<tuple<Us...>>::value;
};

template<class List, int i> struct TypeAt;
template<class Head, typename... Tail>
struct TypeAt<tuple<Head, Tail...>, 0>
{
  using type = Head;
};
template<class Head, typename... Tail, int i>
struct TypeAt<tuple<Head, Tail...>, i>
	: public TypeAt<tuple<Tail...>, i - 1>
{
};

// Find the index corresponding to a given type
template<class List, class Target> struct IndexOf;
template<class Target>
struct IndexOf<tuple<>, Target>
{
	static int constexpr value = -1; // Return -1 if not found
};
template<typename Target, typename... Tail>
struct IndexOf<tuple<Target, Tail...>, Target>
{
  static int constexpr value{0};
};
template<class Head, typename... Tail, class Target>
struct IndexOf<tuple<Head, Tail...>, Target>
{
private: // Store the index of the type in the tail
	static int constexpr temp = IndexOf<tuple<Tail...>, Target>::value;
public:
	// The index in the original tuple will be one higher than
	// the index in the tail, but we need to be careful and not
	// increment when the type was not found in the tail (indicated
	// by an index of -1)
	static int constexpr value = temp == -1 ? -1 : 1 + temp;
};

// Count of type in a tuple. Adding for Tuple3 problems. Use TplType to handle both std::tuple and Tuple3
template<typename X, typename Tpl> struct Count;

template<typename X, template<typename...> class TplType>
struct Count<X, TplType<>> {
	static int constexpr value = 0; // No occurrences
};

template<typename X, template<typename...> class TplType, typename ...Ts>
struct Count<X, TplType<X, Ts...>> {
	static int constexpr value = Count<X, TplType<Ts...>>::value + 1;
};

template<typename X, typename T, template<typename...> class TplType, typename ...Ts>
struct Count<X, TplType<T, Ts...>> : public Count<X, TplType<Ts...>> { // Inherit value
};

// Append the types in two tuples
template<class First, class Second> struct Append;

template<typename... Ts, typename... Us>
struct Append<tuple<Ts...>, tuple<Us...> >
{
	typedef tuple<Ts..., Us...> type;
};

template<typename T, typename A, typename B> struct Replace;

template<typename A, typename B>
struct Replace<tuple<>, A, B> {
	using type = tuple<>;
};

template<typename H, typename... Ts, typename A, typename B>
struct Replace<tuple<H, Ts...>, A, B>
 : public Append<tuple<H>, typename Replace<tuple<Ts...>, A, B>::type> {
};

template<typename... Ts, typename A, typename B>
struct Replace<tuple<A, Ts...>, A, B> {
	using type = tuple<B, Ts...>;
};


// Use the following to get rid of all of the typename!
template<typename T, typename A, typename B>
using Replace_t = typename Replace<T, A, B>::type;
}
#endif