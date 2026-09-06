#pragma once

#include <subedit/core/config/insert_placement.hpp>
#include <subedit/core/config/theme.hpp>
#include <subedit/core/io/file_system.hpp>
#include <subedit/core/model/encoding.hpp>

#include <cstddef>
#include <expected>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace subedit::core {

/// Where a window was, in the coordinates the window system uses.
struct WindowGeometry {
    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0;

    friend bool operator==(const WindowGeometry&, const WindowGeometry&) = default;
};

/// The bounds of the share given to the table, in per cent.
///
/// Neither zero nor a hundred: a table of no height, or a video band of no
/// height, is a window one could no longer reopen otherwise than by deleting
/// its configuration file.
inline constexpr int kSmallestTableShare = 1;
inline constexpr int kLargestTableShare = 99;

/// How many column widths a settings file gives.
///
/// **Four for five columns, and it is no oversight.** The last — the text —
/// takes what the others leave: it is the column that varies, and the first
/// four are known widths. Keeping the fifth would be keeping a number the table
/// works out afresh at every resize, so writing into the file a value the
/// reading could not honour.
///
/// A file that names three or five is a file whose widths mean nothing: the
/// reader keeps its default and says so, rather than guessing which column was
/// meant.
inline constexpr std::size_t kColumnWidthCount = 4;

/// What the window remembers from one session to the next.
///
/// **The values, their defaults, and nothing else** — no location, no
/// singleton. ADR 0022: the settings receive a path rather than looking one up,
/// which is what makes them provable without writing anywhere.
///
/// What is in it is what decision D5 of the phase-7 spec lets in: a preference
/// is persisted if forgetting it is a nuisance, and if remembering it is
/// harmless. The last file opened is the one that fails the second half —
/// remembering it means reopening at launch a document nobody asked for.
struct Settings {
    /// Absent for a window that has never been placed, which is the default:
    /// Qt then sizes it itself, wide enough to read the table.
    std::optional<WindowGeometry> geometry{};

    bool maximised = false;

    /// Empty, or exactly `kColumnWidthCount` widths in pixels. Empty means
    /// « as the table sizes itself », which is the default.
    std::vector<int> columnWidths{};

    /// The share of the window's height the table takes, in per cent.
    ///
    /// **A proportion and not heights**, and that is the point to make: three
    /// absolute heights that do not add up to the window mean nothing as soon
    /// as it opens at another size, and a window often opens at another size. A
    /// share replays anywhere.
    ///
    /// Absent by default: the window then shares its height as it always
    /// has.
    std::optional<int> tableShare{};

    /// Where the "open" box opens, absolute or absent.
    ///
    /// **Keeping a directory is not keeping a file**, and that is the cut of
    /// criterion D5: a directory *points* a dialog box, a file would reopen at
    /// launch a document nobody asked for.
    ///
    /// Absolute, or nothing: a relative path in a configuration file is
    /// relative to a working directory nobody knows.
    std::optional<std::filesystem::path> lastDirectory{};

    Theme theme = Theme::System;

    /// Which side of the selection `Insert Subtitles…` lays its rows on.
    ///
    /// **Below it by default**, which is Gaupol's default. It is the direction
    /// a subtitle file is written in: the next row comes after the one just
    /// looked at.
    InsertPlacement insertPlacement = InsertPlacement::Below;

    /// The encoding last chosen in `Save As…`, absent at the first launch.
    ///
    /// **It never wins over the encoding of a file that was read**, and that is
    /// the cut: an opened document carries its own, and rewriting it in another
    /// because a setting three weeks old says so would be a loss nobody asked
    /// for — the byte round trip of phase 8 is exactly that promise. What it
    /// remembers serves the document with no file: the one just created.
    ///
    /// **The mark is part of it, and the file carries it at one more key.**
    /// `file.write-encoding` names the encoding, `file.write-bom` says whether
    /// its mark precedes it — the same cut the command line makes between
    /// `--encoding` and `--bom`, and the only one left since `-sig` stopped
    /// being a name (#315). One value here, two lines there: the type carries
    /// the mark as a variant of the encoding, and only the writing takes them
    /// apart.
    std::optional<Encoding> writeEncoding{};

    friend bool operator==(const Settings&, const Settings&) = default;
};

/// An option whose value could not be read.
///
/// **The diagnostic accompanies the default rather than replacing it** — ADR
/// 0022. What it carries is what a message needs and nothing more: which
/// option, and what was written there.
struct SettingsDiagnostic {
    std::string key;
    std::string value;

    friend bool operator==(const SettingsDiagnostic&, const SettingsDiagnostic&) = default;
};

/// What reading a settings file gave.
struct SettingsRead {
    Settings settings{};

    /// In the order the options appear in the file.
    std::vector<SettingsDiagnostic> diagnostics{};

    /// Set when the file is there and could not be read at all.
    ///
    /// **Apart from the diagnostics, and not one of them**, because it is not
    /// the same statement: a value nobody can read names an option, this names
    /// none — every setting is at its default. An absent file leaves it empty,
    /// since a first launch is not a failure.
    std::optional<FileError> unreadable{};
};

/// Reads the settings written at `path`, and never fails.
///
/// **There is nothing to fail with, and that is a decision** — ADR 0022. A
/// configuration is a comfort; its failure must cost the comfort and nothing
/// else. So an absent file is every default, an unreadable file is every
/// default, an unknown key is ignored, and a value nobody can read leaves its
/// default in place and produces a diagnostic.
///
/// The phase-7 spec sketched an `std::expected<Settings, ConfigError>`; the ADR
/// settled tolerance afterwards, and tolerance leaves no error to return.
[[nodiscard]] SettingsRead readSettings(const FileSystem& files, const std::filesystem::path& path);

/// Writes `settings` to `path`, making the directories above it if needed.
///
/// **Every known option is written, and those left at their default are
/// written commented out.** That is the detail of Gaupol worth keeping: without
/// it, an option the user never touched would be frozen at the value of the day
/// it was written, and a default one improves would never reach anybody. With
/// it, changing a default changes the behaviour of everyone who has not
/// overridden it — which is exactly what a default should do.
///
/// Unknown keys that were in the file are not written back: they were ignored
/// on the way in, and keeping them would be keeping something nothing reads.
[[nodiscard]] std::expected<void, FileError>
writeSettings(FileSystem& files, const std::filesystem::path& path, const Settings& settings);

/// The text `writeSettings` would write. Exposed for the tests that read it.
[[nodiscard]] std::string renderSettings(const Settings& settings);

} // namespace subedit::core
