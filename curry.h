#ifndef CURRY_H
#define CURRY_H
#include <utility>
#include <tuple>
#include <functional>
#include <type_traits>
#include "cpp17.h"


namespace detail {

    template <typename F, typename... CapturedArgs>
    class curried_fn {
    public:

        curried_fn(const F& f)
            : _f{f}
            , _capturedArgs{std::make_tuple()} {
        }

        curried_fn(const F&& f)
            : _f{std::move(f)} {
        }

        curried_fn(const F& f, const std::tuple<CapturedArgs...>& capturedArgs)
            : _f{f}
            , _capturedArgs{capturedArgs} {
        }

        template <typename Arg, typename... Args>
        auto operator()(Arg&& arg, Args&&... args) const  -> std::enable_if_t<!is_callable<F(CapturedArgs..., Arg, Args...)>::value, curried_fn<F, CapturedArgs..., std::decay_t<Arg>, std::decay_t<Args>...>> {
            return curried_fn<F, CapturedArgs..., std::decay_t<Arg>, std::decay_t<Args>...>(_f, std::tuple_cat(_capturedArgs, std::make_tuple(arg, args...)));
        }

        template <typename... Args>
        auto operator()(Args&&... args) const -> std::enable_if_t<is_callable<F(CapturedArgs..., Args...)>::value, std::remove_reference_t<std::result_of_t<F(CapturedArgs..., Args...)>>> {
           return apply(_f, _capturedArgs, std::make_tuple(std::forward<Args>(args)...));
        }

        template <typename Arg, typename... Args>
        auto operator()(Arg&& arg, Args&&... args) -> std::enable_if_t<!is_callable<F(CapturedArgs..., Arg, Args...)>::value, curried_fn<std::remove_const_t<F>, CapturedArgs..., std::decay_t<Arg>, std::decay_t<Args>...>> {
            return curried_fn<std::remove_const_t<F>, CapturedArgs..., std::decay_t<Arg>, std::decay_t<Args>...>(std::move(_f), std::tuple_cat(std::move(_capturedArgs), std::make_tuple(arg, args...)));
        }

        template <typename... Args>
        auto operator()(Args&&... args) -> std::enable_if_t<is_callable<F(CapturedArgs..., Args...)>::value, std::result_of_t<F(CapturedArgs..., Args...)>> {
           return apply(std::move(_f), std::move(_capturedArgs), std::make_tuple(std::forward<Args>(args)...));
        }
    private:
        F _f;
        std::tuple<CapturedArgs...> _capturedArgs;
    };

} // namespace detail

namespace traits {
    template <typename>
    struct is_curried_fn : std::false_type {};

    template <typename F, typename... Args>
    struct is_curried_fn<detail::curried_fn<F, Args...>> : std::true_type {};

    template <typename T>
    constexpr bool is_curried_fn_v = is_curried_fn<std::remove_reference_t<T>>::value;
} // namespace traits

template <typename F>
auto curry(F&& f) -> std::enable_if_t<!traits::is_curried_fn_v<F>, detail::curried_fn<std::remove_const_t<std::remove_reference_t<F>>>> {
    detail::curried_fn<std::remove_const_t<std::remove_reference_t<F>>> retVal(std::forward<F>(f));
    return retVal;
}

template <typename F, typename... Args>
detail::curried_fn<F, Args...> curry(const detail::curried_fn<F, Args...>& curriedFn) {
    return curriedFn;
}


#endif
