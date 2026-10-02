#pragma once

// Telling the user what is happening, at the level of detail they asked for.

#include <iosfwd>
#include <string>
#include <string_view>

namespace subedit::cli {

class Json;

/// The four levels of narration, and the stream they go to.
///
/// **Narration is never the result.** It goes to standard error, so that a
/// caller piping the result of a subcommand receives the result and nothing
/// else — and so that silence can be asked for without losing what the command
/// was invoked to produce.
class Reporter {

public:
    /// `level` is 0 for `--quiet`, 1 by default, 2 for `-vv`, 3 for `-vvv`.
    Reporter(std::ostream& errors, int level) : m_errors{&errors}, m_level{level} {}

    /// Writes `line` when the asked-for level reaches `atLeast`.
    ///
    /// Callers emit their most detailed line first and their least detailed
    /// last, which is what makes each level contain the one below it word for
    /// word rather than merely resemble it.
    void say(int atLeast, std::string_view line) const;

    /// Writes `line` whatever the level, `--quiet` included.
    ///
    /// A command that fails in silence leaves its exit code as the only clue,
    /// and turns every incident into an investigation. "Nothing" means nothing
    /// of what recounts; what raises the alarm stays.
    void failed(std::string_view line) const;

    [[nodiscard]] int level() const { return m_level; }

    /// The same reporter, also writing one record per file to `out`.
    ///
    /// **Records are the result, narration is the account** — the split the
    /// class keeps for a human holds for a script: `out` is standard output, and
    /// what `say` writes stays on standard error whatever the level, unchanged.
    [[nodiscard]] Reporter withRecords(std::ostream& out) const;

    /// The same reporter, also able to write the text of a result to `out`.
    ///
    /// Standard output when the surface is a terminal, and used for the one
    /// thing a text run puts there: what `--dry-run` would change. A reporter
    /// that records writes none of it — the records are the result then.
    [[nodiscard]] Reporter withTextOutput(std::ostream& out) const;

    /// Writes `text` as the result, when the form is text. `text` carries its
    /// own line ends; nothing is added.
    void result(std::string_view text) const;

    /// The same reporter, knowing which subcommand is speaking.
    [[nodiscard]] Reporter forCommand(std::string command) const;

    /// Whether a record is wanted for each file (`--format json`).
    [[nodiscard]] bool recording() const { return m_records != nullptr; }

    /// The name the records are written under.
    [[nodiscard]] const std::string& command() const { return m_command; }

    /// Writes `record` as one line, when records are wanted.
    void record(const Json& record) const;

private:
    std::ostream* m_errors;
    int m_level;
    std::ostream* m_records = nullptr;
    std::ostream* m_text = nullptr;
    std::string m_command;
};

} // namespace subedit::cli
