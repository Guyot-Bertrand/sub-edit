#include <subedit/gui/cell_delegates.hpp>

#include <QEvent>
#include <QKeyEvent>
#include <QLineEdit>
#include <QObject>
#include <QPlainTextEdit>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QString>
#include <QWidget>

namespace subedit::gui {

namespace {

/// La forme d'un horodatage saisissable, et rien de plus.
///
/// **Elle contraint les caractères, pas les bornes.** `00:70:00,000` la
/// traverse et c'est voulu : dire qu'une minute s'arrête à soixante est le
/// travail de `Timestamp::parse`, qui le fait déjà, et le redire ici en ferait
/// deux définitions à tenir d'accord. Ce que la forme empêche est ce que la
/// lecture ne rattraperait jamais utilement — une lettre au milieu d'un
/// horodatage.
///
/// Permissive comme la lecture l'est : heures facultatives, un ou deux chiffres
/// par champ, une à trois décimales ou aucune, virgule ou point.
constexpr auto kPositionPattern = R"(\s*-?\d{1,2}:\d{1,2}(:\d{1,2})?([.,]\d{1,3})?\s*)";

} // namespace

QWidget* TextDelegate::createEditor(QWidget* parent,
                                    const QStyleOptionViewItem& /*option*/,
                                    const QModelIndex& /*index*/) const {
    // `plainText` est la propriété USER d'un `QPlainTextEdit`, ce qui suffit à
    // `setEditorData` et `setModelData` héritées : elles lisent et écrivent
    // celle-là. Rien à redéfinir pour aller chercher le texte.
    auto* editor = new QPlainTextEdit{parent};

    // Sinon la tabulation ferait un caractère dans le texte plutôt que de
    // passer à la cellule suivante, ce que personne n'attend d'une table.
    editor->setTabChangesFocus(true);
    return editor;
}

bool TextDelegate::eventFilter(QObject* object, QEvent* event) {
    if (event->type() == QEvent::KeyPress) {
        // `dynamic_cast` là où le type de l'événement suffirait : la règle du
        // projet refuse de descendre une hiérarchie sans vérification, et
        // vérifier deux fois une frappe ne coûte rien de mesurable.
        const auto* key = dynamic_cast<const QKeyEvent*>(event);
        if (key != nullptr && (key->key() == Qt::Key_Return || key->key() == Qt::Key_Enter)) {
            // **Le saut de ligne appartient à l'éditeur.** Rendre la frappe
            // sans rien valider est tout ce qu'il y a à faire : c'est le
            // `QPlainTextEdit` qui en fera une ligne de plus.
            if (key->modifiers().testFlag(Qt::ShiftModifier))
                return false;

            // Avalée, et c'est le point. La filtration héritée valide sur
            // `Entrée` mais rend la frappe à l'éditeur — bon pour un champ
            // d'une ligne, désastreux ici : le texte validé porterait le saut
            // de ligne que la validation venait de refuser.
            if (auto* editor = qobject_cast<QWidget*>(object); editor != nullptr) {
                emit commitData(editor);
                emit closeEditor(editor, QAbstractItemDelegate::SubmitModelCache);
                return true;
            }
        }
    }

    // Échap annule, la perte du focus valide : les deux viennent de Qt, et
    // rien ici ne les touche.
    return QStyledItemDelegate::eventFilter(object, event);
}

QWidget* PositionDelegate::createEditor(QWidget* parent,
                                        const QStyleOptionViewItem& /*option*/,
                                        const QModelIndex& /*index*/) const {
    auto* editor = new QLineEdit{parent};
    editor->setValidator(new QRegularExpressionValidator{
        QRegularExpression{QString::fromUtf8(kPositionPattern)}, editor});
    return editor;
}

} // namespace subedit::gui
