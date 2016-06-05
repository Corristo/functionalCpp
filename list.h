#ifndef LIST_H
#define LIST_H
#include <list>
#include "cpp17.h"
#include "type_traits.h"


template <typename T, typename funcType>
auto operator>>= (const std::list<T>& list, funcType&& f)
    -> std::enable_if_t<is_container<std::list, decltype(f(std::declval<T>()))>{}(), decltype(f(std::declval<T>()))> {
        decltype(f(std::declval<T>())) returnList{};
        for (const auto& elem : list) {
            for (auto&& elem2 : f(elem)) {
                returnList.push_back(std::forward<decltype(elem2)>(elem2));
            }
        }
        return returnList;
}

template <typename T, typename funcType>
auto operator>>= (std::list<T>&& list, funcType&& f) -> std::enable_if_t<is_container<std::list, decltype(f(std::declval<T>()))>{}(), decltype(f(std::declval<T>()))> {
    decltype(f(std::declval<T>())) returnList{};
    for (auto&& elem : std::move(list)) {
        for (auto&& elem2: f(std::move(elem))) {
            returnList.push_back(std::move(elem2));
        }
    }
    return returnList;
}
#endif
