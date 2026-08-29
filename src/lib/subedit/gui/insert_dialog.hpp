#pragma once

#include <subedit/core/config/insert_placement.hpp>

#include <QDialog>

#include <cstddef>

class QRadioButton;
class QSpinBox;
class QWidget;

namespace subedit::gui {

/// Combien de lignes vierges insérer, et de quel côté de la sélection.
///
/// **Pas un `OperationDialog`**, et la différence n'est pas de forme : les
/// quatre autres annoncent « Applies to: 4 subtitles », parce qu'ils
/// transforment des sous-titres qui existent. Celui-ci n'en touche aucun — il
/// en ajoute — et la phrase serait fausse dans le seul cas qui compte, le
/// document vide.
///
/// `hasSubtitles` éteint le choix du côté plutôt que de le cacher : dans un
/// document vide il n'y a pas de sélection, donc pas de côté, et l'insertion se
/// fait à l'index zéro. Une case grisée dit pourquoi le choix ne s'offre pas ;
/// une case absente laisse croire à un manque.
class InsertDialog final : public QDialog {
    Q_OBJECT

public:
    InsertDialog(bool hasSubtitles, core::InsertPlacement placement, QWidget* parent = nullptr);

    /// Combien de lignes, jamais zéro.
    [[nodiscard]] std::size_t count() const;

    /// Le côté choisi, qu'on ait validé ou non — l'appelant regarde le code de
    /// retour pour savoir s'il doit en tenir compte, comme pour le thème.
    [[nodiscard]] core::InsertPlacement placement() const;

    /// Les champs, pour qu'un test les règle sans cliquer.
    [[nodiscard]] QSpinBox* countBox() const { return m_count; }

    void setPlacement(core::InsertPlacement placement);

private:
    QSpinBox* m_count;
    QRadioButton* m_above;
    QRadioButton* m_below;
};

} // namespace subedit::gui
