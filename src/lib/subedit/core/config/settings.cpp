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

// Les clés du fichier. **C'est un format, et non de la prose** : elles ne
// passent pas par `core/wording.hpp` pour la même raison que `WEBVTT` n'y passe
// pas — ce qu'un lecteur humain lit ici, ce sont les commentaires, qui eux en
// viennent.
constexpr std::string_view kGeometryKey = "window.geometry";
constexpr std::string_view kMaximisedKey = "window.maximised";
constexpr std::string_view kColumnsKey = "table.columns";
constexpr std::string_view kTableShareKey = "window.table-share";
constexpr std::string_view kDirectoryKey = "file.directory";
constexpr std::string_view kThemeKey = "general.theme";
constexpr std::string_view kInsertPlacementKey = "edit.insert-placement";
constexpr std::string_view kWriteEncodingKey = "file.write-encoding";

// Les trois valeurs du thème, telles que le fichier les porte. En minuscules et
// séparées de `nameOf(Theme)`, qui donne les intitulés du dialogue : ceci est un
// format, cela est de la prose.
constexpr std::string_view kSystemTheme = "system";
constexpr std::string_view kLightTheme = "light";
constexpr std::string_view kDarkTheme = "dark";

// Les deux côtés d'une insertion, tels que le fichier les porte — et pour la
// même raison que le thème : ce sont des jetons de format, pas les intitulés
// que le dialogue montre.
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

/// Un entier, ou rien. Refuse ce qui traîne après le nombre : « 12 pixels » est
/// une valeur qu'on n'a pas su lire, et non un douze.
[[nodiscard]] std::optional<int> integerOf(std::string_view text) {
    int value = 0;
    // `std::to_address` plutôt que `.data()` : la vue porte sa longueur, et un
    // pointeur nu passé sans elle est précisément ce que l'analyse statique
    // refuse — à raison, même quand la fin est donnée juste à côté.
    const char* const first = std::to_address(text.begin());
    const char* const last = std::to_address(text.end());
    const std::from_chars_result read = std::from_chars(first, last, value);
    if (read.ec != std::errc{} || read.ptr != last)
        return std::nullopt;
    return value;
}

/// Les entiers d'une liste séparée par des virgules, ou rien si l'un d'eux
/// n'en est pas un.
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

/// Un répertoire, s'il est absolu.
///
/// Un chemin relatif est relatif à un répertoire courant que personne ne
/// connaît : c'est une valeur qu'on n'a pas su lire, et non un chemin à
/// compléter au petit bonheur.
[[nodiscard]] std::optional<std::filesystem::path> directoryOf(std::string_view text) {
    std::filesystem::path directory{text};
    if (directory.empty() || !directory.is_absolute())
        return std::nullopt;
    return directory;
}

[[nodiscard]] std::optional<WindowGeometry> geometryOf(std::string_view text) {
    const std::optional<std::vector<int>> numbers = integersOf(text);
    // Quatre nombres, et des dimensions qui ne soient pas nulles : une fenêtre
    // large de zéro est une fenêtre qu'on ne retrouverait jamais.
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

/// Retient une option, ou signale la valeur qu'on n'a pas su lire.
///
/// **Une clé inconnue est ignorée, sans un mot** : un fichier écrit par une
/// version qui en connaissait plus n'est pas un fichier fautif, et c'est le mode
/// d'échec que la tolérance choisit.
void applyOption(SettingsRead& read, std::string_view key, std::string_view value) {
    // Ce que les sept options font toutes de la même façon : poser ce qu'on a su
    // lire, nommer l'option sinon. Écrit une fois plutôt que sept, et ce n'est
    // pas qu'une économie de lignes — la septième option a fait franchir à cette
    // fonction le seuil de complexité que la porte tient.
    const auto take = [&read, key, value](auto parsed, auto& field) {
        if (!parsed.has_value()) {
            read.diagnostics.push_back({.key = std::string{key}, .value = std::string{value}});
            return;
        }

        // **Un réglage qui est lui-même optionnel reçoit l'optionnel tel quel**,
        // et n'est pas déballé pour être remballé : c'est le même contenu, et un
        // déréférencement de moins.
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
        // Le nom que la marque ne concerne pas : un réglage nomme un encodage,
        // et `--bom` — ou la case du dialogue — dit ce qu'il en est de la
        // marque. Un nom qu'ICU ne sait pas convertir est un réglage illisible,
        // qui se signale comme les autres.
        take(Encoding::create(value, ByteOrderMark::Absent), read.settings.writeEncoding);
}

/// Une option, écrite nue si elle est réglée, commentée si elle est au défaut.
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

    // **Absent et illisible donnent tous les deux les défauts, et ne se disent
    // pas de la même façon.** Un fichier qui n'existe pas est le premier
    // lancement, et il n'y a rien à signaler ; un fichier qui existe et se
    // refuse est un réglage perdu, que l'utilisateur a le droit de savoir
    // perdu. Ni l'un ni l'autre n'est une raison de ne pas démarrer.
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

        applyOption(read, key, value);
    }

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

    return out;
}

std::expected<void, FileError>
writeSettings(FileSystem& files, const std::filesystem::path& path, const Settings& settings) {
    // Le répertoire d'abord : au premier lancement, personne ne l'a fait.
    if (const std::expected<void, FileError> made = files.createDirectories(path.parent_path());
        !made)
        return made;

    return writeAtomically(files, path, renderSettings(settings));
}

} // namespace subedit::core
