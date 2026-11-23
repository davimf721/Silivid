#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

// Semantic Versioning 2.0.0 parsing and precedence. Build metadata is rejected
// because release tags here must identify exactly one version.
namespace veyra::semver {

struct Version {
    std::array<std::uint64_t, 3> core{};
    std::vector<std::string> prerelease;
    bool isPrerelease() const { return !prerelease.empty(); }
};

std::optional<Version> parse(std::string_view text);
// Negative when a < b, zero when equal, positive when a > b.
int compare(const Version& a, const Version& b);

} // namespace veyra::semver
