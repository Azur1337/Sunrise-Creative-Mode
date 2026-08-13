#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace sunrise {
namespace client {
namespace creative {

constexpr std::uint32_t kUnconfiguredHash = 0;
constexpr std::size_t kCharacterCapacity = 3;

struct CharacterWeapons {
    std::uint32_t kinetic{kUnconfiguredHash};
    std::uint32_t energy{kUnconfiguredHash};
    std::uint32_t heavy{kUnconfiguredHash};
};

struct Settings {
    bool enabled{false};
    std::array<CharacterWeapons, kCharacterCapacity> characters{};
};

void initialize(void* module) noexcept;
void shutdown() noexcept;
[[nodiscard]] Settings get() noexcept;
bool publish(const Settings& settings) noexcept;

} // namespace creative
} // namespace client
} // namespace sunrise
