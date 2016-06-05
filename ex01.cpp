#include "list.h"
#include "monad.h"
#include "monoid.h"
#include "optional.h"
#include "writer.h"
#include <iostream>
#include <string>

// square functor similar to std::plus, std::negate,...
template <typename T = void>
struct square {
    T operator()(T&& x) {
        return x * std::forward<decltype(x)>(x);
    }
};

template <>
struct square<void> {
    template <typename T>
    T operator()(T&& x) const {
        return x * std::forward<decltype(x)>(x);
    }
};

int main() {
    /*************************************
     *      Exercise 1                   *
     *************************************/
    // optional is implemented in optional.h
    auto squareOpt = monad::liftM<optional>(square<>{});
    std::cout << "Testing squareOpt...\n"
              << squareOpt(monad::pure<optional>(8)).to_string() << std::endl
              << squareOpt(optional<int>{}).to_string() << std::endl
              << std::endl;

    auto sumOpt = monad::liftM2<optional>(std::plus<>{});
    std::cout << "Testing sumOpt...\n"
              << sumOpt(monad::pure<optional>(41),
                        monad::pure<optional>(1))
                 .to_string() << std::endl
              << sumOpt(optional<int>{},
                        monad::pure<optional>(1))
                 .to_string() << std::endl
              << sumOpt(monad::pure<optional>(41),
                        optional<int>{})
                 .to_string() << std::endl
              << sumOpt(optional<float>{},
                        optional<float>{})
                 .to_string() << std::endl
              << std::endl;

    auto sumOfSquaresOpt = [&sumOpt, &squareOpt] (auto a, auto b) {
        return sumOpt (squareOpt(a)) (squareOpt(b));
    };
    std::cout << "Testing sumOfSquaresOpt...\n"
              << sumOfSquaresOpt(monad::pure<optional>(3),
                                 monad::pure<optional>(4))
                 .to_string() << std::endl
              << std::endl;

    /*************************************
     *      Exercice 2                   *
     *************************************/

    // The direction pure + bind => pure + map + join is implemented in
    // monad.h
    //
    // For the converse direction note that x >>= f == join(map(f, x))

    /*************************************
     *     Exercise 3                    *
     *************************************/
    // The monad instance for std::list is implemented in list.h

    auto printList = [] (auto list) {
        std::cout << "[ ";
        for (const auto& e : list) {
            std::cout << e << " ";
        }
        std::cout << "]" << std::endl;
    };

    auto squareList = monad::liftM<std::list>(square<>{});
    std::cout << "Testing squareList...\n";
                 printList(squareList(std::list<int>{1, 2, 3, 4}));
                 printList(squareList(std::list<int>{}));
    std::cout << std::endl;

    auto sumList = monad::liftM2<std::list>(std::plus<>{});
    std::cout << "Testing sumList (with implicit conversions)...\n";
                 printList(sumList(std::list<double>{1.0, 3.3},
                                   std::list<int>{4, -1}));
                printList(sumList(std::list<std::string> {"Hello_", "Bye_"},
                                  std::list<const char *> {"World", "Galaxy", "Universe"}));
    std::cout << std::endl;

    auto sumOfSquaresList = [&sumList, &squareList] (auto a, auto b) {
        return sumList(squareList(a), squareList(b));
    };

    std::cout << "Testing sumOfSquareList...\n";
                 printList(sumOfSquaresList(monad::pure<std::list>(-10),
                                            std::list<int>{3, 4, 5}));
    std::cout << std::endl;

    /*************************************
     *     Exercise 4                    *
     *************************************/
    // for the skae of brevity I import *all* the namespaces
using namespace std;
using namespace monad;
using namespace monad::Writer;
using namespace std::literals;

   auto logNumber = [](auto x) {
       return writer(x, pure<list>("Got number: " + to_string(x)));
    };

   auto logAddition = [&logNumber] (auto x, auto y) {
       return liftM2<monad::Writer::Writer>(plus<>{})(logNumber(x), logNumber(y));
    };

   int result;
   list<string> log;

   auto startComputation = tell(pure<list>("Starting computation..."s));

   std::tie(result, log) = runWriter(
           (startComputation
       >>= [&logAddition] { return logAddition(3, 4); })
       >>= [] (auto res) { return writer(res, pure<list>("Result is: " + to_string(res)));});

   std::cout << "Result of logAddition(3, 4): " << result << std::endl;
   printList(log);
   std::cout << std::endl;

   /**************************************
    *     Exercise 5                     *
    **************************************/

   // In writer.h there is an implementation of a Writer_l where l is an
   // arbitrty monoid (see monoid.h)
   // The default is std::list<std::string>, i.e. StringListWriter used in
   // Exercise 4. If one wants to use another monoid for logginig, one has to
   // first declare a templated struct, e.g. for an Writer_int as follows:
   //
   // Make sure that the type you're using actually is a monoid, i.e. supports
   // default construction and operator+
   //
   // Note: Unfortunately, the current 'pure' implementation does not fully support
   // non-unary type constructors yet, so one cannot lift values to a Writer_l
   // using that function. As a workaround one can of course use the constructor
   // (where one has to annotate the types - ugly), or call writer(value, log)
   // with log being the zero element of the monoid one wishes to use.


   auto countOperation = [] (auto x) { return writer(x, 1); };
   auto computation = writer("the answer to life, the universe and everything", 0);
   for (int i = 0; i < 42; i++) {
        computation = computation >>= countOperation;
   }
   auto res = runWriter(computation);
   std::cout << "Result of computation: " << get<0>(res) << std::endl;
   std::cout << "'log' of computation: " << get<1>(res) << std::endl;
}



