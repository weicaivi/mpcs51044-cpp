#ifndef FLEXIBLE_FACTORY_H
#define FLEXIBLE_FACTORY_H
#include <tuple>
#include <memory>
#include <type_traits>
#include <functional>

namespace cspp51045 {

// Type tag for type identification
template<typename T, typename... Args>
struct TTS {
    using type = T;
    using args = std::tuple<Args...>;
};

// Helper to extract type from a function signature
template<typename T>
struct signature_trait;

// Specialization for function types
template<typename R, typename... Args>
struct signature_trait<R(Args...)> {
    using return_type = R;
    using args_tuple = std::tuple<Args...>;
    static constexpr size_t arg_count = sizeof...(Args);
};

// Add this helper to determine if a type is a function signature
template<typename T>
struct is_signature : std::false_type {};

template<typename R, typename... Args>
struct is_signature<R(Args...)> : std::true_type {};

template<typename T>
inline constexpr bool is_signature_v = is_signature<T>::value;

// Modified flexible abstract creator
template<typename T, typename Enable = void>
struct flexible_abstract_creator;

// Specialization for function signatures
template<typename Signature>
struct flexible_abstract_creator<Signature, 
    std::enable_if_t<is_signature_v<Signature>>> {
    using T = typename signature_trait<Signature>::return_type;
    using ArgsT = typename signature_trait<Signature>::args_tuple;
    
    template<typename... Args>
    virtual std::unique_ptr<T> doCreate(TTS<T, Args...>&&, Args&&... args) = 0;
};

// Specialization for non-function types (default constructor case)
template<typename T>
struct flexible_abstract_creator<T, 
    std::enable_if_t<!is_signature_v<T>>> {
    virtual std::unique_ptr<T> doCreate(TTS<T>&&) = 0;
};

// The flexible abstract factory
template<typename... Types>
struct flexible_abstract_factory : public flexible_abstract_creator<Types>... {
    // Helper for parameterized constructor case
    template<typename U, typename... Args, 
             std::enable_if_t<is_signature_v<U(Args...)>, int> = 0>
    std::unique_ptr<typename signature_trait<U(Args...)>::return_type> 
    create(Args&&... args) {
        using ReturnType = typename signature_trait<U(Args...)>::return_type;
        flexible_abstract_creator<U(Args...)>& creator = *this;
        return creator.doCreate(TTS<ReturnType, Args...>(), std::forward<Args>(args)...);
    }
    
    // Helper for default constructor case
    template<typename U, 
             std::enable_if_t<!is_signature_v<U>, int> = 0>
    std::unique_ptr<U> create() {
        flexible_abstract_creator<U>& creator = *this;
        return creator.doCreate(TTS<U>());
    }
    
    virtual ~flexible_abstract_factory() = default;
};

// Concrete creator for default constructor case
template<typename AbstractFactory, typename Abstract, typename Concrete>
struct flexible_concrete_creator_default : virtual public AbstractFactory {
    std::unique_ptr<Abstract> doCreate(TTS<Abstract>&&) override {
        return std::make_unique<Concrete>();
    }
};

// Concrete creator for parameterized constructor case
template<typename AbstractFactory, typename Signature, typename Concrete>
struct flexible_concrete_creator_param;

template<typename AbstractFactory, typename Abstract, typename... Args, typename Concrete>
struct flexible_concrete_creator_param<AbstractFactory, Abstract(Args...), Concrete> 
    : virtual public AbstractFactory {
    std::unique_ptr<Abstract> doCreate(TTS<Abstract, Args...>&&, Args&&... args) override {
        return std::make_unique<Concrete>(std::forward<Args>(args)...);
    }
};

// Helper type trait to determine which concrete creator to use
template<typename T>
struct factory_trait {
    using type = T;
    static constexpr bool has_params = false;
};

template<typename R, typename... Args>
struct factory_trait<R(Args...)> {
    using type = R;
    static constexpr bool has_params = true;
    using signature = R(Args...);
};

// Concrete factory helper
template<typename AbstractFactory, typename... ConcreteTypes>
struct flexible_concrete_factory;

// Implementation of concrete factory
template<typename... AbstractTypes, typename... ConcreteTypes>
struct flexible_concrete_factory<flexible_abstract_factory<AbstractTypes...>, ConcreteTypes...> 
    : public std::conditional_t
        factory_trait<AbstractTypes>::has_params,
        flexible_concrete_creator_param<flexible_abstract_factory<AbstractTypes...>, 
                                       AbstractTypes, 
                                       ConcreteTypes>,
        flexible_concrete_creator_default<flexible_abstract_factory<AbstractTypes...>, 
                                         typename factory_trait<AbstractTypes>::type, 
                                         ConcreteTypes>
    >... 
{};

} // namespace cspp51045
#endif