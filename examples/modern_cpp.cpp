// Optional toolchain experiment; not an engine dependency.
#include "kestrel/types.hpp"
#include <expected>
#include <print>
#include <utility>

struct Depth {
    int value;
    constexpr int get(this const Depth &self) { return self.value; }
};

constexpr int contextValue() {
    if consteval {
        return 1;
    } else {
        return 2;
    }
}
static_assert(contextValue() == 1);
static_assert(Depth{3}.get() == 3);
static_assert(std::to_underlying(kestrel::Color::White) == 0);

#if __cplusplus > 202302L && defined(__cpp_pack_indexing) &&                   \
    __cpp_pack_indexing >= 202311L
template <class... Types> using First = Types...[0];
static_assert(sizeof(First<int, char>) == sizeof(int));
#endif

int main() {
    std::expected<int, const char *> depth = Depth{3}.get();
    auto next = depth.transform([](int value) { return value + 1; });
    std::println("C++23: expected/transform, println, explicit object "
                 "parameters, if consteval, to_underlying");
#if __cplusplus > 202302L && defined(__cpp_pack_indexing) &&                   \
    __cpp_pack_indexing >= 202311L
    std::println("C++26: pack indexing available");
#else
    std::println("C++26: pack indexing not enabled");
#endif
    return next && *next == 4 && contextValue() == 2 ? 0 : 1;
}
