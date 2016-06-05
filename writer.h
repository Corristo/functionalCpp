#ifndef WRITER_H
#define WRITER_H
#include <list>
#include <memory>
#include <string>
#include <tuple>
#include <type_traits>
#include "cpp17.h"
#include "curry.h"
#include "monad.h"
#include "monoid.h"


namespace monad {
    namespace Writer {
        template <typename T, typename W = std::list<std::string>>
        class Writer {
        public:
            Writer(T val)
                : _log{}
                , _val{val} {
            }

            Writer(T val, W log)
                : _log{log}
                , _val{val} {
            }

            W execWriter() const {
                return _log;
            }

            auto runWriter() const {
                return std::make_pair(_val, _log);
            }

            template <typename funcType>
            auto operator>>=(funcType&& f) const;

        private:
            W _log;
            std::remove_reference_t<T> _val;
            // bind if f returns writer<void, W>
            template <typename funcType>
            auto bindImpl(std::true_type, funcType&& f) const;

            // bind if f returns writer<T, W> with T != void
            template <typename funcType>
            auto bindImpl(std::false_type, funcType&& f) const;
        };

        // specialization for void
        template <typename W>
        class Writer<void, W> {
        public:
            Writer()
                : _log{} {
            }

            Writer(W log)
                : _log{log} {
            }

            W execWriter() const {
                return _log;
            }

            template <typename funcType>
            auto operator>>=(funcType&& f) const;

        private:
            W _log;
            // bind if f returns writer<void, W>
            template <typename funcType>
            auto bindImpl(std::true_type, funcType&& f) const;

            // bind if f returns writer<T, W> with T!= void
            template <typename funcType>
            auto bindImpl(std::false_type, funcType&& f) const;
        };

        namespace traits {
            template <typename>
            struct is_writer : std::false_type{};

            template <typename T, typename W>
            struct is_writer<Writer<T, W>> : ::traits::is_monoid<W> {};

            template <typename T>
            constexpr bool is_writer_v = is_writer<T>::value;

            template <typename>
            struct writer_base;

            template <typename T, typename W>
            struct writer_base<Writer<T,W>> : type_is<T> {};

            template <typename>
            struct writer_log;

            template <typename T, typename W>
            struct writer_log<Writer<T,W>> : type_is<W> {};

            template <typename T>
            using writer_base_t = typename writer_base<T>::type;

            template <typename T>
            using writer_log_t = typename writer_log<T>::type;
        } // namespace monad::writer::traits

        template <typename T, typename W>
        template <typename funcType>
        auto Writer<T,W>::bindImpl(std::false_type, funcType&& f) const {
            auto res_f = f(_val).runWriter();
            return Writer<traits::writer_base_t<decltype(f(std::declval<T>()))>, W> { std::get<0>(res_f), _log + std::get<1>(res_f) };
        }

        template <typename T, typename W>
        template <typename funcType>
        auto Writer<T, W>::bindImpl(std::true_type, funcType&& f) const {
            return Writer<void, W> { _log + f(_val).execWriter()};
        }

        template <typename T, typename W>
        template <typename funcType>
        auto Writer<T,W>::operator>>=(funcType&& f) const {
           //TODO: static asserts
           return bindImpl(std::is_same<traits::writer_base_t<decltype(f(std::declval<T>()))>, void>{}, std::forward<funcType>(f));
        }

        template <typename W>
        template <typename funcType>
        auto Writer<void, W>::bindImpl(std::true_type, funcType&& f) const {
           return Writer<void, W> { _log + f().execWriter() };
        }

        template <typename W>
        template <typename funcType>
        auto Writer<void, W>::bindImpl(std::false_type, funcType&& f) const {
            traits::writer_base_t<decltype(f())> f_val;
            W f_log;
            std::tie(f_val, f_log) = f().runWriter();
            return Writer<traits::writer_base_t<decltype(f())>, W> { f_val, _log + f_log };
        }

        template <typename W>
        template <typename funcType>
        auto Writer<void, W>::operator>>=(funcType&& f) const {
            // TODO: static asserts
            return bindImpl(std::is_same<traits::writer_base_t<decltype(f())>, void>{}, std::forward<decltype(f)>(f));
        }

        template <typename T, typename W>
        auto writer(T&& val, W&& log) {
            return Writer<std::decay_t<std::remove_reference_t<T>>, W>(std::forward<decltype(val)>(val), std::forward<decltype(log)>(log));
        }

        template<typename W>
        auto tell(W&& log) {
            return Writer<void, W> { std::forward<decltype(log)>(log) };
        }

        template <typename T, typename W>
        auto runWriter(const Writer<T, W>& writer) {
            return writer.runWriter();
        }

        template <typename T, typename W>
        auto execWriter(const Writer<T, W>& writer) {
            return writer.execWriter();
        }
    } // namespace monad::Writer
} // namespace monad
#endif
