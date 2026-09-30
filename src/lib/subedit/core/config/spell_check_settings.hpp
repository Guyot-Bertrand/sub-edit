#pragma once

// What `Tools > Check Spelling…` retains from one session to the next — issue
// #509, the three fields Gaupol keeps as `language`, `target` and `field`.

#include <string>

namespace subedit::core {

/// Which subtitles the spell check walks.
enum class SpellCheckTarget {
    Selection,
    CurrentProject,
    AllProjects,
};

/// Which of a subtitle's two texts is checked.
enum class SpellCheckDocument {
    Main,
    Translation,
};

struct SpellCheckSettings {
    /// The locale code of the dictionary; empty until one was chosen, the
    /// window then takes the system's.
    std::string language{};
    SpellCheckTarget target = SpellCheckTarget::CurrentProject;
    SpellCheckDocument document = SpellCheckDocument::Main;

    friend bool operator==(const SpellCheckSettings&, const SpellCheckSettings&) = default;
};

} // namespace subedit::core
