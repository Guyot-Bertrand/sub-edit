// Le thème clair et sombre — issue #241, décision D3.
//
// **Ce qui rend le thème éprouvable est sa forme.** Qt 6.4 n'a aucune API de
// schéma de couleurs, donc « système » ne peut rien lire ; mais parce que clair
// et sombre sont des palettes *que nous posons*, un test peut les poser aussi et
// lire ce qu'il obtient. Une lecture du bureau ne serait ni testable ni
// reproductible.
//
// **La lisibilité des teintes d'anomalie est vérifiée et non supposée.** Le
// modèle les veut translucides pour suivre le fond ; une couleur lisible sur
// blanc ne l'est pas nécessairement sur presque-noir, et rien ne le disait.

#include <subedit/core/config/theme.hpp>
#include <subedit/core/edit/session.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/time/timestamp.hpp>
#include <subedit/gui/subtitle_table_model.hpp>
#include <subedit/gui/theme.hpp>

#include <QApplication>
#include <QBrush>
#include <QColor>
#include <QPalette>
#include <QVariant>
#include <catch2/catch_test_macros.hpp>

#include <array>
#include <cmath>
#include <cstdint>
#include <vector>

namespace {

using subedit::core::Project;
using subedit::core::Session;
using subedit::core::Subtitle;
using subedit::core::Theme;
using subedit::core::Timestamp;
using subedit::gui::applyTheme;
using subedit::gui::paletteFor;
using subedit::gui::SubtitleTableModel;

/// Remet la palette de l'application telle qu'elle était.
///
/// `applyTheme` touche un état global du processus, et les cas qui suivent
/// n'ont pas demandé à en hériter. Le rendre dans un destructeur plutôt qu'en
/// fin de cas : une assertion qui échoue ne doit pas laisser le binaire peint
/// en sombre pour tout le reste.
class PaletteRestored {
public:
    PaletteRestored() : m_held(QApplication::palette()) {}

    PaletteRestored(const PaletteRestored&) = delete;
    PaletteRestored& operator=(const PaletteRestored&) = delete;
    PaletteRestored(PaletteRestored&&) = delete;
    PaletteRestored& operator=(PaletteRestored&&) = delete;

    ~PaletteRestored() { QApplication::setPalette(m_held); }

private:
    QPalette m_held;
};

/// La luminance relative d'une couleur, comme le calcul de contraste la définit.
[[nodiscard]] double luminanceOf(const QColor& colour) {
    const auto channel = [](double value) {
        return value <= 0.03928 ? value / 12.92 : std::pow((value + 0.055) / 1.055, 2.4);
    };
    return (0.2126 * channel(static_cast<double>(colour.redF()))) +
           (0.7152 * channel(static_cast<double>(colour.greenF()))) +
           (0.0722 * channel(static_cast<double>(colour.blueF())));
}

/// Le rapport de contraste entre deux couleurs opaques, de 1 à 21.
[[nodiscard]] double contrastOf(const QColor& one, const QColor& other) {
    const double bright = std::max(luminanceOf(one), luminanceOf(other));
    const double dim = std::min(luminanceOf(one), luminanceOf(other));
    return (bright + 0.05) / (dim + 0.05);
}

/// La couleur qu'on voit quand `wash` est peinte par-dessus `under`.
[[nodiscard]] QColor washedOver(const QColor& wash, const QColor& under) {
    // `QColor` travaille en `float` ; le calcul de contraste en `double`. La
    // composition se fait donc dans le type de Qt, et la conversion est écrite
    // plutôt que subie.
    const float alpha = wash.alphaF();
    const auto mix = [alpha](float top, float bottom) {
        return (top * alpha) + (bottom * (1 - alpha));
    };
    return QColor::fromRgbF(mix(wash.redF(), under.redF()),
                            mix(wash.greenF(), under.greenF()),
                            mix(wash.blueF(), under.blueF()));
}

[[nodiscard]] Subtitle from(std::int64_t start, std::int64_t end) {
    return Subtitle{.start = Timestamp::fromMilliseconds(start),
                    .end = Timestamp::fromMilliseconds(end),
                    .mainText = "x"};
}

/// Un document qui porte les trois anomalies à la fois, donc les trois teintes.
[[nodiscard]] Project damaged() {
    Project project;
    project.setSubtitles({from(1000, 2000),
                          // finit avant de commencer
                          from(3000, 2500),
                          // chevauche le précédent
                          from(2400, 5000),
                          // commence avant le précédent
                          from(2000, 6000)});
    return project;
}

/// Les teintes que la table pose vraiment, lues à travers le modèle.
[[nodiscard]] std::vector<QColor> tintsOf(const SubtitleTableModel& model) {
    std::vector<QColor> tints;
    for (int row = 0; row < model.rowCount({}); ++row) {
        const QVariant painted = model.data(model.index(row, 1), Qt::BackgroundRole);
        if (painted.isValid())
            tints.push_back(painted.value<QBrush>().color());
    }
    return tints;
}

} // namespace

