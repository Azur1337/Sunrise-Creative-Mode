#include "creative_panel.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdio>

#include "../../../../vendor/imgui/imgui.h"

#include "../../../core/ui/components/toggle/ui_toggle_component.h"
#include "../../../state/runtime/runtime.h"
#include "../../creative/creative_settings_store.h"

namespace sunrise {
namespace client {
namespace ui {
namespace creative {
namespace {

namespace creative_store = sunrise::client::creative;

std::size_t g_selectedCharacter{};

[[nodiscard]] bool hash_input(const char* label, std::uint32_t& value) noexcept {
    std::uint32_t edited = value;
    if (!ImGui::InputScalar(label,
                            ImGuiDataType_U32,
                            &edited,
                            nullptr,
                            nullptr,
                            "0x%08X",
                            ImGuiInputTextFlags_CharsHexadecimal)) {
        return false;
    }
    value = edited;
    return true;
}

} // namespace

void draw() noexcept {
    creative_store::Settings settings = creative_store::get();
    const state::AccountState account = state::account_snapshot();
    const std::size_t characterCount = (std::min)(account.characterCount, settings.characters.size());

    ImGui::TextUnformatted("Creative Mode");
    ImGui::Separator();
    ImGui::TextWrapped(
        "Edit equipped weapon hashes per character. Values are saved to creative_loadout.json.");
    ImGui::Spacing();

    bool changed = core::ui::components::toggle::control("Enabled", settings.enabled);

    if (characterCount == 0) {
        ImGui::Spacing();
        ImGui::TextDisabled("No authored characters are available.");
        if (changed) {
            (void)creative_store::publish(settings);
        }
        return;
    }
    if (g_selectedCharacter >= characterCount) {
        g_selectedCharacter = 0;
    }

    std::array<std::array<char, 48>, state::kCharacterCapacity> labels{};
    for (std::size_t index = 0; index < characterCount; ++index) {
        (void)std::snprintf(labels[index].data(),
                            labels[index].size(),
                            "Character %zu (0x%llX)",
                            index + 1,
                            static_cast<unsigned long long>(account.characters[index].soid));
    }

    ImGui::Spacing();
    ImGui::TextUnformatted("Character");
    ImGui::SetNextItemWidth(-1.0F);
    if (ImGui::BeginCombo("##creative_character", labels[g_selectedCharacter].data())) {
        for (std::size_t index = 0; index < characterCount; ++index) {
            const bool selected = index == g_selectedCharacter;
            if (ImGui::Selectable(labels[index].data(), selected)) {
                g_selectedCharacter = index;
            }
            if (selected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }

    creative_store::CharacterWeapons& weapons = settings.characters[g_selectedCharacter];
    ImGui::Spacing();
    changed = hash_input("Kinetic", weapons.kinetic) || changed;
    changed = hash_input("Energy", weapons.energy) || changed;
    changed = hash_input("Heavy", weapons.heavy) || changed;

    if (changed && !creative_store::publish(settings)) {
        ImGui::Spacing();
        ImGui::TextUnformatted("invalid value, not saved");
    }
}

} // namespace creative
} // namespace ui
} // namespace client
} // namespace sunrise
