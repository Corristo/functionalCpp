#ifndef CPP17_H
#define CPP17_H
#include <utility>
#include <tuple>
#include <functional>
#include <type_traits>

/********************************************
 *  c++17 additional type_traits            *
 ********************************************/
template <typename...>
using void_t = void;

template <typename, typename = void>
struct is_callable : std::false_type {};

template <typename F, typename... Args>
struct is_callable<F(Args...), void_t<std::result_of_t<F(Args...)>>> : std::true_type {};

// constexpr bool convenience wrappers
template <typename F, typename...Args>
constexpr bool is_callable_v = is_callable<F(Args...)>::value;

template <typename T, typename U>
constexpr bool is_same_v = std::is_same<T, U>::value;

template <typename T>
constexpr bool is_trivially_default_constructible_v = std::is_trivially_default_constructible<T>::value;

template <typename T>
constexpr bool is_default_constructible_v = std::is_default_constructible<T>::value;

template< class T >
constexpr bool is_function_v = std::is_function<T>::value;

template< class Base, class Derived >
constexpr bool is_base_of_v = std::is_base_of<Base, Derived>::value;

template< class T >
constexpr bool is_member_pointer_v = std::is_member_pointer<T>::value;

/******************************************
 * c++17 std::invoke in <functional>      *
 ******************************************/
namespace detail {
template <class T>
struct is_reference_wrapper : std::false_type {};
template <class U>
struct is_reference_wrapper<std::reference_wrapper<U>> : std::true_type {};
template <class T>
constexpr bool is_reference_wrapper_v = is_reference_wrapper<T>::value;

template <class Base, class T, class Derived, class... Args>
auto INVOKE(T Base::*pmf, Derived&& ref, Args&&... args)
    noexcept(noexcept((std::forward<Derived>(ref).*pmf)(std::forward<Args>(args)...)))
 -> std::enable_if_t<is_function_v<T> &&
                     is_base_of_v<Base, std::decay_t<Derived>>,
    decltype((std::forward<Derived>(ref).*pmf)(std::forward<Args>(args)...))>
{
      return (std::forward<Derived>(ref).*pmf)(std::forward<Args>(args)...);
}

template <class Base, class T, class RefWrap, class... Args>
auto INVOKE(T Base::*pmf, RefWrap&& ref, Args&&... args)
    noexcept(noexcept((ref.get().*pmf)(std::forward<Args>(args)...)))
 -> std::enable_if_t<is_function_v<T> &&
                     is_reference_wrapper_v<std::decay_t<RefWrap>>,
    decltype((ref.get().*pmf)(std::forward<Args>(args)...))>

{
      return (ref.get().*pmf)(std::forward<Args>(args)...);
}

template <class Base, class T, class Pointer, class... Args>
auto INVOKE(T Base::*pmf, Pointer&& ptr, Args&&... args)
    noexcept(noexcept(((*std::forward<Pointer>(ptr)).*pmf)(std::forward<Args>(args)...)))
 -> std::enable_if_t<is_function_v<T> &&
                     !is_reference_wrapper_v<std::decay_t<Pointer>> &&
                     !is_base_of_v<Base, std::decay_t<Pointer>>,
    decltype(((*std::forward<Pointer>(ptr)).*pmf)(std::forward<Args>(args)...))>
{
      return ((*std::forward<Pointer>(ptr)).*pmf)(std::forward<Args>(args)...);
}

template <class Base, class T, class Derived>
auto INVOKE(T Base::*pmd, Derived&& ref)
    noexcept(noexcept(std::forward<Derived>(ref).*pmd))
 -> std::enable_if_t<!is_function_v<T> &&
                     is_base_of_v<Base, std::decay_t<Derived>>,
    decltype(std::forward<Derived>(ref).*pmd)>
{
      return std::forward<Derived>(ref).*pmd;
}

template <class Base, class T, class RefWrap>
auto INVOKE(T Base::*pmd, RefWrap&& ref)
    noexcept(noexcept(ref.get().*pmd))
 -> std::enable_if_t<!is_function_v<T> &&
                     is_reference_wrapper_v<std::decay_t<RefWrap>>,
    decltype(ref.get().*pmd)>
{
      return ref.get().*pmd;
}

template <class Base, class T, class Pointer>
auto INVOKE(T Base::*pmd, Pointer&& ptr)
    noexcept(noexcept((*std::forward<Pointer>(ptr)).*pmd))
 -> std::enable_if_t<!is_function_v<T> &&
                     !is_reference_wrapper_v<std::decay_t<Pointer>> &&
                     !is_base_of_v<Base, std::decay_t<Pointer>>,
    decltype((*std::forward<Pointer>(ptr)).*pmd)>
{
      return (*std::forward<Pointer>(ptr)).*pmd;
}

template <class F, class... Args>
auto INVOKE(F&& f, Args&&... args)
    noexcept(noexcept(std::forward<F>(f)(std::forward<Args>(args)...)))
 -> std::enable_if_t<!is_member_pointer_v<std::decay_t<F>>,
    decltype(std::forward<F>(f)(std::forward<Args>(args)...))>
{
      return std::forward<F>(f)(std::forward<Args>(args)...);
}
} // namespace detail

template< class F, class... ArgTypes >
auto invoke(F&& f, ArgTypes&&... args)
    // exception specification for QoI
    noexcept(noexcept(detail::INVOKE(std::forward<F>(f), std::forward<ArgTypes>(args)...)))
 -> decltype(detail::INVOKE(std::forward<F>(f), std::forward<ArgTypes>(args)...))
{
    return detail::INVOKE(std::forward<F>(f), std::forward<ArgTypes>(args)...);
}

/*************************************************************
 * slightly improved version of c++17 std::apply in <tuple>  *
 *************************************************************/
namespace detail {
template <class F, class Tuple, std::size_t... I>
constexpr decltype(auto) apply_impl( F&& f, Tuple&& t, std::index_sequence<I...> )
{
  return invoke(std::forward<F>(f), std::get<I>(std::forward<Tuple>(t))...);
  // Note: std::invoke is a C++17 feature
}
} // namespace detail

template <class F, class... Tuples>
constexpr decltype(auto) apply(F&& f, Tuples&&... t)
{
    return detail::apply_impl(std::forward<F>(f), std::forward<decltype(std::tuple_cat(std::declval<decltype(t)>()...))>(std::tuple_cat(t...)),
        std::make_index_sequence<std::tuple_size<std::decay_t<decltype(std::tuple_cat(std::declval<decltype(t)>()...))>>{}>{});
}

#endif
