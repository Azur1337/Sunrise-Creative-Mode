#include "client_ui_module_runtime.h"

#include <string_view>

#include "../../../core/ui/modules/registry/ui_module_registry.h"
#include "../../../core/ui/modules/ui_module_descriptor.h"
#include "../creative/creative_panel.h"
#include "../teleport/teleport_panel.h"

namespace sunrise::client::ui::runtime {
namespace {

constexpr std::string_view kCreativeStableId = "client.creative";
constexpr std::string_view kCreativeDisplayName = "Creative";
constexpr std::string_view kTeleportStableId = "client.teleport";
constexpr std::string_view kTeleportDisplayName = "Teleport";

core::ui::modules::registry::PageRegistration g_creativePage;
core::ui::modules::registry::PageRegistration g_teleportPage;

} // namespace

/** @return True when the Client module owns its Core UI registry slot. */
bool initialize() noexcept {
    if (!g_creativePage.acquire(
            core::ui::modules::Owner::client, kCreativeStableId, kCreativeDisplayName, &creative::draw)) {
        return false;
    }
    if (!g_teleportPage.acquire(
            core::ui::modules::Owner::client, kTeleportStableId, kTeleportDisplayName, &teleport::draw)) {
        g_creativePage.release();
        return false;
    }
    return true;
}

void shutdown() noexcept {
    g_teleportPage.release();
    g_creativePage.release();
}

} // namespace sunrise::client::ui::runtime
