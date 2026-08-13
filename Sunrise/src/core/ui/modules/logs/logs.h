#pragma once

#include <Windows.h>

namespace sunrise {
namespace core {
namespace ui {
namespace modules {
namespace logs {

[[nodiscard]] bool initialize() noexcept;
void shutdown() noexcept;
void draw() noexcept;
void dispatch_pending_copy(HWND window) noexcept;

} // namespace logs
} // namespace modules
} // namespace ui
} // namespace core
} // namespace sunrise
