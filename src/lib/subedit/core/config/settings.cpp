#include <subedit/core/config/settings.hpp>
#include <subedit/core/io/atomic_write.hpp>
#include <subedit/core/io/file_system.hpp>
#include <subedit/core/model/encoding.hpp>
#include <subedit/core/wording.hpp>

#include <algorithm>
#include <array>
#include <charconv>
#include <cmath>
#include <concepts>
#include <expected>
#include <format>
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
constexpr std::string_view kColumnOrderKey = "table.order";
constexpr std::string_view kHiddenColumnsKey = "table.hidden";
constexpr std::string_view kTableShareKey = "window.table-share";
constexpr std::string_view kDirectoryKey = "file.directory";
constexpr std::string_view kThemeKey = "general.theme";
constexpr std::string_view kInsertPlacementKey = "edit.insert-placement";
constexpr std::string_view kWriteEncodingKey = "file.write-encoding";
constexpr std::string_view kWriteBomKey = "file.write-bom";
constexpr std::string_view kSearchRegexKey = "search.regex";
constexpr std::string_view kSearchIgnoreCaseKey = "search.ignore-case";

// The form of `Adjust Durations…`, nine keys under one prefix: the prefix is
// what `applyOption` dispatches on.
constexpr std::string_view kDurationPrefix = "duration-adjust.";
constexpr std::string_view kDurationSpeedKey = "duration-adjust.speed";
constexpr std::string_view kDurationLengthenKey = "duration-adjust.lengthen";
constexpr std::string_view kDurationShortenKey = "duration-adjust.shorten";
constexpr std::string_view kDurationMinimumEnabledKey = "duration-adjust.minimum-enabled";
constexpr std::string_view kDurationMinimumKey = "duration-adjust.minimum-ms";
constexpr std::string_view kDurationMaximumEnabledKey = "duration-adjust.maximum-enabled";
constexpr std::string_view kDurationMaximumKey = "duration-adjust.maximum-ms";
constexpr std::string_view kDurationGapEnabledKey = "duration-adjust.gap-enabled";
constexpr std::string_view kDurationGapKey = "duration-adjust.gap-ms";

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

