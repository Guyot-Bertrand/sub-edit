#include <subedit/core/i18n/language.hpp>
#include <subedit/gui/manual_path.hpp>

#include <QCoreApplication>
#include <QString>

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace subedit::gui {

std::filesystem::path installedManualPath() {
    // `applicationDirPath()` asks for a `QCoreApplication` already built,
    // which is the case everywhere this function serves: `main` calls it after
    // building its own.
    const QString binaries = QCoreApplication::applicationDirPath();

    // `lexically_normal` so that the `..` leaves the path rather than being
    // carried all the way into the error messages.
    return (std::filesystem::path{binaries.toStdString()} / ".." / "share" / "subedit" / "manual")
        .lexically_normal();
}

std::vector<std::filesystem::path> manualTrees(const std::filesystem::path& root,
                                               const std::optional<std::string>& language) {
    std::vector<std::filesystem::path> trees;

    if (language && core::isLanguageName(*language)) {
        trees.push_back(root / *language);

        // `fr_CA` falls back on `fr` before English: a French reader in Canada reads the French
        // manual rather than the English one.
        const std::string::size_type region = language->find('_');
        if (region != std::string::npos)
            trees.push_back(root / language->substr(0, region));
    }

    trees.push_back(root);
    return trees;
}

} // namespace subedit::gui
