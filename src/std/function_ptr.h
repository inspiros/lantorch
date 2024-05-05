#pragma once

#include <type_traits>
#include <utility>
#include <new>

namespace std {
    namespace detail {
        template<int, typename Callable, typename Ret, typename... Args>
        auto _function_ptr(Callable &&c, Ret (*)(Args...)) {
            static std::decay_t<Callable> storage = std::forward<Callable>(c);
            static bool used = false;
            if (used) {
                using type = decltype(storage);
                storage.~type();
                new(&storage) type(std::forward<Callable>(c));
            }
            used = true;

            return [](Args... args) -> Ret {
                auto &c = *std::launder(&storage);
                return Ret(c(std::forward<Args>(args)...));
            };
        }
    }

    template<typename Fn, int N = 0, typename Callable>
    Fn *function_ptr(Callable &&c) {
        return detail::_function_ptr<N>(std::forward<Callable>(c), (Fn *) nullptr);
    }
}
