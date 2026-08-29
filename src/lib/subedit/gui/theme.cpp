#include <subedit/core/config/theme.hpp>
#include <subedit/gui/theme.hpp>

#include <QApplication>
#include <QColor>
#include <QPalette>

#include <utility>

namespace subedit::gui {

namespace {

/// Les deux fonds, et le reste s'en déduit.
///
/// Ce ne sont pas des couleurs choisies pour plaire : ce sont celles dont la
/// table a besoin pour que les quatre teintes d'anomalie restent lisibles
/// par-dessus. Le contraste est vérifié par un test plutôt que supposé.
constexpr QColor kLightWindow{239, 239, 239};
constexpr QColor kLightBase{255, 255, 255};
constexpr QColor kLightText{16, 16, 16};

constexpr QColor kDarkWindow{45, 45, 45};
constexpr QColor kDarkBase{30, 30, 30};
constexpr QColor kDarkText{232, 232, 232};

/// Le bleu de sélection, le même dans les deux palettes.
///
/// Une sélection qui change de teinte avec le thème serait un repère de moins :
/// c'est le seul élément que l'œil cherche sans le nommer.
constexpr QColor kHighlight{53, 110, 190};
constexpr QColor kHighlightText{255, 255, 255};

/// Le gris de ce qui est éteint, assez loin du texte pour se voir éteint.
constexpr QColor kLightDisabled{130, 130, 130};
constexpr QColor kDarkDisabled{120, 120, 120};

[[nodiscard]] QPalette
paletteOf(const QColor& window, const QColor& base, const QColor& text, const QColor& disabled) {
    QPalette palette;

    palette.setColor(QPalette::Window, window);
    palette.setColor(QPalette::WindowText, text);
    palette.setColor(QPalette::Base, base);
    // Une ligne sur deux, quand la table les alterne : assez proche du fond pour
    // ne pas rayer, assez loin pour se voir.
    palette.setColor(QPalette::AlternateBase, window);
    palette.setColor(QPalette::Text, text);
    palette.setColor(QPalette::Button, window);
    palette.setColor(QPalette::ButtonText, text);
    palette.setColor(QPalette::ToolTipBase, base);
    palette.setColor(QPalette::ToolTipText, text);
    palette.setColor(QPalette::Highlight, kHighlight);
    palette.setColor(QPalette::HighlightedText, kHighlightText);

    palette.setColor(QPalette::Disabled, QPalette::Text, disabled);
    palette.setColor(QPalette::Disabled, QPalette::ButtonText, disabled);
    palette.setColor(QPalette::Disabled, QPalette::WindowText, disabled);

    return palette;
}

} // namespace

QPalette paletteFor(core::Theme theme) {
    switch (theme) {
    case core::Theme::System:
        return QApplication::palette();
    case core::Theme::Light:
        return paletteOf(kLightWindow, kLightBase, kLightText, kLightDisabled);
    case core::Theme::Dark:
        return paletteOf(kDarkWindow, kDarkBase, kDarkText, kDarkDisabled);
    }
    std::unreachable();
}

void applyTheme(core::Theme theme) {
    if (theme == core::Theme::System)
        return;

    QApplication::setPalette(paletteFor(theme));
}

} // namespace subedit::gui
