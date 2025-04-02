#include <tuple>
#include <iostream>
#include <chrono>
#include <type_traits>

using std::tuple;
using std::cout;
using std::endl;

// Original class template approach for Length
template<class TList> struct Length;

template<>
struct Length<tuple<>>
{
    static constexpr size_t value = 0;
};

template<class T, typename... Us>
struct Length<tuple<T, Us...>>
{
    static constexpr size_t value = 1 + Length<tuple<Us...>>::value;
};

// Function template approach for Length
template<typename... Ts>
constexpr size_t length(tuple<Ts...> const&)
{
    return sizeof...(Ts);
}

// Original class template approach for TypeAt
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

// Helper alias
template<class List, int i>
using TypeAt_t = typename TypeAt<List, i>::type;

// Function template approach for TypeAt using a simpler technique
namespace type_utils {
    template<size_t I, typename... Ts>
    struct type_at_func;
    
    // Specialization for index 0
    template<typename T, typename... Ts>
    struct type_at_func<0, T, Ts...> {
        using type = T;
        
        // This function is never called, just used for decltype
        static constexpr T get(tuple<T, Ts...> const&) {
            return T{};
        }
    };
    
    // Specialization for index > 0
    template<size_t I, typename T, typename... Ts>
    struct type_at_func<I, T, Ts...> {
        using type = typename type_at_func<I-1, Ts...>::type;
        
        // This function is never called, just used for decltype
        static constexpr type get(tuple<T, Ts...> const&) {
            return type{};
        }
    };
}

// Function-like interface using decltype
template<size_t I, typename... Ts>
constexpr auto type_at_fn(tuple<Ts...> const&) 
    -> typename type_utils::type_at_func<I, Ts...>::type;

// Type alias for easier use
template<size_t I, typename... Ts>
using type_at = decltype(type_at_fn<I>(std::declval<tuple<Ts...>>()));

// test tuples
using SmallTuple = tuple<int, double, char>;
using MediumTuple = tuple<int, double, char, float, bool, short, long, unsigned, void*, int, double>;
using LargeTuple = tuple<
    int, double, char, float, bool, short, long, unsigned, void*, int, double,
    int, double, char, float, bool, short, long, unsigned, void*, int, double,
    int, double, char, float, bool, short, long, unsigned, void*, int, double,
    int, double, char, float, bool, short, long, unsigned, void*, int, double,
    int, double, char, float, bool, short, long, unsigned, void*, int, double
>;

int main()
{
    // Runtime measurements
    cout << "=== Runtime Measurements (informational only) ===" << endl;
    
    // Benchmark class template approach
    {
        auto start = std::chrono::high_resolution_clock::now();
        
        volatile size_t result = 0;
        for (int i = 0; i < 1000000; i++) {
            result += Length<SmallTuple>::value;
            result += Length<MediumTuple>::value;
            result += Length<LargeTuple>::value;
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        
        cout << "Class template approach runtime: " << duration << " µs" << endl;
        cout << "Result: " << result << endl;
    }
    
    // Benchmark function template approach
    {
        auto start = std::chrono::high_resolution_clock::now();
        
        volatile size_t result = 0;
        for (int i = 0; i < 1000000; i++) {
            SmallTuple s;
            MediumTuple m;
            LargeTuple l;
            result += length(s);
            result += length(m);
            result += length(l);
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        
        cout << "Function template approach runtime: " << duration << " µs" << endl;
        cout << "Result: " << result << endl;
    }
    
    cout << "\n=== Compile-Time Information ===" << endl;
    
    cout << "Small tuple size using class template: " << Length<SmallTuple>::value << endl;
    cout << "Small tuple size using function template: " << length(SmallTuple{}) << endl;
    
    cout << "Large tuple size using class template: " << Length<LargeTuple>::value << endl;
    cout << "Large tuple size using function template: " << length(LargeTuple{}) << endl;
    
    cout << "\nTypeAt for SmallTuple index 1 gives type: " 
         << typeid(TypeAt_t<SmallTuple, 1>).name() << endl;
    
    cout << "type_at for SmallTuple index 1 gives type: " 
         << typeid(type_at<1, int, double, char>).name() << endl;
    
    return 0;
}