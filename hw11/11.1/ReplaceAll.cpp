#include <tuple>
#include <type_traits>
#include <iostream>

template <typename TL, typename T, typename U>
struct ReplaceAll;

template <typename... Ts, typename T, typename U>
struct ReplaceAll<std::tuple<Ts...>, T, U> {
    using type = std::tuple<typename std::conditional<std::is_same<Ts, T>::value, U, Ts>::type...>;
};

// Helper alias template for cleaner syntax
template <typename TL, typename T, typename U>
using ReplaceAll_t = typename ReplaceAll<TL, T, U>::type;

int main() {
    using Original = std::tuple<char, int, char>;
    using Expected = std::tuple<double, int, double>;
    using Result = ReplaceAll_t<Original, char, double>;
    
    static_assert(std::is_same<Result, Expected>::value, 
                  "ReplaceAll failed to correctly replace types");
    
    std::cout << "ReplaceAll successfully replaced all occurrences of the specified type." << std::endl;
    
    return 0;
}