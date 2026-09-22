#pragma once

#include <subedit/core/command/command.hpp>
#include <subedit/core/format/degradation.hpp>

#include <cstddef>
#include <memory>

namespace subedit::core {

class Project;

/// What appending a file planned, and what it will cost — decision D6 of the
/// phase-11 spec.
struct AppendedFile {
    /// One command, one entry in the history. Nothing when `appended` held no
    /// subtitle — there is then nothing to undo.
    std::unique_ptr<Command> command{};

    /// How many subtitles were appended.
    std::size_t inserted = 0;

    /// What crossing into the project's own format cost — ADR 0031, counted by
    /// `convertFor`. Of the main text only: an appended file has no
    /// translation of its own.
    ConversionLoss loss{};
};

/// Plans appending `appended` to the end of `project`.
///
/// **Shifted from the end of the last subtitle**, as Gaupol does: nothing is
/// inserted between the two, and `appended`'s own positions are shifted by that
/// amount, keeping their order and their spacing. An empty `project` shifts by
/// nothing — there is no end to shift from, and the appended file lands as it
/// stands.
///
/// **`appended` crosses into the format of `project`**, through
/// `convertProjectFor`: its tags are translated into `project`'s vocabulary and
/// what will not fit is counted, the rule and the words of pasting texts from
/// another format (`GUI-CLIP-02`). Positions are counted at `project`'s own
/// frame rate, the one they will be read against from here on.
///
/// **The appended subtitles carry no translation**, whatever `appended` itself
/// held: a file opened on its own has none to give, and D6 does not ask for one.
[[nodiscard]] AppendedFile appendFile(const Project& project, const Project& appended);

} // namespace subedit::core
