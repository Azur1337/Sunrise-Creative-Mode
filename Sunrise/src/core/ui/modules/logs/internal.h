#pragma once

namespace sunrise {
namespace core {
namespace ui {
namespace modules {
namespace logs {

struct State {
    bool initialized{};
};

State& state() noexcept;

} // namespace logs
} // namespace modules
} // namespace ui
} // namespace core
} // namespace sunrise
