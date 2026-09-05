#include <subedit/core/model/encoding.hpp>
#include <subedit/core/model/source_file.hpp>
#include <subedit/core/wording.hpp>
#include <subedit/gui/qt_prompts.hpp>
#include <subedit/gui/save_shape.hpp>

#include <QCheckBox>
#include <QComboBox>
#include <QFileDialog>
#include <QGridLayout>
#include <QLabel>
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

/// What the « Other… » entry carries, and nothing else does: the name of an
/// encoding is a string, so a recognisable item datum is what tells that entry
/// apart without comparing its label.
constexpr int kOtherEncoding = -1;

/// The three line endings, in the order the command line names them.
constexpr std::array<core::Newline, 3> kNewlines = {
    core::Newline::Lf,
    core::Newline::CrLf,
    core::Newline::Cr,
};

/// « UTF-8 (Unicode) », the label of one entry of the list.
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
    // The index of the entry carried as its datum: that is what says which
    // encoding the list offers, without reading a label back.
    for (std::size_t index = 0; index < kOfferedEncodings.size(); ++index)
        m_encoding->addItem(labelOf(kOfferedEncodings[index]), static_cast<int>(index));

    // **« Other… » rather than a list of ninety-seven entries.** What the list
    // offers is what a subtitle file carries in practice; what the field takes
    // is anything ICU can convert.
    m_encoding->addItem(QStringLiteral("Other…"), kOtherEncoding);

    m_other->setPlaceholderText(QStringLiteral("Name of an encoding, e.g. cp1257"));

    // The label comes from the core, as everywhere; a datum would be one thing
    // too many — the index does, the three entries being in `kNewlines` order.
    for (const core::Newline ending : kNewlines)
        m_newline->addItem(QString::fromUtf8(core::nameOf(ending)));

    // What the file carries, for defaults: writing it back otherwise without
    // being asked would lose what the reading took care to keep.
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

    // **No layout here, and that is the whole of issue #321.** A `QFormLayout`
    // put on this widget gave it a column geometry of its own: its labels
    // landed in the same column as the dialog's — both start at the left
    // margin — and its fields two columns too far left, nothing tying the two
    // grids together.
    connect(m_encoding, &QComboBox::currentIndexChanged, this, [this] { refresh(); });
    connect(m_other, &QLineEdit::textChanged, this, [this] { refresh(); });
    refresh();
}

void SaveShape::refresh() {
    // The field shows for « Other… » and for nothing else: always shown, it
    // would suggest it has to be filled in; never shown, there would be no
    // other.
    m_other->setVisible(m_encoding->currentData().toInt() == kOtherEncoding);

    // **A mark exists for the Unicode encodings and for no other.** The box is
    // greyed rather than hidden, for the reason that holds elsewhere in this
    // window: a greyed box says why the choice is not on offer, an absent one
    // reads as something missing.
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

void SaveShape::layOutInto(QGridLayout& grid, int firstRow) {
    // The two columns a file dialog already uses: the label on the left, the
    // field in the middle. The third carries its buttons, over both rows, and
    // nothing of ours belongs there.
    constexpr int kLabels = 0;
    constexpr int kFields = 1;

    grid.addWidget(new QLabel{QStringLiteral("Encoding:"), this}, firstRow, kLabels);
    grid.addWidget(m_encoding, firstRow, kFields);
    // Under its list and with no label: the field is the rest of « Other… »,
    // not one more question. Hidden, it takes no height at all.
    grid.addWidget(m_other, firstRow + 1, kFields);
    grid.addWidget(new QLabel{QStringLiteral("Line endings:"), this}, firstRow + 2, kLabels);
    grid.addWidget(m_newline, firstRow + 2, kFields);
    grid.addWidget(m_mark, firstRow + 3, kFields);
}

SaveShape* addSaveShapeTo(QWidget& host, const core::Encoding& encoding, core::Newline newline) {
    auto* shape = new SaveShape{encoding, newline, &host};

    QLayout* layout = host.layout();
    if (layout == nullptr)
        return shape;

    // **`dynamic_cast` and not `qobject_cast`, and that is measured.** The
    // second is what Qt teaches, and it is the one that must not be used here:
    // on the very layout of a `QFileDialog`, which *is* a grid, it answers no.
    //
    //     className     QGridLayout
    //     qobject_cast  fails
    //     dynamic_cast  succeeds
    //     inherits      1
    //
    // Two `QGridLayout::staticMetaObject` live in the process — the Qt
    // library's and this executable's — and `qobject_cast` compares their
    // addresses. The language's own cast compares type identities and is right.
    // Checked under both linkages, with and without this project's libraries.
    auto* grid = dynamic_cast<QGridLayout*>(layout);
    if (grid == nullptr) {
        layout->addWidget(shape);
        return shape;
    }

    shape->layOutInto(*grid, grid->rowCount());

    // The widget itself never shows: its fields live in the dialog's grid now,
    // which has adopted them. It stays a child of the dialog, so `findChild`
    // still reaches it, and it keeps what it knows about them.
    shape->hide();
    return shape;
}

std::unique_ptr<QFileDialog>
saveDialogFor(const core::SourceFile& current, const core::Encoding& encoding, QWidget* owner) {
    auto dialog = std::make_unique<QFileDialog>(owner, QStringLiteral("Save As"));

    // **Qt's own and not the system's**: a native dialog lets nothing be added
    // to it, and the three fields are worth that price.
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
    // The guard for a dialog that is not ours, or that kept nothing. No test
    // reaches it — `saveDialogFor` always puts the shape in, and an accepted
    // dialog always carries a name — and it stays: the next step dereferences
    // both.
    const auto* shape = dialog.findChild<const SaveShape*>();
    if (shape == nullptr || dialog.selectedFiles().isEmpty())
        return std::unexpected(std::string{"nothing was chosen"});

    // An encoding the model refuses does not write a file wrong: it writes no
    // file. The words are the core's, as everywhere — the command line says the
    // same thing of the same refusal.
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
