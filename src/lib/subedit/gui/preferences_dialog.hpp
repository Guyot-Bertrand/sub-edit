#pragma once

#include <subedit/core/config/theme.hpp>

#include <QDialog>

class QComboBox;
class QWidget;

namespace subedit::gui {

/// Ce qui se règle sans autre geste que de le régler.
///
/// **Une seule préférence pour l'instant, et c'est un critère et non un
/// manque** : la géométrie, l'état agrandi, les largeurs de colonnes et la
/// position de la poignée se posent en déplaçant la fenêtre ou en tirant une
/// bordure, et une préférence qui a déjà un geste n'a pas besoin d'un champ. Le
/// thème n'en a aucun.
///
/// La fréquence d'image par défaut, annoncée par le cadrage, viendra le jour où
/// quelque chose la lira : une préférence dont personne ne se sert est une case
/// qui ment. La relecture de fin de phase 7 a retiré la promesse de la spec et
/// lui a donné une issue plutôt qu'une condition — #267.
class PreferencesDialog final : public QDialog {
    Q_OBJECT

public:
    explicit PreferencesDialog(core::Theme theme, QWidget* parent = nullptr);

    /// Le thème choisi, qu'on ait validé ou non — l'appelant regarde le code de
    /// retour pour savoir s'il doit en tenir compte.
    [[nodiscard]] core::Theme theme() const;

    /// La liste des thèmes, pour qu'un test choisisse sans cliquer.
    [[nodiscard]] QComboBox* themeBox() const { return m_theme; }

private:
    QComboBox* m_theme;
};

} // namespace subedit::gui
