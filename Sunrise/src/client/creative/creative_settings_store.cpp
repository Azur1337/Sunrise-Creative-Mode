#include "creative_settings_store.h"

#include <Windows.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace sunrise {
namespace client {
namespace creative {
namespace {

constexpr wchar_t kSubdirectory[] = L"\\Sunrise";
constexpr wchar_t kFileSuffix[] = L"\\creative_loadout.json";
constexpr std::size_t kPathCapacity = 32768;
constexpr std::size_t kFileCapacity = 2048;

SRWLOCK g_lock{SRWLOCK_INIT};
Settings g_settings{};
std::array<wchar_t, kPathCapacity> g_path{};
bool g_pathResolved{};

void clear_path() noexcept {
    g_path.fill(wchar_t{});
}

[[nodiscard]] bool append_wide(std::array<wchar_t, kPathCapacity>& target,
                               std::size_t& length,
                               const wchar_t* suffix) noexcept {
    if (suffix == nullptr) {
        return false;
    }
    std::size_t index = 0;
    while (suffix[index] != L'\0') {
        if (length + index + 1 >= target.size()) {
            return false;
        }
        target[length + index] = suffix[index];
        ++index;
    }
    length += index;
    target[length] = L'\0';
    return true;
}

[[nodiscard]] bool resolve_path(void* module, std::array<wchar_t, kPathCapacity>& output) noexcept {
    output.fill(wchar_t{});
    const HMODULE loaded = static_cast<HMODULE>(module);
    const DWORD written = GetModuleFileNameW(loaded, output.data(), static_cast<DWORD>(output.size()));
    if (written == 0 || written >= output.size()) {
        return false;
    }

    std::size_t length = static_cast<std::size_t>(written);
    while (length > 0 && output[length - 1] != L'\\' && output[length - 1] != L'/') {
        --length;
    }
    if (length == 0) {
        return false;
    }
    output[length - 1] = L'\0';
    --length;

    if (!append_wide(output, length, kSubdirectory)) {
        return false;
    }
    (void)CreateDirectoryW(output.data(), nullptr);
    return append_wide(output, length, kFileSuffix);
}

[[nodiscard]] const char* find_scalar_start(const char* text, const char* key) noexcept {
    if (text == nullptr || key == nullptr) {
        return nullptr;
    }
    const char* at = std::strstr(text, key);
    if (at == nullptr) {
        return nullptr;
    }
    const char* colon = std::strchr(at, ':');
    if (colon == nullptr) {
        return nullptr;
    }
    const char* start = colon + 1;
    while (*start == ' ' || *start == '\t') {
        ++start;
    }
    return start;
}

[[nodiscard]] bool parse_u32_scalar(const char* scalar, std::uint32_t& output) noexcept {
    if (scalar == nullptr || *scalar == '\0') {
        return false;
    }
    char buffer[32]{};
    std::size_t copied = 0;
    const char* read = scalar;
    if (*read == '"') {
        ++read;
    }
    while (*read != '\0' && *read != '"' && *read != ',' && *read != '}' && *read != '\n'
           && *read != '\r' && copied + 1 < sizeof(buffer)) {
        buffer[copied++] = *read++;
    }
    buffer[copied] = '\0';
    if (copied == 0) {
        return false;
    }

    char* end = nullptr;
    const unsigned long value = std::strtoul(buffer, &end, 0);
    if (end == buffer || end == nullptr || *end != '\0') {
        return false;
    }
    output = static_cast<std::uint32_t>(value);
    return true;
}

void parse(const char* text, Settings& output) noexcept {
    if (text == nullptr) {
        return;
    }

    output.enabled = std::strstr(text, "\"enabled\": true") != nullptr;

    constexpr const char* kSlotNames[] = {"kinetic", "energy", "heavy"};
    for (std::size_t character = 0; character < output.characters.size(); ++character) {
        std::uint32_t* slots[] = {
            &output.characters[character].kinetic,
            &output.characters[character].energy,
            &output.characters[character].heavy,
        };
        for (std::size_t slot = 0; slot < 3; ++slot) {
            char key[64]{};
            const int keyLength =
                std::snprintf(key, sizeof(key), "\"character_%zu_%s\"", character, kSlotNames[slot]);
            if (keyLength <= 0) {
                continue;
            }
            const char* start = find_scalar_start(text, key);
            std::uint32_t value = 0;
            if (parse_u32_scalar(start, value)) {
                *slots[slot] = value;
            }
        }
    }
}

[[nodiscard]] bool valid(const Settings& settings) noexcept {
    for (const CharacterWeapons& character : settings.characters) {
        if (character.kinetic == 0x811C9DC5U || character.energy == 0x811C9DC5U
            || character.heavy == 0x811C9DC5U) {
            return false;
        }
    }
    return true;
}

[[nodiscard]] bool store(const Settings& settings) noexcept {
    if (!g_pathResolved) {
        return false;
    }
    char document[kFileCapacity]{};
    const int size = std::snprintf(document,
                                   sizeof(document),
                                   "{\n"
                                   "  \"enabled\": %s,\n"
                                   "  \"character_0_kinetic\": \"0x%08X\",\n"
                                   "  \"character_0_energy\": \"0x%08X\",\n"
                                   "  \"character_0_heavy\": \"0x%08X\",\n"
                                   "  \"character_1_kinetic\": \"0x%08X\",\n"
                                   "  \"character_1_energy\": \"0x%08X\",\n"
                                   "  \"character_1_heavy\": \"0x%08X\",\n"
                                   "  \"character_2_kinetic\": \"0x%08X\",\n"
                                   "  \"character_2_energy\": \"0x%08X\",\n"
                                   "  \"character_2_heavy\": \"0x%08X\"\n"
                                   "}\n",
                                   settings.enabled ? "true" : "false",
                                   settings.characters[0].kinetic,
                                   settings.characters[0].energy,
                                   settings.characters[0].heavy,
                                   settings.characters[1].kinetic,
                                   settings.characters[1].energy,
                                   settings.characters[1].heavy,
                                   settings.characters[2].kinetic,
                                   settings.characters[2].energy,
                                   settings.characters[2].heavy);
    if (size <= 0) {
        return false;
    }

    const HANDLE file = CreateFileW(g_path.data(),
                                    GENERIC_WRITE,
                                    0,
                                    nullptr,
                                    CREATE_ALWAYS,
                                    FILE_ATTRIBUTE_NORMAL,
                                    nullptr);
    if (file == INVALID_HANDLE_VALUE) {
        return false;
    }

    DWORD written = 0;
    bool complete =
        WriteFile(file, document, static_cast<DWORD>(size), &written, nullptr) != FALSE
        && written == static_cast<DWORD>(size);
    complete = CloseHandle(file) != FALSE && complete;
    return complete;
}

void load() noexcept {
    if (!g_pathResolved) {
        return;
    }

    const HANDLE file = CreateFileW(g_path.data(),
                                    GENERIC_READ,
                                    FILE_SHARE_READ,
                                    nullptr,
                                    OPEN_EXISTING,
                                    FILE_ATTRIBUTE_NORMAL,
                                    nullptr);
    if (file == INVALID_HANDLE_VALUE) {
        return;
    }

    char buffer[kFileCapacity]{};
    DWORD read = 0;
    const bool readOk =
        ReadFile(file, buffer, static_cast<DWORD>(sizeof(buffer) - 1), &read, nullptr) != FALSE;
    (void)CloseHandle(file);
    if (!readOk || read == 0) {
        return;
    }

    Settings parsed{};
    parse(buffer, parsed);
    if (valid(parsed)) {
        g_settings = parsed;
    }
}

} // namespace

void initialize(void* module) noexcept {
    AcquireSRWLockExclusive(&g_lock);
    g_settings = Settings{};
    clear_path();
    g_pathResolved = resolve_path(module, g_path);
    if (g_pathResolved) {
        load();
    }
    ReleaseSRWLockExclusive(&g_lock);
}

void shutdown() noexcept {
    AcquireSRWLockExclusive(&g_lock);
    g_settings = Settings{};
    clear_path();
    g_pathResolved = false;
    ReleaseSRWLockExclusive(&g_lock);
}

Settings get() noexcept {
    AcquireSRWLockShared(&g_lock);
    const Settings snapshot = g_settings;
    ReleaseSRWLockShared(&g_lock);
    return snapshot;
}

bool publish(const Settings& settings) noexcept {
    if (!valid(settings)) {
        return false;
    }
    AcquireSRWLockExclusive(&g_lock);
    g_settings = settings;
    const bool stored = store(settings);
    ReleaseSRWLockExclusive(&g_lock);
    return stored;
}

} // namespace creative
} // namespace client
} // namespace sunrise
