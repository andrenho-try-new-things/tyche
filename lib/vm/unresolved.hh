#ifndef TYCHE_UNRESOLVED_HH
#define TYCHE_UNRESOLVED_HH

#include <cstdint>
#include <optional>
#include <random>
#include <string>
#include <unordered_map>

namespace compiler {

struct UnresolvedKey {
    UnresolvedKey() {
        static std::random_device dev;
        static std::mt19937 rng(dev());

        std::uniform_int_distribution<int> dist(0, 15);

        const char *v = "0123456789abcdef";
        const bool dash[] = { 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0 };

        for (int i = 0; i < 16; i++) {
            if (dash[i]) value += "-";
            value += v[dist(rng)];
            value += v[dist(rng)];
        }
    }

    explicit UnresolvedKey(std::string s) :value(std::move(s)) {}
    explicit UnresolvedKey(const char* s) :value(s) {}

    // Prevent implicit conversion from plain std::string
    UnresolvedKey(std::string const&) = delete;

    bool operator==(UnresolvedKey const& o) const { return value == o.value; }
    [[nodiscard]] std::string const& str() const { return value; }

private:
    std::string value;
};

using UnresolvedMap = std::unordered_map<UnresolvedKey, std::optional<int32_t>>;

}

template<>
struct std::hash<compiler::UnresolvedKey> {
    size_t operator()(compiler::UnresolvedKey const& s) const noexcept {
        return std::hash<std::string>{}(s.str());
    }
};

#endif //TYCHE_UNRESOLVED_HH
