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
/// La fréquence d'image par défaut, annoncée par le cadrage, **ne viendra
/// pas** : #267 l'a instruite puis écartée. Aucun des trois lecteurs possibles
/// n'en avait besoin — la conversion pré-remplit son champ du haut avec la
/// grille déduite, l'alignement s'ouvre sur ce que la vidéo déclare, et un
/// projet sans l'une ni l'autre est un projet où l'utilisateur choisit. Une
/// préférence dont personne ne se sert est une case qui ment.
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
