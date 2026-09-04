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

/// Les bornes de la part donnée à la table, en pour cent.
///
/// Ni zéro ni cent : une table haute de rien, ou une bande vidéo haute de rien,
/// est une fenêtre qu'on ne saurait plus rouvrir autrement qu'en effaçant son
/// fichier de configuration.
inline constexpr int kSmallestTableShare = 1;
inline constexpr int kLargestTableShare = 99;

/// How many column widths a settings file gives.
///
/// **Quatre pour cinq colonnes, et ce n'est pas un oubli.** La dernière — le
/// texte — prend ce que les autres laissent : c'est la colonne qui varie, et
/// les quatre premières sont des largeurs connues. Retenir la cinquième serait
/// retenir un nombre que la table recalcule à chaque redimensionnement, donc
/// écrire dans le fichier une valeur que la relecture ne pourrait pas honorer.
///
/// Un fichier qui en nomme trois ou cinq est un fichier dont les largeurs ne
/// veulent rien dire : le lecteur garde son défaut et le dit, plutôt que de
/// deviner de quelle colonne il s'agissait.
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
    /// **Une proportion et non des hauteurs**, et c'est le point à poser :
    /// trois hauteurs absolues qui ne totalisent pas la fenêtre ne veulent rien
    /// dire dès que celle-ci s'ouvre à une autre taille, et une fenêtre s'ouvre
    /// souvent à une autre taille. Une part, elle, se rejoue partout.
    ///
    /// Absente par défaut : la fenêtre partage alors sa hauteur comme elle l'a
    /// toujours fait.
    std::optional<int> tableShare{};

    /// Où la boîte « ouvrir » s'ouvre, absolu ou absent.
    ///
    /// **Retenir un répertoire n'est pas retenir un fichier**, et c'est la
    /// coupure du critère D5 : un répertoire *pointe* une boîte de dialogue,
    /// un fichier rouvrirait au lancement un document que personne n'a
    /// demandé.
    ///
    /// Absolu, ou rien : un chemin relatif dans un fichier de configuration est
    /// relatif à un répertoire courant que personne ne connaît.
    std::optional<std::filesystem::path> lastDirectory{};

    Theme theme = Theme::System;

    /// De quel côté de la sélection `Insert Subtitles…` pose ses lignes.
    ///
    /// **Après elle par défaut**, ce qui est le défaut de Gaupol. C'est le sens
    /// où l'on écrit un fichier de sous-titres : la ligne suivante vient après
    /// celle qu'on vient de regarder.
    InsertPlacement insertPlacement = InsertPlacement::Below;

    /// Le dernier encodage choisi dans `Save As…`, absent au premier lancement.
    ///
    /// **Il ne l'emporte jamais sur l'encodage d'un fichier lu**, et c'est la
    /// coupure : un document ouvert porte le sien, et le réécrire dans un autre
    /// parce qu'un réglage vieux de trois semaines le dit serait une perte que
    /// personne n'a demandée — l'aller-retour d'octets de la phase 8 est
    /// exactement cette promesse-là. Ce dont il se souvient sert au document qui
    /// n'a pas de fichier : celui qu'on vient de créer.
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
