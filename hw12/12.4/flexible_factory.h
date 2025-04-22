#ifndef FLEXIBLE_FACTORY_H
#define FLEXIBLE_FACTORY_H

#include <memory>
#include <tuple>
#include <type_traits>
#include <utility>

namespace cspp51045 {

// Helper to extract return type from a signature like T(Args...)
template<typename Signature>
struct signature_traits;

template<typename R, typename... Args>
struct signature_traits<R(Args...)> {
    using return_type = R;
    using args_tuple = std::tuple<Args...>;
    static constexpr size_t args_count = sizeof...(Args);
};

// TT type for tag dispatching
template<typename T>
struct TT {};

// Abstract creator with arguments
template<typename Signature>
struct flexible_abstract_creator {
    using return_type = typename signature_traits<Signature>::return_type;
    using args_tuple = typename signature_traits<Signature>::args_tuple;
    
    template<typename... Args>
    std::unique_ptr<return_type> create(Args&&... args) {
        return doCreate(TT<return_type>{}, std::forward<Args>(args)...);
    }
    
    virtual ~flexible_abstract_creator() = default;
    
protected:
    template<typename... Args>
    virtual std::unique_ptr<return_type> doCreate(TT<return_type>&&, Args&&... args) = 0;
};

// Flexible abstract factory that handles multiple signatures
template<typename... Signatures>
struct flexible_abstract_factory : public flexible_abstract_creator<Signatures>... {
    template<typename U, typename... Args>
    std::unique_ptr<U> create(Args&&... args) {
        flexible_abstract_creator<U(Args...)>& creator = *this;
        return creator.create(std::forward<Args>(args)...);
    }
    
    virtual ~flexible_abstract_factory() = default;
};

// Concrete creator for flexible factory
template<typename AbstractFactory, typename Signature, typename Concrete>
struct flexible_concrete_creator : virtual public AbstractFactory {
    using return_type = typename signature_traits<Signature>::return_type;
    
    template<typename... Args>
    std::unique_ptr<return_type> doCreate(TT<return_type>&&, Args&&... args) override {
        return std::make_unique<Concrete>(std::forward<Args>(args)...);
    }
};

// Helper to make pairs of abstract and concrete types
template<typename AbstractFactory, typename Signature, typename Concrete>
using concrete_pair = flexible_concrete_creator<AbstractFactory, Signature, Concrete>;

// Flexible concrete factory
template<typename AbstractFactory, typename... ConcretePairs>
struct flexible_concrete_factory : public ConcretePairs... {
};

}

#endif