TEST_CASE("sombre est sombre, et clair est clair", "[gui][theme][GUI-THEME-02]") {
    // Le fond, et non le nom : c'est ce sur quoi tout le reste se lit.
    CHECK(luminanceOf(paletteFor(Theme::Dark).base().color()) < 0.1);
    CHECK(luminanceOf(paletteFor(Theme::Light).base().color()) > 0.8);
}

TEST_CASE("la palette de système est celle qui est déjà là", "[gui][theme]") {
    // Rendre la palette courante est la façon la plus honnête de dire « rien » :
    // celle qui serait posée est celle qui l'est.
    const PaletteRestored restored;
    QPalette peculiar;
    peculiar.setColor(QPalette::Base, QColor{7, 8, 9});
    QApplication::setPalette(peculiar);

    CHECK(paletteFor(Theme::System).base().color() == QColor{7, 8, 9});
}

TEST_CASE("chaque palette lit son texte sur son fond", "[gui][theme]") {
    // Quatre et demi pour un : le seuil qu'un texte doit tenir pour être lu.
    for (const Theme theme : {Theme::Light, Theme::Dark}) {
        const QPalette palette = paletteFor(theme);
        CHECK(contrastOf(palette.text().color(), palette.base().color()) > 4.5);
        CHECK(contrastOf(palette.windowText().color(), palette.window().color()) > 4.5);
        CHECK(contrastOf(palette.highlightedText().color(), palette.highlight().color()) > 4.5);
    }
}

// **« Système » ne pose aucune palette**, et c'est le cœur de la décision D3 :
// on livre les deux thèmes qu'on sait poser, et on n'invente pas une lecture du
// bureau que Qt 6.4 ne permet pas.
TEST_CASE("système ne pose rien", "[gui][theme]") {
    const PaletteRestored restored;
    QPalette peculiar;
    peculiar.setColor(QPalette::Base, QColor{1, 2, 3});
    QApplication::setPalette(peculiar);

    applyTheme(Theme::System);

    CHECK(QApplication::palette().base().color() == QColor{1, 2, 3});
}

TEST_CASE("sombre pose sa palette sur l'application", "[gui][theme][GUI-THEME-02]") {
    const PaletteRestored restored;

    applyTheme(Theme::Dark);

    CHECK(QApplication::palette().base().color() == paletteFor(Theme::Dark).base().color());
}

// **Le point que l'issue demandait de vérifier plutôt que de supposer.** Le
// modèle teinte les colonnes de temps d'un lavis translucide, à dessein, pour
// que la fenêtre suive la palette du bureau. Une couleur lisible sur blanc ne
// l'est pas nécessairement sur presque-noir, et c'est le genre de chose qui ne
// se voit qu'en la regardant.
TEST_CASE("les teintes d'anomalie laissent lire le texte, clair comme sombre", "[gui][theme]") {
    Session session{damaged()};
    const SubtitleTableModel model{session};
    const std::vector<QColor> tints = tintsOf(model);

    REQUIRE(tints.size() >= 3);

    for (const Theme theme : {Theme::Light, Theme::Dark}) {
        const QPalette palette = paletteFor(theme);
        for (const QColor& tint : tints) {
            const QColor seen = washedOver(tint, palette.base().color());
            // Trois pour un : moins que le seuil d'un texte nu, parce qu'il
            // s'agit d'un fond teinté et non d'une couleur de texte — mais
            // assez pour que la ligne reste lue et non devinée.
            CHECK(contrastOf(palette.text().color(), seen) > 3.0);
        }
    }
}

TEST_CASE("les teintes restent distinctes du fond, clair comme sombre", "[gui][theme]") {
    // L'autre moitié : une teinte qu'on lit bien mais qu'on ne voit pas ne
    // signale rien. Elle doit se détacher du fond sans effacer le texte.
    Session session{damaged()};
    const SubtitleTableModel model{session};

    for (const Theme theme : {Theme::Light, Theme::Dark}) {
        const QColor base = paletteFor(theme).base().color();
        for (const QColor& tint : tintsOf(model))
            CHECK(contrastOf(washedOver(tint, base), base) > 1.05);
    }
}
