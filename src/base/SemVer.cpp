#include "veyra/base/SemVer.h"

#include <algorithm>
#include <charconv>

namespace veyra::semver {
namespace {

bool isDigit(char c) { return c >= '0' && c <= '9'; }
bool isIdentifierChar(char c) {
    return isDigit(c) || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '-';
}
bool isNumeric(std::string_view text) { return !text.empty() && std::all_of(text.begin(), text.end(), isDigit); }

std::optional<std::uint64_t> number(std::string_view text) {
    if (!isNumeric(text) || (text.size() > 1 && text.front() == '0')) return std::nullopt;
    std::uint64_t value = 0;
    const auto* end = text.data() + text.size();
    const auto result = std::from_chars(text.data(), end, value);
    if (result.ec != std::errc{} || result.ptr != end) return std::nullopt;
    return value;
}

int sign(auto a, auto b) { return a < b ? -1 : (b < a ? 1 : 0); }

int compareIdentifier(const std::string& a, const std::string& b) {
    const bool numericA = isNumeric(a), numericB = isNumeric(b);
    if (numericA != numericB) return numericA ? -1 : 1;
    // Parsed numeric identifiers have no leading zeros, so length orders them first.
    if (numericA && a.size() != b.size()) return sign(a.size(), b.size());
    return sign(std::string_view(a), std::string_view(b));
}

} // namespace

std::optional<Version> parse(std::string_view text) {
    Version version;
    const auto dash = text.find('-');
    std::string_view core = text.substr(0, dash);
    for (std::size_t i = 0; i < version.core.size(); ++i) {
        const auto dot = core.find('.');
        if ((i < 2) == (dot == std::string_view::npos)) return std::nullopt;
        const auto value = number(core.substr(0, dot));
        if (!value) return std::nullopt;
        version.core[i] = *value;
        if (i < 2) core.remove_prefix(dot + 1);
    }
    if (dash == std::string_view::npos) return version;

    std::string_view rest = text.substr(dash + 1);
    while (true) {
        const auto dot = rest.find('.');
        const auto identifier = rest.substr(0, dot);
        if (identifier.empty() || !std::all_of(identifier.begin(), identifier.end(), isIdentifierChar)) return std::nullopt;
        if (isNumeric(identifier) && !number(identifier)) return std::nullopt;
        version.prerelease.emplace_back(identifier);
        if (dot == std::string_view::npos) return version;
        rest.remove_prefix(dot + 1);
    }
}

int compare(const Version& a, const Version& b) {
    for (std::size_t i = 0; i < a.core.size(); ++i)
        if (a.core[i] != b.core[i]) return sign(a.core[i], b.core[i]);
    if (a.isPrerelease() != b.isPrerelease()) return a.isPrerelease() ? -1 : 1;
    const auto shared = std::min(a.prerelease.size(), b.prerelease.size());
    for (std::size_t i = 0; i < shared; ++i)
        if (const int order = compareIdentifier(a.prerelease[i], b.prerelease[i])) return order;
    return sign(a.prerelease.size(), b.prerelease.size());
}

} // namespace veyra::semver
