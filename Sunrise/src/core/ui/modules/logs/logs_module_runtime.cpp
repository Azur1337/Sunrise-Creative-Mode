#include "logs.h"

#include <string_view>

#include "../registry/ui_module_registry.h"

namespace sunrise {
namespace core {
namespace ui {
namespace modules {
namespace logs {
namespace {

constexpr std::string_view kStableId = "core.logs";
constexpr std::string_view kDisplayName = "Logs";
registry::PageRegistration g_page;

} // namespace

[[nodiscard]] bool initialize() noexcept {
    return g_page.acquire(Owner::core, kStableId, kDisplayName, &draw);
}

void shutdown() noexcept {
    g_page.release();
}

} // namespace logs
} // namespace modules
} // namespace ui
} // namespace core
} // namespace sunrise
