#pragma once

#include <subedit/core/config/theme.hpp>

class QPalette;

namespace subedit::gui {

/// La palette d'un thème, posée par nous.
///
/// **Clair et sombre sont des palettes que ce fichier écrit**, et c'est ce qui
/// les rend éprouvables : un test demande « sombre » et lit ce qu'il obtient.
/// Une lecture du schéma de couleurs du système ne serait ni testable ni
/// reproductible sous Qt 6.4, qui n'en a pas l'API — décision D3.
///
/// **`Theme::System` rend la palette telle qu'elle est**, celle que la
/// plate-forme a posée : c'est la valeur qui ne fait rien, et rendre la palette
/// courante est la façon la plus honnête de le dire.
[[nodiscard]] QPalette paletteFor(core::Theme theme);

/// Pose la palette du thème sur l'application entière, ou ne pose rien.
///
/// **Rien du tout pour « système »**, et pas « la palette d'origine » : reposer
/// une palette capturée au démarrage la figerait, alors que ne rien poser laisse
/// la boîte à outils décider — y compris le jour où elle saura le faire.
void applyTheme(core::Theme theme);

} // namespace subedit::gui
