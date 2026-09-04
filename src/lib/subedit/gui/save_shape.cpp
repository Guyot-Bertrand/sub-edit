#include <subedit/core/model/encoding.hpp>
#include <subedit/core/model/source_file.hpp>
#include <subedit/core/wording.hpp>
#include <subedit/gui/qt_prompts.hpp>
#include <subedit/gui/save_shape.hpp>

#include <QCheckBox>
#include <QComboBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QLayout>
#include <QLineEdit>
#include <QString>
#include <QVariant>

#include <array>
#include <cstddef>
#include <expected>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>

namespace subedit::gui {

namespace {

/// Ce que porte l'entrée « autre… », et que rien d'autre ne porte : le nom d'un
/// encodage est une chaîne, donc une donnée d'entrée reconnaissable est ce qui
/// distingue l'entrée sans comparer son intitulé.
constexpr int kOtherEncoding = -1;

/// Les trois fins de ligne, dans l'ordre où la ligne de commande les nomme.
constexpr std::array<core::Newline, 3> kNewlines = {
    core::Newline::Lf,
    core::Newline::CrLf,
    core::Newline::Cr,
};

/// « UTF-8 (Unicode) », l'intitulé d'une entrée de la liste.
[[nodiscard]] QString labelOf(const OfferedEncoding& offered) {
    return QStringLiteral("%1 (%2)").arg(
        QString::fromUtf8(offered.charset.data(), static_cast<int>(offered.charset.size())),
        QString::fromUtf8(offered.description.data(),
                          static_cast<int>(offered.description.size())));
}

} // namespace

SaveShape::SaveShape(const core::Encoding& encoding, core::Newline newline, QWidget* parent)
    : QWidget(parent),
      m_encoding(new QComboBox{this}),
      m_other(new QLineEdit{this}),
      m_newline(new QComboBox{this}),
      m_mark(new QCheckBox{QStringLiteral("Byte order mark"), this}) {
    // L'indice de l'entrée porté en donnée : c'est lui qui dit quel encodage la
    // liste propose, sans avoir à relire un intitulé.
    for (std::size_t index = 0; index < kOfferedEncodings.size(); ++index)
        m_encoding->addItem(labelOf(kOfferedEncodings[index]), static_cast<int>(index));

    // **« Autre… » plutôt qu'une liste de quatre-vingt-dix-sept entrées.** Ce
    // que la liste offre est ce qu'un fichier de sous-titres porte en pratique ;
    // ce que le champ accepte est tout ce qu'ICU sait convertir.
    m_encoding->addItem(QStringLiteral("Other…"), kOtherEncoding);

    m_other->setPlaceholderText(QStringLiteral("Name of an encoding, e.g. cp1257"));

    // L'intitulé vient du noyau, comme partout ; la donnée serait de trop —
    // l'indice suffit, les trois entrées étant dans l'ordre de `kNewlines`.
    for (const core::Newline ending : kNewlines)
        m_newline->addItem(QString::fromUtf8(core::nameOf(ending)));

    // Ce que le fichier porte, pour défauts : le réécrire autrement sans qu'on
    // l'ait demandé serait perdre ce que la lecture a pris soin de garder.
    const int known = m_encoding->findText(
        QString::fromUtf8(encoding.charset().data(), static_cast<int>(encoding.charset().size())),
        Qt::MatchStartsWith);
    if (known >= 0) {
        m_encoding->setCurrentIndex(known);
    } else {
        m_encoding->setCurrentIndex(m_encoding->count() - 1);
        m_other->setText(QString::fromUtf8(encoding.charset().data(),
                                           static_cast<int>(encoding.charset().size())));
    }

    m_newline->setCurrentIndex(m_newline->findText(QString::fromUtf8(core::nameOf(newline))));
    m_mark->setChecked(encoding.byteOrderMark() == core::ByteOrderMark::Present);

    auto* fields = new QFormLayout{this};
    fields->setContentsMargins(0, 0, 0, 0);
    fields->addRow(QStringLiteral("Encoding:"), m_encoding);
    fields->addRow(QString{}, m_other);
    fields->addRow(QStringLiteral("Line endings:"), m_newline);
    fields->addRow(QString{}, m_mark);

    connect(m_encoding, &QComboBox::currentIndexChanged, this, [this] { refresh(); });
    connect(m_other, &QLineEdit::textChanged, this, [this] { refresh(); });
    refresh();
}

void SaveShape::refresh() {
    // Le champ n'apparaît que pour « autre… » : montré toujours, il donnerait à
    // croire qu'il faut le remplir ; caché toujours, il n'y aurait pas d'autre.
    m_other->setVisible(m_encoding->currentData().toInt() == kOtherEncoding);

    // **Une marque n'existe que pour les encodages Unicode.** La case est
    // éteinte plutôt que cachée, pour la raison qui vaut ailleurs dans cette
    // fenêtre : une case grisée dit pourquoi le choix ne s'offre pas, une case
    // absente laisse croire à un manque.
    const std::expected<core::Encoding, core::EncodingRefusal> chosen = encoding();
    const bool carries = chosen.has_value() && !chosen->byteOrderMarkBytes().empty();
    m_mark->setEnabled(carries);
}

std::expected<core::Encoding, core::EncodingRefusal> SaveShape::encoding() const {
    const int index = m_encoding->currentData().toInt();
    const core::ByteOrderMark mark =
        m_mark->isChecked() ? core::ByteOrderMark::Present : core::ByteOrderMark::Absent;

    if (index != kOtherEncoding)
        return core::Encoding::create(kOfferedEncodings[static_cast<std::size_t>(index)].charset,
                                      mark);

    return core::Encoding::create(m_other->text().trimmed().toStdString(), mark);
}

core::Newline SaveShape::newline() const {
    return kNewlines[static_cast<std::size_t>(m_newline->currentIndex())];
}

bool SaveShape::wantsByteOrderMark() const {
    return m_mark->isChecked() && m_mark->isEnabled();
}

SaveShape*
addSaveShapeTo(QFileDialog& dialog, const core::Encoding& encoding, core::Newline newline) {
    auto* shape = new SaveShape{encoding, newline, &dialog};

    // `QLayout::addWidget` plutôt qu'un `qobject_cast` vers la grille de Qt :
    // le premier vaut pour toute disposition, le second parie sur celle que
    // `QFileDialog` se donne aujourd'hui.
    if (QLayout* layout = dialog.layout())
        layout->addWidget(shape);

    return shape;
}

std::unique_ptr<QFileDialog>
saveDialogFor(const core::SourceFile& current, const core::Encoding& encoding, QWidget* owner) {
    auto dialog = std::make_unique<QFileDialog>(owner, QStringLiteral("Save As"));

    // **Celle de Qt et non celle du système** : une boîte native ne se laisse
    // rien ajouter, et les trois champs sont à ce prix.
    dialog->setOption(QFileDialog::DontUseNativeDialog);
    dialog->setAcceptMode(QFileDialog::AcceptSave);
    dialog->setNameFilter(subtitleFilters());
    dialog->selectNameFilter(subtitleFilters().section(QStringLiteral(";;"), 1, 1));
    if (current.path.has_value())
        dialog->selectFile(QString::fromStdString(current.path->string()));

    (void)addSaveShapeTo(*dialog, encoding, current.newline);
    return dialog;
}

std::expected<SaveTarget, std::string> targetOf(const QFileDialog& dialog) {
    // La garde d'une boîte qui n'est pas la nôtre, ou qui n'a rien retenu.
    // Aucun test ne l'atteint — `saveDialogFor` pose toujours la forme, et une
    // boîte acceptée porte toujours un nom — et elle reste : le pas suivant
    // déréférence les deux.
    const auto* shape = dialog.findChild<const SaveShape*>();
    if (shape == nullptr || dialog.selectedFiles().isEmpty())
        return std::unexpected(std::string{"nothing was chosen"});

    // Un encodage que le modèle refuse n'écrit pas un fichier de travers : il
    // n'écrit pas de fichier. Les mots sont ceux du noyau, comme partout — la
    // ligne de commande dit la même chose du même refus.
    const std::expected<core::Encoding, core::EncodingRefusal> chosen = shape->encoding();
    if (!chosen.has_value())
        return std::unexpected(
            core::refusalOf(chosen.error(), shape->otherName()->text().trimmed().toStdString()));

    return SaveTarget{
        .path = std::filesystem::path{dialog.selectedFiles().first().toStdString()},
        .format = formatOfFilter(dialog.selectedNameFilter()),
        .encoding =
            chosen->withByteOrderMark(shape->wantsByteOrderMark() ? core::ByteOrderMark::Present
                                                                  : core::ByteOrderMark::Absent),
        .newline = shape->newline(),
    };
}

} // namespace subedit::gui
