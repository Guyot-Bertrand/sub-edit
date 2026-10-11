#include <subedit/cli/shared_settings.hpp>
#include <subedit/core/io/in_memory_file_system.hpp>

#include <catch2/catch_test_macros.hpp>

#include <sstream>
#include <string>

using namespace subedit::cli;
using subedit::core::InMemoryFileSystem;

namespace {
const std::filesystem::path kUser = "/u/subedit/settings.conf";
const std::filesystem::path kOther = "/o/other.conf";
} // namespace

TEST_CASE("With --no-config nothing is read and the core defaults hold", "[shared-settings]") {
    InMemoryFileSystem files;
    REQUIRE(files.writeFile(kUser, "duration-adjust.speed = 12\n").has_value());
    const SharedSettings shared = loadSharedSettings(files, {.disabled = true, .userPath = kUser});
    CHECK(shared.source == SettingsSource::Disabled);
    CHECK(shared.values == subedit::core::OperationSettings{});
    CHECK(shared.path.empty());
    CHECK(shared.target == kUser);
}

TEST_CASE("By default the user's file is read", "[shared-settings]") {
    InMemoryFileSystem files;
    REQUIRE(files.writeFile(kUser, "duration-adjust.speed = 12\n").has_value());
    const SharedSettings shared = loadSharedSettings(files, {.userPath = kUser});
    CHECK(shared.source == SettingsSource::Read);
    CHECK(shared.path == kUser);
    CHECK(shared.values.durationAdjustment.charactersPerSecond == 12.0);
}

TEST_CASE("A file named by --config wins over the user's", "[shared-settings]") {
    InMemoryFileSystem files;
    REQUIRE(files.writeFile(kUser, "duration-adjust.speed = 12\n").has_value());
    REQUIRE(files.writeFile(kOther, "duration-adjust.speed = 20\n").has_value());
    const SharedSettings shared =
        loadSharedSettings(files, {.explicitPath = kOther, .userPath = kUser});
    CHECK(shared.path == kOther);
    CHECK(shared.values.durationAdjustment.charactersPerSecond == 20.0);
}

TEST_CASE("A missing file is defaults, said as missing", "[shared-settings]") {
    InMemoryFileSystem files;
    const SharedSettings shared = loadSharedSettings(files, {.userPath = kUser});
    CHECK(shared.source == SettingsSource::Missing);
    CHECK(shared.values == subedit::core::OperationSettings{});
}

TEST_CASE("An unreadable value is reported and the default kept", "[shared-settings]") {
    InMemoryFileSystem files;
    REQUIRE(files.writeFile(kUser, "duration-adjust.speed = abc\n").has_value());
    const SharedSettings shared = loadSharedSettings(files, {.userPath = kUser});
    CHECK(shared.source == SettingsSource::Read);
    REQUIRE(shared.diagnostics.size() == 1);
    CHECK(shared.values == subedit::core::OperationSettings{});
}

TEST_CASE("No resolvable user path is defaults, said as missing", "[shared-settings]") {
    const InMemoryFileSystem files;
    const SharedSettings shared = loadSharedSettings(files, {});
    CHECK(shared.source == SettingsSource::Missing);
    CHECK(shared.path.empty());
    CHECK(shared.target.empty());
    CHECK(shared.values == subedit::core::OperationSettings{});
    CHECK(shared.diagnostics.empty());
}

namespace {
std::string announced(const SharedSettings& shared, int level = 1) {
    std::ostringstream errors;
    announceSettings(Reporter{errors, level}, shared);
    return errors.str();
}
} // namespace

TEST_CASE("What was read is announced, with what it refused", "[shared-settings]") {
    InMemoryFileSystem files;
    REQUIRE(files.writeFile(kUser, "duration-adjust.speed = abc\n").has_value());
    const SharedSettings shared = loadSharedSettings(files, {.userPath = kUser});
    CHECK(announced(shared) ==
          "settings: read /u/subedit/settings.conf\n"
          "settings: duration-adjust.speed: cannot read 'abc', default kept\n");
    CHECK(announced(shared, 0).empty());
    CHECK(settingsRecord(shared).dump() == "\"/u/subedit/settings.conf\"");
}

TEST_CASE("A file not read is announced as such, and recorded as null", "[shared-settings]") {
    const InMemoryFileSystem files;
    const SharedSettings missing = loadSharedSettings(files, {.userPath = kUser});
    CHECK(announced(missing) == "settings: /u/subedit/settings.conf not found, defaults used\n");
    CHECK(settingsRecord(missing).dump() == "null");

    const SharedSettings nowhere = loadSharedSettings(files, {});
    CHECK(announced(nowhere).empty());

    const SharedSettings disabled =
        loadSharedSettings(files, {.disabled = true, .userPath = kUser});
    CHECK(announced(disabled).empty());
    CHECK(settingsRecord(disabled).dump() == "null");

    InMemoryFileSystem locked;
    REQUIRE(locked.writeFile(kUser, "duration-adjust.speed = 12\n").has_value());
    locked.failNextRead(subedit::core::FileErrorKind::PermissionDenied);
    const SharedSettings unreadable = loadSharedSettings(locked, {.userPath = kUser});
    CHECK(unreadable.source == SettingsSource::Unreadable);
    CHECK(unreadable.values == subedit::core::OperationSettings{});
    CHECK(announced(unreadable) ==
          "settings: /u/subedit/settings.conf unreadable, defaults used\n");
    CHECK(settingsRecord(unreadable).dump() == "null");
}
