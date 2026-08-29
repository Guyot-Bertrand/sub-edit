#include <subedit/core/config/insert_placement.hpp>
#include <subedit/core/wording.hpp>
#include <subedit/gui/insert_dialog.hpp>

#include <QButtonGroup>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QRadioButton>
#include <QSpinBox>
#include <QString>
#include <QVBoxLayout>

#include <cstddef>

namespace subedit::gui {

namespace {

/// Combien de lignes on peut demander d'un coup.
///
/// Les bornes de Gaupol, reprises telles quelles : une au moins — insérer zéro
/// ligne n'est pas une opération — et un plafond assez haut pour n'avoir jamais
/// à être discuté.
constexpr int kSmallestCount = 1;
constexpr int kLargestCount = 99999;

/// Le bouton d'un côté, nommé par le noyau.
///
/// Une fonction pour deux appels, et non deux `new` dans la liste
/// d'initialisation : écrits là, ils dépassent la ligne, et une ligne coupée
/// dans une liste d'initialisation devient une ligne que la couverture compte
/// sans jamais l'atteindre — c'est le code de nettoyage d'exception qui s'y
/// range.
[[nodiscard]] QRadioButton* buttonFor(core::InsertPlacement placement, QWidget* parent) {
    return new QRadioButton{QString::fromUtf8(core::nameOf(placement)), parent};
}

} // namespace

InsertDialog::InsertDialog(bool hasSubtitles, core::InsertPlacement placement, QWidget* parent)
    : QDialog(parent),
      m_count(new QSpinBox{this}),
      m_above(buttonFor(core::InsertPlacement::Above, this)),
      m_below(buttonFor(core::InsertPlacement::Below, this)) {
    setWindowTitle(QStringLiteral("Insert subtitles"));

    m_count->setRange(kSmallestCount, kLargestCount);
    m_count->setValue(kSmallestCount);

    // Groupés explicitement : deux boutons radio d'un même parent le sont déjà,
    // mais le groupe dit l'intention là où la disposition ne fait que
    // l'impliquer — et il survivrait à un champ ajouté entre les deux.
    auto* side = new QButtonGroup{this};
    side->addButton(m_above);
    side->addButton(m_below);

    setPlacement(placement);

    // Éteint plutôt que caché dans un document vide : il n'y a pas de sélection
    // à situer, donc pas de côté à choisir, et l'insertion se fait au début.
    m_above->setEnabled(hasSubtitles);
    m_below->setEnabled(hasSubtitles);

    auto* fields = new QFormLayout;
    fields->addRow(QStringLiteral("How many"), m_count);
    fields->addRow(QStringLiteral("Where"), m_above);
    fields->addRow(QString{}, m_below);

    auto* buttons = new QDialogButtonBox{QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this};
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto* stack = new QVBoxLayout{this};
    stack->addLayout(fields);
    stack->addWidget(buttons);
}

std::size_t InsertDialog::count() const {
    // Le champ ne descend pas sous un, donc la conversion ne peut pas rendre
    // zéro : c'est la borne qui le garantit, et non un test ici.
    return static_cast<std::size_t>(m_count->value());
}

core::InsertPlacement InsertDialog::placement() const {
    return m_above->isChecked() ? core::InsertPlacement::Above : core::InsertPlacement::Below;
}

void InsertDialog::setPlacement(core::InsertPlacement placement) {
    m_above->setChecked(placement == core::InsertPlacement::Above);
    m_below->setChecked(placement == core::InsertPlacement::Below);
}

} // namespace subedit::gui
