#include "logs.h"

#include "../../../../../vendor/imgui/imgui.h"

#include "filters/logs_filter_controls.h"

namespace sunrise {
namespace core {
namespace ui {
namespace modules {
namespace logs {

void draw() noexcept {
    filters::draw();
    ImGui::TextUnformatted("Logs module is currently minimal in this branch.");
}

} // namespace logs
} // namespace modules
} // namespace ui
} // namespace core
} // namespace sunrise
