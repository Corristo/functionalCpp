#ifndef MONAD_H
#define MONAD_H
#include "curry.h"
#include <type_traits>
#include "cpp17.h"
#include "type_traits.h"

namespace monad {
    // returns the return type of operator>>=(const Monad<T>&, funcType) if such
    // an operator is defined and nothing otherwise. Here Monad is a fixed
    // type constructor and T and funcType are arbitrary types.
    template <template <typename, typename...> class Monad, typename T, typename funcType>
    using monadic_bind_t = decltype(std::declval<const Monad<T>&>() >>= std::declval<funcType>());

    // checks whether a given type constructor Monad actually has a monadic
    // bind operator, i.e. whether operator>>=(const Monad<T1>& >>= (T ->
    // Monad<T2>)) is defined and correctly returns Monad<T2>
    template <template <typename, typename...> class, typename = void>
    struct is_monad : std::false_type {};

    template <template <typename, typename...> class Monad>
    struct is_monad<Monad, void_t<monadic_bind_t<Monad, int, Monad<double>(*)(int)>>>
        : std::is_same<monadic_bind_t<Monad, int, Monad<double>(*)(int)>, Monad<double>> {};

    template <template <typename, typename...> class Monad>
    constexpr bool is_monad_v = is_monad<Monad>::value;

    // checks whether a given type T is of the form T = Monad<U> where U is
    // an arbitrary type and Monad is some type constructor that is a monad in
    // the sense of is_monad.
    template <typename>
    struct is_monadic_type : std::false_type {};

    template <template <typename, typename...> class Monad, typename T>
    struct is_monadic_type<Monad<T>> : is_monad<Monad> {};

    template <typename T>
    decltype(auto) join(T& x) {
        static_assert(is_monadic_type<T>{}() && is_nested_container<T>{}(),
            "join can only be called on an argument of type Monad<Monad<T>> where T is an arbitrary type.");
        return x >>= [] (auto val) { return val; };
    }

    template <template <typename, typename...> class Monad, typename T, typename funcType>
    auto fmap(funcType&& f, const Monad<T>& x) {
        static_assert(is_monad<Monad>{}(), "expected type: Monad<T> for some monad type constructor 'Monad' and some type T \n actual type: ");
        return x >>= ( [_f = curry(std::forward<funcType>(f))] (const T& val) {
            return  Monad<decltype(_f(std::declval<T>()))> {_f(val)}; });
    }
    template <template <typename, typename...> class Monad, typename T, typename funcType>
    auto fmap(funcType&& f, Monad<T>&& x) {
        static_assert(is_monad<Monad>{}(), "");
        return std::move(x) >>= ([ _f = curry(std::forward<funcType>(f))] (T&& val) {
            return Monad<decltype(_f(std::declval<T>()))> { _f(std::move(val)) }; });
    }

    template <template <typename, typename...> class Monad, typename T, typename funcType>
    auto ap(const Monad<funcType>& wrappedFn, const Monad<T>& x) {
        return wrappedFn >>= [x] (auto&& x1) { return x >>= [x1 = curry(std::forward<decltype(x1)>(x1))] (auto&& x2) {
            return Monad<decltype(curry(std::declval<funcType>())(std::declval<T>()))> { x1 (std::forward<decltype(x2)>(x2)) }; }; };
    }

    template <template <typename, typename...> class Monad, typename T>
    Monad<std::remove_const_t<std::remove_reference_t<T>>> pure(T&& val) {
        return Monad<std::remove_const_t<std::remove_reference_t<T>>> { std::forward<decltype(val)>(val) };
    }

    template <template <typename, typename...> class Monad, typename funcType>
    auto liftM(funcType&& f) {
        return curry([_f = std::forward<decltype(f)>(f)] (auto x) {
            return ap(pure<Monad>(_f), x);
        });
    }

    template <template <typename, typename...> class Monad, typename funcType>
    auto liftM2(funcType&& f) {
        return curry([_f = std::forward<decltype(f)>(f)] (auto x, auto y) {
            return ap(ap(pure<Monad>(_f), x), y);
        });
    }

}
#endif
