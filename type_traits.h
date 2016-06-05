#ifndef OWN_TYPE_TRAITS_H
#define OWN_TYPE_TRAITS_H
#include <type_traits>


template <typename T>
struct type_is {
    typedef T type;
};

template <typename T>
using type_t = T;



// check wether a given type is of the form Container<T> for
// some type T and a fixed type constructor Container.
template <template <typename, typename...> class, typename>
struct is_container : std::false_type {};

template <template <typename, typename...> class Container, template <typename, typename...> class actualContainer, typename T>
struct is_container<Container, actualContainer<T>> : std::is_same<Container<T>, actualContainer<T>> {};

// For a fixed type constructor Container and a given type
// T, returns type U if T = Container<U> for some type U and returns
// nothing if T is not of the form Container<U> for any type U.
template <template <typename, typename...> class, typename>
struct remove_container;

template <template <typename, typename...> class ContainerToRemove, template <typename, typename...> class ActualContainer, typename T>
struct remove_container<ContainerToRemove, ActualContainer<T>>
    : std::enable_if<std::is_same<ContainerToRemove<T>, ActualContainer<T>>{}(), T>
{};

// convenience wrapper around remove_container
template <template <typename, typename...> class ContainerToRemove, typename T>
using remove_container_t = typename remove_container<ContainerToRemove, T>::type;

// checks whether a given type T is of the form T = Container<Container<U>>
// for some type constructor Container and some type U.
template <typename>
struct is_nested_container : std::false_type {};

template <template <typename, typename...> class Container, typename T>
struct is_nested_container<Container<Container<T>>> : std::true_type {};

#endif
