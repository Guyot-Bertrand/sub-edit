#pragma once

// Running the command-line tool for real.
//
// A unit test links the library and calls into it. These tests do neither:
// they start the binary a user would start, and read what a user would see.
// Nothing here knows what subedit-cli does — only how to run it.

#include <filesystem>
#include <string>
#include <vector>

namespace subedit::e2e {

/// What one run of subedit-cli produced.
struct CliRun {
    /// The process exit code.
    ///
    /// A process killed by a signal reports `128 + signal` rather than the
    /// exit status it never returned. Without that, a crash would be
    /// indistinguishable from a clean zero, which is the one value most
    /// assertions expect.
    int exitCode = 0;

    /// Standard output, verbatim, newlines included.
    std::string output;

    /// Standard error, kept apart from the above.
    std::string errors;
};

/// Runs subedit-cli with `args` and waits for it to finish.
///
/// No shell is involved: arguments reach the program exactly as written, with
/// no splitting, globbing or quoting to undo. The binary path comes from the
/// build (`SUBEDIT_CLI_BINARY`), never from the current directory.
///
/// Throws `std::system_error` when the process cannot be started or its output
/// cannot be read. A test that cannot run the binary has nothing to assert.
CliRun invoke(const std::vector<std::string>& args);

/// The same, for the window binary.
///
/// **One runner for both, and not two.** Everything above — the pipes, the
/// signal turned into an exit code, the absence of a shell — has nothing to do
/// with which binary is started; only the path differs. #80 removed the same
/// duplication once already, in this very harness.
///
/// It runs with no screen: CTest gives the binary `QT_QPA_PLATFORM=offscreen`,
/// without which a `QApplication` aborts on « could not connect to display ».
CliRun invokeGui(const std::vector<std::string>& args);

/// The configuration home every launched binary is given, and none other.
///
/// **A binary launched for a test must not touch the settings of whoever runs
/// it.** Two things would go wrong at once, and the second is the worse: the
/// run destroys the developer's preferences, which is at least visible; and it
/// starts depending on what that developer already had, so that a test passing
/// for someone who never started the program fails for someone who started it
/// once.
///
/// `Scratch` covers what a test writes inside the repository, and
/// `check-untracked.sh` catches what it leaves there. Neither can see a file
/// written into a home directory. This is what covers that — see
/// `check-config-home.sh` for the control that watches the real location.
///
/// The directory is created empty, named after the running process so that two
/// test binaries never share one, and removed when the process ends.
[[nodiscard]] std::string configHome();

/// The environment a launched binary receives: this process's own, with the
/// configuration home replaced by the above.
///
/// Exported so that the substitution can be asserted on rather than trusted —
/// it happens in the one place a binary is started, and a harness nobody
/// checks is a harness that stops working quietly.
[[nodiscard]] std::vector<std::string> childEnvironment();

/// A path into the test corpus, resolved by the build.
///
/// `SUBEDIT_TEST_DATA_DIR` comes from CMake and never from the current
/// directory: a test must not depend on where it was launched from.
[[nodiscard]] std::string corpus(const std::string& relative);

/// The bytes of a file, verbatim, or nothing if it cannot be opened.
///
/// Opened in binary: these tests compare line endings and byte order marks,
/// which a text-mode read would be free to touch.
[[nodiscard]] std::string contentOf(const std::filesystem::path& path);

/// A directory of its own, removed with everything in it.
///
/// One per test that writes, created empty and gone when the test ends —
/// including when it ends on a failed assertion, which is the case that used
/// to leave files behind. Four of these tests wrote into one shared directory
/// and removed their files by hand, path by path, on the paths where they
/// succeeded.
class Scratch {

public:
    Scratch();

    Scratch(const Scratch&) = delete;
    Scratch& operator=(const Scratch&) = delete;
    Scratch(Scratch&&) = delete;
    Scratch& operator=(Scratch&&) = delete;

    /// Removes the directory and everything under it.
    ///
    /// **Never throws**, unlike the plain `remove_all`: a destructor that does
    /// so while an assertion is already unwinding the stack ends the process
    /// instead of reporting the assertion.
    ~Scratch();

    /// The path of a file inside, which need not exist.
    [[nodiscard]] std::string of(const std::string& name) const;

    /// The directory itself, for `--output-dir`.
    [[nodiscard]] std::string path() const;

private:
    std::filesystem::path m_path;
};

} // namespace subedit::e2e
