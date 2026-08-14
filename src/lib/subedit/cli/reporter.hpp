#pragma once

// Telling the user what is happening, at the level of detail they asked for.

#include <iosfwd>
#include <string_view>

namespace subedit::cli {

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

private:
    std::ostream* m_errors;
    int m_level;
};

} // namespace subedit::cli
