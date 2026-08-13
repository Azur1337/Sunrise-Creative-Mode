#include "logs.h"

#include "internal.h"

namespace sunrise {
namespace core {
namespace ui {
namespace modules {
namespace logs {
namespace {

State g_state;

} // namespace

State& state() noexcept {
    return g_state;
}

void dispatch_pending_copy(HWND) noexcept {}

} // namespace logs
} // namespace modules
} // namespace ui
} // namespace core
} // namespace sunrise
