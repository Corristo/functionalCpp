#ifndef MONOID_H
#define MONOID_H
#include <type_traits>
#include "cpp17.h"

// monoid instance for std::list
template <typename T>
std::list<T> operator+(const std::list<T>& l1, const std::list<T>& l2) {
    std::list<T> returnList{l1};
    for (const auto& elem : l2) {
        returnList.push_back(elem);
    }
    return returnList;
}

namespace traits {
    template <typename T>
    using mappend_t = decltype(std::declval<T>() + std::declval<T>());

    template <typename, typename = void>
    struct is_monoid : std::false_type {};

    template <typename T>
    struct is_monoid<T, void_t<mappend_t<T>>> : std::integral_constant<bool, is_same_v<mappend_t<T>, T> && is_default_constructible_v<T>> {};

    template <typename T>
    constexpr bool is_monoid_v = is_monoid<T>::value;
} // namespace traits

#endif
