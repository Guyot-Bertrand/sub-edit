#pragma once

// How the tool names its settings to the user: the themes and insertion sides
// the dialogs offer, and the sentences about the settings file. Shared wording
// of `core/wording/`; see `formats.hpp` for why it lives in the core.

#include <subedit/core/config/insert_placement.hpp>
#include <subedit/core/config/theme.hpp>

#include <string_view>

namespace subedit::core {

/// A theme, as the preferences dialog offers it.
///
/// Separate from the token the settings file carries — one is prose a reader
/// picks from a list, the other is a format. Merging them would tie the words
/// of an interface to the shape of a file, and the day the interface is
/// translated the file would follow.
[[nodiscard]] std::string_view nameOf(Theme theme);

/// A side of the selection, as the insertion dialog offers it.
///
/// Separate from the token the settings file carries, for the reason
/// `nameOf(Theme)` is: one is prose a reader picks from a pair of buttons, the
/// other is a format.
[[nodiscard]] std::string_view nameOf(InsertPlacement placement);

/// What « system » does, in the second half of a sentence about the theme.
///
/// It does nothing, and the manual has to say so rather than let a reader
/// wonder: Qt 6.4 has no colour-scheme API, so the palette stays the
/// platform's.
[[nodiscard]] std::string_view systemThemeExplained();

/// The lines a settings file opens with.
///
/// **Here rather than in the writer** because they are prose meant for whoever
/// opens the file in an editor, and the project keeps every such word in one
/// place. The key names are not: they are the format, like `WEBVTT` is, and a
/// format is not wording.
///
/// Ends with a newline, and each line is already a comment.
[[nodiscard]] std::string_view settingsFileHeader();

/// What is wrong with an option, in the second half of a sentence starting with
/// the file and the option's name.
///
/// One sentence for the only thing that can go wrong with a value the reader
/// keeps a default for: it could not be read. Which value it was, and what the
/// default is, belong to the caller — it has both, and this has neither.
[[nodiscard]] std::string_view unreadableSetting();

/// What is said when a settings file could not be written.
///
/// Its own sentence rather than `reasonOf(FileErrorKind)`, whose words are
/// those of a reading: « cannot be read » is a strange thing to say about a
/// save. Nothing more precise is offered on purpose — a settings file that
/// will not be written costs a comfort, and the program carries on.
[[nodiscard]] std::string_view settingsNotWritten();

} // namespace subedit::core
