#include <subedit/core/config/settings.hpp>
#include <subedit/core/io/atomic_write.hpp>
#include <subedit/core/io/file_system.hpp>
#include <subedit/core/model/encoding.hpp>
#include <subedit/core/wording.hpp>

#include <charconv>
#include <concepts>
#include <expected>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <system_error>
#include <type_traits>
#include <vector>

namespace subedit::core {

namespace {

// The keys of the file. **This is a format, and not prose**: they do not go
// through `core/wording.hpp` for the reason `WEBVTT` does not — what a human
// reader reads here are the comments, and those do come from it.
constexpr std::string_view kGeometryKey = "window.geometry";
constexpr std::string_view kMaximisedKey = "window.maximised";
constexpr std::string_view kColumnsKey = "table.columns";
constexpr std::string_view kTableShareKey = "window.table-share";
constexpr std::string_view kDirectoryKey = "file.directory";
constexpr std::string_view kThemeKey = "general.theme";
constexpr std::string_view kInsertPlacementKey = "edit.insert-placement";
constexpr std::string_view kWriteEncodingKey = "file.write-encoding";
constexpr std::string_view kWriteBomKey = "file.write-bom";

// The three values of the theme, as the file carries them. Lower case, and
// kept apart from `nameOf(Theme)`, which gives the labels of the dialog: this
// is a format, that is prose.
constexpr std::string_view kSystemTheme = "system";
constexpr std::string_view kLightTheme = "light";
constexpr std::string_view kDarkTheme = "dark";

// The two sides of an insertion, as the file carries them — and for the reason
// the theme is: these are tokens of a format, not the labels the dialog
// shows.
constexpr std::string_view kAbovePlacement = "above";
constexpr std::string_view kBelowPlacement = "below";

constexpr char kSeparator = '=';
constexpr char kComment = '#';
constexpr char kListSeparator = ',';

[[nodiscard]] std::string_view trimmed(std::string_view text) {
    constexpr std::string_view kBlanks = " \t\r";
    const std::size_t first = text.find_first_not_of(kBlanks);
    if (first == std::string_view::npos)
        return {};
    return text.substr(first, text.find_last_not_of(kBlanks) - first + 1);
}

/// An integer, or nothing. Refuses what trails after the number: "12 pixels"
/// is a value that could not be read, and not a twelve.
[[nodiscard]] std::optional<int> integerOf(std::string_view text) {
    int value = 0;
    // `std::to_address` rather than `.data()`: the view carries its length,
    // and a bare pointer handed over without it is precisely what the static
    // analysis refuses — rightly, even when the end is given right beside
    // it.
    const char* const first = std::to_address(text.begin());
    const char* const last = std::to_address(text.end());
    const std::from_chars_result read = std::from_chars(first, last, value);
    if (read.ec != std::errc{} || read.ptr != last)
        return std::nullopt;
    return value;
}

/// The integers of a comma-separated list, or nothing if one of them is not
/// one.
[[nodiscard]] std::optional<std::vector<int>> integersOf(std::string_view text) {
    std::vector<int> values;
    std::size_t start = 0;

    while (start <= text.size()) {
        const std::size_t next = text.find(kListSeparator, start);
        const std::size_t end = next == std::string_view::npos ? text.size() : next;

        const std::optional<int> one = integerOf(trimmed(text.substr(start, end - start)));
        if (!one.has_value())
            return std::nullopt;
        values.push_back(*one);

        if (next == std::string_view::npos)
            break;
        start = next + 1;
    }

    return values;
}

[[nodiscard]] std::optional<bool> booleanOf(std::string_view text) {
    if (text == "true")
        return true;
    if (text == "false")
        return false;
    return std::nullopt;
}

[[nodiscard]] std::optional<Theme> themeOf(std::string_view text) {
    if (text == kSystemTheme)
        return Theme::System;
    if (text == kLightTheme)
        return Theme::Light;
    if (text == kDarkTheme)
        return Theme::Dark;
    return std::nullopt;
}

[[nodiscard]] std::string_view textOf(Theme theme) {
    switch (theme) {
    case Theme::System:
        return kSystemTheme;
    case Theme::Light:
        return kLightTheme;
    case Theme::Dark:
        return kDarkTheme;
    }
    std::unreachable();
}

[[nodiscard]] std::optional<InsertPlacement> placementOf(std::string_view text) {
    if (text == kAbovePlacement)
        return InsertPlacement::Above;
    if (text == kBelowPlacement)
        return InsertPlacement::Below;
    return std::nullopt;
}

[[nodiscard]] std::string_view textOf(InsertPlacement placement) {
    switch (placement) {
    case InsertPlacement::Above:
        return kAbovePlacement;
    case InsertPlacement::Below:
        return kBelowPlacement;
    }
    std::unreachable();
}

[[nodiscard]] std::optional<int> tableShareOf(std::string_view text) {
    std::optional<int> share = integerOf(text);
    if (share.has_value() && (*share < kSmallestTableShare || *share > kLargestTableShare))
        share.reset();
    return share;
}

/// A directory, if it is absolute.
///
/// A relative path is relative to a working directory nobody knows: it is a
/// value that could not be read, and not a path to complete on a hunch.
[[nodiscard]] std::optional<std::filesystem::path> directoryOf(std::string_view text) {
    std::filesystem::path directory{text};
    if (directory.empty() || !directory.is_absolute())
        return std::nullopt;
    return directory;
}

[[nodiscard]] std::optional<WindowGeometry> geometryOf(std::string_view text) {
    const std::optional<std::vector<int>> numbers = integersOf(text);
    // Four numbers, and dimensions that are not zero: a window zero wide is a
    // window one would never find again.
    if (!numbers.has_value() || numbers->size() != 4 || (*numbers)[2] <= 0 || (*numbers)[3] <= 0)
        return std::nullopt;

    return WindowGeometry{
        .x = (*numbers)[0], .y = (*numbers)[1], .width = (*numbers)[2], .height = (*numbers)[3]};
}

[[nodiscard]] std::optional<std::vector<int>> columnsOf(std::string_view text) {
    std::optional<std::vector<int>> widths = integersOf(text);
    if (!widths.has_value() || widths->size() != kColumnWidthCount)
        return std::nullopt;

    for (const int width : *widths) {
        if (width <= 0)
            return std::nullopt;
    }
    return widths;
}

[[nodiscard]] std::string joined(const std::vector<int>& numbers) {
    std::string text;
    for (std::size_t index = 0; index < numbers.size(); ++index) {
        if (index > 0)
            text += kListSeparator;
        text += std::to_string(numbers[index]);
    }
    return text;
}

/// Keeps an option, or reports the value that could not be read.
///
/// **An unknown key is ignored, without a word**: a file written by a version
/// that knew more of them is not a faulty file, and that is the failure mode
/// tolerance chooses.
void applyOption(SettingsRead& read,
                 bool& wantsByteOrderMark,
                 std::string_view key,
                 std::string_view value) {
    // What all eight options do the same way: lay down what could be read,
    // name the option otherwise. Written once rather than eight times, and it
    // is not only a saving of lines — the seventh option took this function
    // over the complexity threshold the gate holds.
    const auto take = [&read, key, value](auto parsed, auto& field) {
        if (!parsed.has_value()) {
            read.diagnostics.push_back({.key = std::string{key}, .value = std::string{value}});
            return;
        }

        // **A setting that is itself optional receives the option as it is**,
        // and is not unwrapped to be wrapped again: it is the same content, and
        // one dereference fewer.
        if constexpr (std::same_as<std::remove_cvref_t<decltype(field)>, decltype(parsed)>)
            field = std::move(parsed);
        else
            field = *std::move(parsed);
    };

    if (key == kGeometryKey)
        take(geometryOf(value), read.settings.geometry);
    else if (key == kMaximisedKey)
        take(booleanOf(value), read.settings.maximised);
    else if (key == kColumnsKey)
        take(columnsOf(value), read.settings.columnWidths);
    else if (key == kTableShareKey)
        take(tableShareOf(value), read.settings.tableShare);
    else if (key == kDirectoryKey)
        take(directoryOf(value), read.settings.lastDirectory);
    else if (key == kThemeKey)
        take(themeOf(value), read.settings.theme);
    else if (key == kInsertPlacementKey)
        take(placementOf(value), read.settings.insertPlacement);
    else if (key == kWriteEncodingKey)
        // **The name alone, the mark at the next key**, as the command line
        // keeps `--encoding` apart from `--bom`: `-sig` stopped being a name in
        // #315, and gluing it back on here would make it one again. The mark is
        // put on the encoding once the file is read — see `readSettings`.
        //
        // A name `create` refuses is an unreadable setting, reported like the
        // others: here, an encoding ICU does not know and one that would write
        // its own mark are the same news — a setting nothing can be done with —
        // and `take` reads either the same way, the refusal of an `expected`
        // being asked after like the absence of an `optional`.
        take(Encoding::create(value, ByteOrderMark::Absent), read.settings.writeEncoding);
    else if (key == kWriteBomKey)
        take(booleanOf(value), wantsByteOrderMark);
}

/// An option, written bare when set, commented out when at its default.
void writeOption(std::string& out, std::string_view key, std::string_view value, bool atDefault) {
    if (atDefault)
        out += kComment;
    out += key;
    out += " = ";
    out += value;
    out += '\n';
}

} // namespace

SettingsRead readSettings(const FileSystem& files, const std::filesystem::path& path) {
    SettingsRead read;

    // What `file.write-bom` said, until `file.write-encoding` has spoken too.
    bool wantsByteOrderMark = false;

    // **Absent and unreadable both give the defaults, and are not said the
    // same way.** A file that does not exist is the first launch, and there is
    // nothing to report; a file that exists and refuses itself is a setting
    // lost, which the user has a right to know is lost. Neither is a reason not
    // to start.
    std::expected<std::string, FileError> content = files.readFile(path);
    if (!content) {
        if (content.error().kind != FileErrorKind::NotFound)
            read.unreadable = std::move(content.error());
        return read;
    }

    std::size_t start = 0;
    while (start < content->size()) {
        const std::size_t end = content->find('\n', start);
        const std::string_view line = trimmed(std::string_view{*content}.substr(
            start, (end == std::string::npos ? content->size() : end) - start));
        start = (end == std::string::npos) ? content->size() : end + 1;

        if (line.empty() || line.front() == kComment)
            continue;

        const std::size_t separator = line.find(kSeparator);
        if (separator == std::string_view::npos)
            continue;

        const std::string_view key = trimmed(line.substr(0, separator));
        const std::string_view value = trimmed(line.substr(separator + 1));

        applyOption(read, wantsByteOrderMark, key, value);
    }

    // **The mark goes on at the end, because the order of the file is nobody's
    // to command.** Two keys carry one value, and nothing says which of the two
    // lines comes first — a file edited by hand puts them in whatever order it
    // likes. Applying the mark as it is read would need the encoding to be read
    // already; applying it here needs nothing.
    //
    // A mark with no encoding says nothing and puts nothing anywhere: it is the
    // encoding that carries a mark, and there is none.
    if (wantsByteOrderMark && read.settings.writeEncoding.has_value())
        read.settings.writeEncoding =
            read.settings.writeEncoding->withByteOrderMark(ByteOrderMark::Present);

    return read;
}

std::string renderSettings(const Settings& settings) {
    std::string out{settingsFileHeader()};
    out += '\n';

    const WindowGeometry geometry = settings.geometry.value_or(WindowGeometry{});
    writeOption(out,
                kGeometryKey,
                settings.geometry.has_value()
                    ? joined({geometry.x, geometry.y, geometry.width, geometry.height})
                    : "0,0,1280,800",
                !settings.geometry.has_value());

    writeOption(out, kMaximisedKey, settings.maximised ? "true" : "false", !settings.maximised);

    writeOption(out,
                kColumnsKey,
                settings.columnWidths.empty() ? "60,110,110,110" : joined(settings.columnWidths),
                settings.columnWidths.empty());

    writeOption(out,
                kTableShareKey,
                settings.tableShare.has_value() ? std::to_string(*settings.tableShare) : "70",
                !settings.tableShare.has_value());

    writeOption(out,
                kDirectoryKey,
                settings.lastDirectory.has_value() ? settings.lastDirectory->string()
                                                   : "/home/vous/films",
                !settings.lastDirectory.has_value());

    writeOption(
        out, kThemeKey, std::string{textOf(settings.theme)}, settings.theme == Theme::System);

    writeOption(out,
                kInsertPlacementKey,
                std::string{textOf(settings.insertPlacement)},
                settings.insertPlacement == InsertPlacement::Below);

    writeOption(out,
                kWriteEncodingKey,
                settings.writeEncoding.has_value() ? std::string{settings.writeEncoding->charset()}
                                                   : "UTF-8",
                !settings.writeEncoding.has_value());

    // **Written even with no encoding remembered**, as the seven others are: an
    // option at its default is written commented out, which is what makes the
    // file readable without a manual beside it.
    const bool marked = settings.writeEncoding.has_value() &&
                        settings.writeEncoding->byteOrderMark() == ByteOrderMark::Present;
    writeOption(out, kWriteBomKey, marked ? "true" : "false", !marked);

    return out;
}

std::expected<void, FileError>
writeSettings(FileSystem& files, const std::filesystem::path& path, const Settings& settings) {
    // The directory first: at the first launch, nobody has made it.
    if (const std::expected<void, FileError> made = files.createDirectories(path.parent_path());
        !made)
        return made;

    return writeAtomically(files, path, renderSettings(settings));
}

} // namespace subedit::core
