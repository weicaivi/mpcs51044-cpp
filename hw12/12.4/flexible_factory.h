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

// Creator for flexible factory
template<typename Signature>
struct flexible_abstract_creator {
    using T = typename signature_trait<Signature>::return_type;
    using ArgsT = typename signature_trait<Signature>::args_tuple;
    
    template<typename... Args>
    virtual std::unique_ptr<T> doCreate(TTS<T, Args...>&&, Args&&... args) = 0;
};

// Default case - no args
template<typename T>
struct flexible_abstract_creator<T> {
    virtual std::unique_ptr<T> doCreate(TTS<T>&&) = 0;
};

// flexible abstract factory
template<typename... Signatures>
struct flexible_abstract_factory : public flexible_abstract_creator<Signatures>... {
    // Helper to handle no parameter case
    template<typename U>
    std::unique_ptr<U> create() {
        flexible_abstract_creator<U>& creator = *this;
        return creator.doCreate(TTS<U>());
    }
    
    // Helper to handle parameterized constructors
    template<typename U, typename... Args>
    std::unique_ptr<U> create(Args&&... args) {
        flexible_abstract_creator<U(Args...)>& creator = *this;
        return creator.doCreate(TTS<U, Args...>(), std::forward<Args>(args)...);
    }
    
    virtual ~flexible_abstract_factory() = default;
};

// Concrete creator for flexible factory - default constructor case
template<typename AbstractFactory, typename Abstract, typename Concrete>
struct flexible_concrete_creator : virtual public AbstractFactory {
    std::unique_ptr<Abstract> doCreate(TTS<Abstract>&&) override {
        return std::make_unique<Concrete>();
    }
};

// Concrete creator for flexible factory - parameterized constructor case
template<typename AbstractFactory, typename Abstract, typename... Args, typename Concrete>
struct flexible_concrete_creator<AbstractFactory, Abstract(Args...), Concrete> : virtual public AbstractFactory {
    std::unique_ptr<Abstract> doCreate(TTS<Abstract, Args...>&&, Args&&... args) override {
        return std::make_unique<Concrete>(std::forward<Args>(args)...);
    }
};

// Concrete factory helper
template<typename AbstractFactory, typename... ConcreteTypes>
struct flexible_concrete_factory;

// Specialization for matching abstract and concrete types
template<typename... AbstractTypes, typename... ConcreteTypes>
struct flexible_concrete_factory<flexible_abstract_factory<AbstractTypes...>, ConcreteTypes...> 
    : public flexible_concrete_creator<flexible_abstract_factory<AbstractTypes...>, 
                                      AbstractTypes, ConcreteTypes>... {
};

} 
#endif