/// A decimal number, or nothing. Refuses what trails after it, like
/// `integerOf`, and refuses what is not a number a person would write: `inf`
/// and `nan` are read by `from_chars`, and are not values a file should carry.
[[nodiscard]] std::optional<double> decimalOf(std::string_view text) {
    double value = 0.0;
    const char* const first = std::to_address(text.begin());
    const char* const last = std::to_address(text.end());
    const std::from_chars_result read = std::from_chars(first, last, value);
    if (read.ec != std::errc{} || read.ptr != last || !std::isfinite(value))
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

/// A reading speed, which is strictly positive.
///
/// The same refusal `ReadingSpeed::create` makes, so that a file cannot hold
/// what the form could not show as a speed.
[[nodiscard]] std::optional<double> speedOf(std::string_view text) {
    std::optional<double> speed = decimalOf(text);
    if (speed.has_value() && *speed <= 0.0)
        speed.reset();
    return speed;
}

/// A duration in milliseconds, which is not negative.
///
/// Zero is a duration: a minimum or a gap of zero is asked for, not absent.
/// `integerOf` reads an `int`, which holds far more than the ninety-nine
/// seconds the dialog can show.
[[nodiscard]] std::optional<int> millisecondsOf(std::string_view text) {
    std::optional<int> milliseconds = integerOf(text);
    if (milliseconds.has_value() && *milliseconds < 0)
        milliseconds.reset();
    return milliseconds;
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

/// The six columns as the file names them — tokens of a format, like the
/// theme's, and not the labels of the table's header.
constexpr std::array<std::string_view, kTableColumnCount> kColumnNames = {
    "number", "start", "end", "duration", "text", "translation"};

[[nodiscard]] std::string_view textOf(TableColumn column) {
    return kColumnNames.at(static_cast<std::size_t>(column));
}

/// The columns of a comma-separated list of names, each at most once, or
/// nothing if one is not a column or comes twice. An empty text is an empty
/// list.
[[nodiscard]] std::optional<std::vector<TableColumn>> columnNamesOf(std::string_view text) {
    std::vector<TableColumn> columns;
    if (text.empty())
        return columns;

    std::size_t start = 0;
    while (start <= text.size()) {
        const std::size_t next = text.find(kListSeparator, start);
        const std::size_t end = next == std::string_view::npos ? text.size() : next;
        const std::string_view name = trimmed(text.substr(start, end - start));

        const auto* found = std::ranges::find(kColumnNames, name);
        if (found == kColumnNames.end())
            return std::nullopt;
        const auto column = static_cast<TableColumn>(found - kColumnNames.begin());
        if (std::ranges::find(columns, column) != columns.end())
            return std::nullopt;
        columns.push_back(column);

        if (next == std::string_view::npos)
            break;
        start = next + 1;
    }
    return columns;
}

/// An order: every column, once each.
[[nodiscard]] std::optional<std::vector<TableColumn>> columnOrderOf(std::string_view text) {
    std::optional<std::vector<TableColumn>> order = columnNamesOf(text);
    if (!order.has_value() || order->size() != kTableColumnCount)
        return std::nullopt;
    return order;
}

/// Hidden columns: only those that may be.
[[nodiscard]] std::optional<std::vector<TableColumn>> hiddenColumnsOf(std::string_view text) {
    std::optional<std::vector<TableColumn>> hidden = columnNamesOf(text);
    if (!hidden.has_value() || !std::ranges::all_of(*hidden, isHideable))
        return std::nullopt;
    return hidden;
}

[[nodiscard]] std::string namesOf(const std::vector<TableColumn>& columns) {
    std::string text;
    for (const TableColumn column : columns) {
        if (!text.empty())
            text += kListSeparator;
        text += textOf(column);
    }
    return text;
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

/// Lays down an option that could be read, and names it otherwise.
///
/// What all the options do the same way, written once rather than once per
/// option — the seventh took `applyOption` over the complexity threshold the
/// gate holds, and the nine of the duration adjustment would have been worse.
template<typename Parsed, typename Field>
void keepOption(
    SettingsRead& read, std::string_view key, std::string_view value, Parsed parsed, Field& field) {
    if (!parsed.has_value()) {
        read.diagnostics.push_back({.key = std::string{key}, .value = std::string{value}});
        return;
    }

    // **A setting that is itself optional receives the option as it is**, and
    // is not unwrapped to be wrapped again: it is the same content, and one
    // dereference fewer.
    if constexpr (std::same_as<std::remove_cvref_t<Field>, Parsed>)
        field = std::move(parsed);
    else
        field = *std::move(parsed);
}

/// Keeps one of the nine options of the duration adjustment.
///
/// **Apart from `applyOption`, which sends every `duration-adjust.` key here**:
/// nine more branches would have taken it over the complexity threshold, and
/// these have a shape of their own — a speed and a duration each carry a bound.
void applyDurationAdjustmentOption(SettingsRead& read,
                                   std::string_view key,
                                   std::string_view value) {
    const auto take = [&read, key, value](auto parsed, auto& field) {
        keepOption(read, key, value, std::move(parsed), field);
    };

    DurationAdjustmentSettings& form = read.settings.durationAdjustment;
    if (key == kDurationSpeedKey)
        take(speedOf(value), form.charactersPerSecond);
    else if (key == kDurationLengthenKey)
        take(booleanOf(value), form.lengthen);
    else if (key == kDurationShortenKey)
        take(booleanOf(value), form.shorten);
    else if (key == kDurationMinimumEnabledKey)
        take(booleanOf(value), form.minimumEnabled);
    else if (key == kDurationMinimumKey)
        take(millisecondsOf(value), form.minimumMilliseconds);
    else if (key == kDurationMaximumEnabledKey)
        take(booleanOf(value), form.maximumEnabled);
    else if (key == kDurationMaximumKey)
        take(millisecondsOf(value), form.maximumMilliseconds);
    else if (key == kDurationGapEnabledKey)
        take(booleanOf(value), form.gapEnabled);
    else if (key == kDurationGapKey)
        take(millisecondsOf(value), form.gapMilliseconds);
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
    // name the option otherwise.
    const auto take = [&read, key, value](auto parsed, auto& field) {
        keepOption(read, key, value, std::move(parsed), field);
    };

    if (key == kGeometryKey)
        take(geometryOf(value), read.settings.geometry);
    else if (key == kMaximisedKey)
        take(booleanOf(value), read.settings.maximised);
    else if (key == kColumnsKey)
        take(columnsOf(value), read.settings.columnWidths);
    else if (key == kColumnOrderKey)
        take(columnOrderOf(value), read.settings.columnOrder);
    else if (key == kHiddenColumnsKey)
        take(hiddenColumnsOf(value), read.settings.hiddenColumns);
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
    else if (key == kSearchRegexKey)
        take(booleanOf(value), read.settings.search.regex);
    else if (key == kSearchIgnoreCaseKey)
        take(booleanOf(value), read.settings.search.ignoreCase);
    else if (key.starts_with(kDurationPrefix))
        applyDurationAdjustmentOption(read, key, value);
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

[[nodiscard]] std::string_view flagText(bool flag) {
    return flag ? "true" : "false";
}

/// The nine options of the duration adjustment, each written bare when it
/// differs from Gaupol's and commented out when it does not.
///
/// **The speed in its shortest form**, `15` and not `15.000000`: `std::format`
/// writes the fewest digits that read back as the same number, so what the file
/// says is what was held, and a hand-edited `12.5` is not rewritten as
/// something longer.
void renderDurationAdjustment(std::string& out, const DurationAdjustmentSettings& form) {
    const DurationAdjustmentSettings defaults;
    writeOption(out,
                kDurationSpeedKey,
                std::format("{}", form.charactersPerSecond),
                form.charactersPerSecond == defaults.charactersPerSecond);
    writeOption(
        out, kDurationLengthenKey, flagText(form.lengthen), form.lengthen == defaults.lengthen);
    writeOption(out, kDurationShortenKey, flagText(form.shorten), form.shorten == defaults.shorten);
    writeOption(out,
                kDurationMinimumEnabledKey,
                flagText(form.minimumEnabled),
                form.minimumEnabled == defaults.minimumEnabled);
    writeOption(out,
                kDurationMinimumKey,
                std::to_string(form.minimumMilliseconds),
                form.minimumMilliseconds == defaults.minimumMilliseconds);
    writeOption(out,
                kDurationMaximumEnabledKey,
                flagText(form.maximumEnabled),
                form.maximumEnabled == defaults.maximumEnabled);
    writeOption(out,
                kDurationMaximumKey,
                std::to_string(form.maximumMilliseconds),
                form.maximumMilliseconds == defaults.maximumMilliseconds);
    writeOption(out,
                kDurationGapEnabledKey,
                flagText(form.gapEnabled),
                form.gapEnabled == defaults.gapEnabled);
    writeOption(out,
                kDurationGapKey,
                std::to_string(form.gapMilliseconds),
                form.gapMilliseconds == defaults.gapMilliseconds);
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
                kColumnOrderKey,
                settings.columnOrder.empty() ? "number,start,end,duration,text,translation"
                                             : namesOf(settings.columnOrder),
                settings.columnOrder.empty());

    writeOption(out,
                kHiddenColumnsKey,
                settings.hiddenColumns.empty() ? "start,end" : namesOf(settings.hiddenColumns),
                settings.hiddenColumns.empty());

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

    const SearchOptions search = settings.search;
    const SearchOptions defaults;
    writeOption(
        out, kSearchRegexKey, search.regex ? "true" : "false", search.regex == defaults.regex);
    writeOption(out,
                kSearchIgnoreCaseKey,
                search.ignoreCase ? "true" : "false",
                search.ignoreCase == defaults.ignoreCase);

    renderDurationAdjustment(out, settings.durationAdjustment);

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
