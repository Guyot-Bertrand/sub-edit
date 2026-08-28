// Where the settings live, and what keeps a test away from them.
//
// **Two things are proved here, and the second is the reason the file
// exists.** The first is the shape of the production path — the one piece of
// code that resolves a standard location, and therefore the one that no other
// test can reach through. The second is what happens when the harness is not
// there: the very same call answers the developer's own `~/.config`.
//
// A defect one cannot see is a defect one stops believing in. The harness is
// removed for the length of one case, the answer is read, and it is put back —
// and nothing here writes anything anywhere, which is the whole point.

#include <subedit/gui/settings_path.hpp>

#include <QByteArray>
#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <string>

namespace {

/// The user's real configuration home, derived the way the standard does.
[[nodiscard]] std::filesystem::path homeDirectory() {
    return std::filesystem::path{qgetenv("HOME").toStdString()};
}

/// Takes `XDG_CONFIG_HOME` away for as long as it lives, and puts it back.
///
/// The harness of this binary is one environment variable, set by `main()`
/// before anything else runs. Removing it is how a case shows what the code
/// does without it — and restoring it in a destructor rather than at the end of
/// the case is what keeps a failed assertion from leaving the process
/// unharnessed for every case that follows.
class WithoutTheHarness {
public:
    WithoutTheHarness() : m_held(qgetenv("XDG_CONFIG_HOME")) { qunsetenv("XDG_CONFIG_HOME"); }

    WithoutTheHarness(const WithoutTheHarness&) = delete;
    WithoutTheHarness& operator=(const WithoutTheHarness&) = delete;
    WithoutTheHarness(WithoutTheHarness&&) = delete;
    WithoutTheHarness& operator=(WithoutTheHarness&&) = delete;

    ~WithoutTheHarness() {
        // An empty value is a variable that exists and says nothing, which is
        // not the same as one that does not exist. Only one of the two is what
        // was there before.
        if (m_held.isEmpty())
            qunsetenv("XDG_CONFIG_HOME");
        else
            qputenv("XDG_CONFIG_HOME", m_held);
    }

private:
    QByteArray m_held;
};

} // namespace

TEST_CASE("the settings have one path, and it is spelled out", "[gui]") {
    const std::filesystem::path settings = subedit::gui::userSettingsPath();

    CHECK(settings.is_absolute());
    CHECK(settings.filename() == "settings.conf");
    // Under a directory of ours, so that the file is ours to name and the
    // config home stays the user's to fill.
    CHECK(settings.parent_path().filename() == "subedit");
}

// The demonstration asked for by #238: absent the harness, a test that wrote
// what this call returns would write into the settings of whoever ran it.
TEST_CASE("without the harness, the settings path is the user's own", "[gui]") {
    REQUIRE_FALSE(homeDirectory().empty());
    const std::filesystem::path real = homeDirectory() / ".config" / "subedit" / "settings.conf";

    CHECK(subedit::gui::userSettingsPath() != real);
    // Not merely elsewhere under the home directory: nowhere near it.
    CHECK_FALSE(subedit::gui::userSettingsPath().string().starts_with(homeDirectory().string()));

    {
        const WithoutTheHarness demonstration;

        CHECK(subedit::gui::userSettingsPath() == real);
    }

    CHECK(subedit::gui::userSettingsPath() != real);
}
