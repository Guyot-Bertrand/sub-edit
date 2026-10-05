#pragma once

// Running the command-line tool for real.
//
// A unit test links the library and calls into it. These tests do neither:
// they start the binary a user would start, and read what a user would see.
// Nothing here knows what subedit-cli does — only how to run it.

#include <catch2/matchers/catch_matchers_templated.hpp>

#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
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

/// Runs the probe that resolves the pattern directories as the command line will
/// and reads the catalogue (`tools/pattern_catalogue.cpp`), under the same
/// harness as the two binaries: its own configuration, data and Enchant homes.
[[nodiscard]] CliRun invokePatternCatalogue();

/// A file of patterns dropped in the user's patterns directory — the one the
/// harness moved — and taken away again when this goes out of scope.
///
/// **What a test drops must not outlive it**, or the next case would read it:
/// the directory is the process's own, shared by every case. The file is named
/// as the command line reads it, `<code>.<type>` (`Latn-xb.common-error`).
class UserPatterns {
public:
    UserPatterns(const std::string& name, const std::string& text);

    UserPatterns(const UserPatterns&) = delete;
    UserPatterns& operator=(const UserPatterns&) = delete;
    UserPatterns(UserPatterns&&) = delete;
    UserPatterns& operator=(UserPatterns&&) = delete;

    ~UserPatterns();
};

/// The data home and the Enchant home every launched binary is given: where the
/// patterns a user drops in `$XDG_DATA_HOME/subedit/patterns` would be, and where
/// Enchant keeps its personal word list. **Empty, and the test's own** — the
/// command line reads both, and a run that read the developer's would pass for
/// the one who has no patterns and fail for the one who has. Created as
/// `configHome()` is, and removed with it.
[[nodiscard]] std::string dataHome();
[[nodiscard]] std::string enchantHome();

/// The environment a launched binary receives: this process's own, with the
/// configuration home, the data home and the Enchant home replaced by the above.
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

/// Where two texts first part ways, worded for a person, or nothing if they are
/// the same bytes.
///
/// Texts are cut into lines **keeping their terminators**, and the first pair
/// that is not byte-for-byte equal is reported with its 1-based number, the
/// expected line and the actual one. Because the terminator belongs to the line,
/// "a\r\n" against "a\n" is a difference on line 1, and a text that is the
/// prefix of the other differs at the first line one of them lacks. Control
/// characters, line ends and the UTF-8 byte order mark are printed as escapes
/// (`\r`, `\n`, `\xEF\xBB\xBF`), so that what differs can be seen.
[[nodiscard]] std::optional<std::string> firstDifference(std::string_view actual,
                                                         std::string_view expected);

/// Catch2 matcher: the string is byte-for-byte the content of a file.
///
/// ```
/// CHECK_THAT(contentOf(out), MatchesFile(corpus("attendus/mentions.hearing-impaired.srt")));
/// ```
///
/// On failure Catch prints `firstDifference`, not two walls of text.
///
/// **An expected file is never produced by the program under test.** That is
/// the lesson of #338: a file the tool wrote and a test then reads back proves
/// that the tool agrees with itself, and stays green through every regression
/// it introduces. An expected file is written by hand from what the input and
/// the rule say, or observed on another tool, and says which in a comment or in
/// the README of its directory. Looking at the real output to *check* one's own
/// expectation is fine; copying it in is not.
class MatchesFile final : public Catch::Matchers::MatcherGenericBase {
public:
    explicit MatchesFile(std::filesystem::path expected);

    [[nodiscard]] bool match(const std::string& actual) const;

    [[nodiscard]] std::string describe() const override;

private:
    std::filesystem::path m_expected;
    mutable std::string m_difference;
};

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

/// The text of a valid SubRip file of `cues` cues, at most 9, built by hand.
///
/// Cue `i` (1-based) runs from `i` s to `i` s + 500 ms and says `"<tag> <i>"`,
/// so that a file written by this helper is told apart from another by its
/// content, and a test can write the expected result of an operation as a
/// literal rather than reading it back from the program. Line ends are `\n`.
[[nodiscard]] std::string srtText(const std::string& tag, int cues);

/// Writes a valid SubRip file of `cues` cues (see `srtText`) at `name` inside
/// the scratch directory, creating the directories on the way, and returns its
/// full path. The tag is `name` itself.
///
/// Two names with the same base name in two directories (`"a/film.srt"`,
/// `"b/film.srt"`) give two different files of the same name, which is what an
/// `--output-dir` collision needs.
std::string writeSrt(const Scratch& scratch, const std::string& name, int cues = 2);

/// Writes `count` valid files `<directory>/file-1.srt` … `file-<count>.srt`
/// and returns their paths in order.
std::vector<std::string>
writeSrtBatch(const Scratch& scratch, int count, const std::string& directory = "in");

/// Copies a file that no format claims (`malformes/vide.srt`, versioned) to
/// `name` inside the scratch directory and returns its full path. Put it in the
/// middle of a list of valid ones to test a batch that fails halfway.
std::string writeUnreadable(const Scratch& scratch, const std::string& name);

/// Writes `text` verbatim at `name` inside the scratch directory, creating the
/// directories on the way — for a destination that exists before the run.
std::string writeFile(const Scratch& scratch, const std::string& name, const std::string& text);

} // namespace subedit::e2e
