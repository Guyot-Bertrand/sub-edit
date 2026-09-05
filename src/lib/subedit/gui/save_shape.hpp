#pragma once

#include <subedit/core/model/encoding.hpp>
#include <subedit/core/model/source_file.hpp>
#include <subedit/gui/prompts.hpp>

#include <QWidget>

#include <array>
#include <expected>
#include <memory>
#include <optional>
#include <string>
#include <string_view>

class QCheckBox;
class QComboBox;
class QFileDialog;
class QGridLayout;
class QLineEdit;

namespace subedit::gui {

/// An encoding the window offers, and what it is for.
struct OfferedEncoding {
    /// The name ICU knows it by, which is the one the report and the settings
    /// file carry.
    std::string_view charset;

    /// Who writes in it, in the words gedit uses and Gaupol copied.
    std::string_view description;
};

/// The encodings the window offers, in the order it offers them.
///
/// **This is not a table of encodings, and the difference is the whole of D2**
/// of the phase's scoping. The set of encodings is ICU's — ninety-seven and
/// more, none of them written here — and anything in it can still be typed into
/// the box. What is written here is *what a menu proposes*: the encodings a
/// subtitle file carries in practice. A menu of ninety-seven entries is not a
/// help, and a list that refused the ninety-eighth would be a second truth.
///
/// UTF-8 first because it is what to write when nothing forces otherwise; the
/// rest by script, as Gaupol groups them.
inline constexpr std::array<OfferedEncoding, 14> kOfferedEncodings = {
    OfferedEncoding{.charset = "UTF-8", .description = "Unicode"},
    OfferedEncoding{.charset = "UTF-16LE", .description = "Unicode"},
    OfferedEncoding{.charset = "UTF-16BE", .description = "Unicode"},
    OfferedEncoding{.charset = "ISO-8859-1", .description = "Western"},
    OfferedEncoding{.charset = "windows-1252", .description = "Western"},
    OfferedEncoding{.charset = "ISO-8859-2", .description = "Central European"},
    OfferedEncoding{.charset = "windows-1250", .description = "Central European"},
    OfferedEncoding{.charset = "windows-1251", .description = "Cyrillic"},
    OfferedEncoding{.charset = "KOI8-R", .description = "Cyrillic"},
    OfferedEncoding{.charset = "ISO-8859-7", .description = "Greek"},
    OfferedEncoding{.charset = "ISO-8859-9", .description = "Turkish"},
    OfferedEncoding{.charset = "Shift_JIS", .description = "Japanese"},
    OfferedEncoding{.charset = "GB18030", .description = "Chinese simplified"},
    OfferedEncoding{.charset = "Big5", .description = "Chinese traditional"},
};

/// The shape of the file `Save As…` is about to write, beyond its format.
///
/// **Three questions the window could not ask until phase 8**, and the command
/// line has answered since phase 3: in which encoding, with which line endings,
/// and with a byte order mark or not. Asking one without the others would have
/// left the window half-way.
///
/// A widget of ours rather than something built inside `QtPrompts`: what the
/// box does is worth testing — a name nobody knows must not become a file
/// written wrong, and a mark asked of an encoding that has none must not be
/// silently dropped — and nothing of `QDialog::exec` can be. This is the same
/// cut every other dialog of this window is drawn on.
///
/// **It holds the fields and never shows them itself.** It used to carry a
/// `QFormLayout`, which gave it a column geometry of its own — so its labels
/// lined up with the dialog's and its fields did not, two columns to the left.
/// The fields go into the dialog's own grid now, and this object is left as
/// what it always really was: the three answers and the rules between them.
class SaveShape final : public QWidget {
    Q_OBJECT

public:
    /// Opens on what the file carries, which is what writing it back needs.
    SaveShape(const core::Encoding& encoding, core::Newline newline, QWidget* parent = nullptr);

    /// The encoding chosen, mark included, or why the name typed names none.
    ///
    /// A refusal is what stops the saving: writing a file in an encoding the
    /// model will not carry is not something to settle by picking another. The
    /// reason travels with it so that the box can say which of the two it is —
    /// a name nobody knows, or one whose converter writes its own mark.
    [[nodiscard]] std::expected<core::Encoding, core::EncodingRefusal> encoding() const;

    /// The line endings chosen.
    [[nodiscard]] core::Newline newline() const;

    /// Whether a byte order mark was asked for. False whenever the encoding
    /// carries none, whatever the box shows.
    [[nodiscard]] bool wantsByteOrderMark() const;

    /// Puts its fields into `grid`, from `firstRow` down, in the two columns a
    /// file dialog already uses for its own.
    ///
    /// **Here rather than in the caller** because which field goes where is
    /// what this class knows; the caller knows only where the free rows start.
    void layOutInto(QGridLayout& grid, int firstRow);

    /// The fields, so that a test sets them without clicking.
    [[nodiscard]] QComboBox* encodingBox() const { return m_encoding; }

    [[nodiscard]] QComboBox* newlineBox() const { return m_newline; }

    [[nodiscard]] QCheckBox* markBox() const { return m_mark; }

    [[nodiscard]] QLineEdit* otherName() const { return m_other; }

private:
    /// Shows the name field when « Other… » is picked, hides it otherwise, and
    /// greys the mark for an encoding that carries none.
    void refresh();

    QComboBox* m_encoding;
    QLineEdit* m_other;
    QComboBox* m_newline;
    QCheckBox* m_mark;
};

/// Puts a `SaveShape` into the layout of `host`, and hands it back.
///
/// **Here rather than in `QtPrompts`** so that the same three lines are what
/// the window shows, what a test drives and what the manual photographs. What
/// is left in `QtPrompts` is `exec`, which is all it may ever hold.
///
/// **A grid gets the fields row by row; anything else gets the widget whole.**
/// The first is what a `QFileDialog` gives — a grid of three columns, labels
/// left, fields middle, buttons right — and it is the only way the three new
/// rows line up with `File name:` and `Files of type:`. The second is a fallback
/// that keeps the fields present, if less well placed, the day Qt lays its
/// dialog out otherwise: a field absent would be worse than a field misplaced.
///
/// Which of the two is decided by `dynamic_cast` and never by `qobject_cast`,
/// for a reason the implementation measures.
///
/// `host` is a `QWidget` and not a `QFileDialog` because that is all this needs
/// to know — and because a fallback nothing can reach is a promise nobody
/// checks.
[[nodiscard]] SaveShape*
addSaveShapeTo(QWidget& host, const core::Encoding& encoding, core::Newline newline);

/// The whole question `Save As…` asks: where, in what format, in what shape.
///
/// **Qt's own dialog and not the system's**, and that is the price of the three
/// fields: a native box lets nothing be added to it.
///
/// Built here rather than in `QtPrompts` for the reason everything else is:
/// building a dialog blocks nothing, so a test fills this one in and reads what
/// it makes of it. Only `exec` is out of a test's reach, and only `exec` is
/// left there.
[[nodiscard]] std::unique_ptr<QFileDialog>
saveDialogFor(const core::SourceFile& current, const core::Encoding& encoding, QWidget* owner);

/// What the dialog was filled with, or the sentence that says why it was not.
///
/// The other half of what `QtPrompts` would otherwise hold out of reach. An
/// encoding nobody can name is the one refusal: writing a file in it is not
/// something to settle by picking another.
[[nodiscard]] std::expected<SaveTarget, std::string> targetOf(const QFileDialog& dialog);

} // namespace subedit::gui
