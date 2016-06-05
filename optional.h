#ifndef OPTIONAL_H
#define OPTIONAL_H
#include <string>
#include <type_traits>
#include <stdexcept>

template <typename T>
class optional {
public:
    optional()
        : _nothing(true) {};

    optional(T val)
        : _nothing(false)
        , _val{val} {
    }

    bool is_nothing() const {
        return _nothing;
    }

    T from_optional() const {
        if (_nothing) {
            throw std::runtime_error("Accessed empty optional");
        }
        return _val;
    }

    template <typename funcType>
    auto operator>>=(funcType&& f) const {
        if (!is_nothing()) {
            return f(from_optional());
        } else {
            return decltype(f(std::declval<T>())) {};
        }
    }

    std::string to_string() const {
        if (_nothing) {
            return "Nothing";
        } else {
            return "Just " + std::to_string(_val);
        }
    }

private:
    bool _nothing = true;
    union {
        std::remove_reference_t<T> _val;
    };
};
#endif
