#pragma once

// The four task pages of the correction assistant — issue #505, task 9.

#include <subedit/core/config/correction_settings.hpp>
#include <subedit/core/text/correction_pattern.hpp>

#include <QWizardPage>

#include <string>
#include <vector>

namespace subedit::core {
class PatternCatalogue;
} // namespace subedit::core

class QCheckBox;
class QDoubleSpinBox;
class QSpinBox;
class QComboBox;
class QVBoxLayout;

namespace subedit::gui {

class PatternCodeSelector;
class PatternList;

/// D8's shared shape of a task page: the code its patterns are chosen for,
/// and the patterns themselves by name. Whether the task itself runs is not
/// this page's concern — see the note on `applyBase` below.
class CorrectionTaskPage : public QWizardPage {
    Q_OBJECT

public:
    CorrectionTaskPage(const QString& title,
                       const core::PatternCatalogue& catalogue,
                       core::PatternKind kind,
                       QWidget* parent = nullptr);

    /// Opens the page on `settings` — every subclass overrides this to read
    /// its own `TaskSettings` field and any task-specific extra it owns.
    virtual void applySettings(const core::CorrectionSettings& settings) = 0;

    [[nodiscard]] std::string code() const;

    /// Writes this page's pattern choices into `activations`: every entry of
    /// this page's kind is replaced by what the page now holds — the settings
    /// it opened on, with every code shown this run folded in
    /// (`core::foldActivations`). Entries of the other three kinds are left
    /// alone, each belonging to its own page.
    void mergeActivationsInto(std::vector<core::PatternActivation>& activations) const;

protected:
    /// Sets the shared widgets from `code`; a subclass calls this first from
    /// its own `applySettings`, then sets what it owns on top. **Whether the
    /// task runs lives on the target page alone (Task 10), never here** — a
    /// second "enabled" checkbox on this page would be a second place to say
    /// the same thing, and the two could disagree.
    void applyBase(const std::string& code, const core::CorrectionSettings& settings);

    /// Where a subclass adds its own rows, between the code selector and the
    /// pattern list.
    [[nodiscard]] QVBoxLayout* extraLayout() const { return m_extraLayout; }

private:
    core::PatternKind m_kind;
    /// The settings the page opened on, with the pattern list's state folded
    /// in each time the code changes — so a box toggled under one code is
    /// found as it was left when that code comes back, and still reaches
    /// `mergeActivationsInto` if the run finishes on another.
    core::CorrectionSettings m_settings;
    PatternCodeSelector* m_selector;
    QVBoxLayout* m_extraLayout;
    PatternList* m_list;
};

/// GUI-HEARING-03: the two "Sound in…" checkboxes of D7, decoupled from the
/// pattern list — neither names a compiled pattern.
class MentionsPage final : public CorrectionTaskPage {
public:
    explicit MentionsPage(const core::PatternCatalogue& catalogue, QWidget* parent = nullptr);

    void applySettings(const core::CorrectionSettings& settings) override;

    [[nodiscard]] bool soundInBrackets() const;
    [[nodiscard]] bool soundInParentheses() const;

private:
    QCheckBox* m_brackets;
    QCheckBox* m_parentheses;
};

/// D4: the Human/OCR class filter.
class CommonErrorsPage final : public CorrectionTaskPage {
public:
    explicit CommonErrorsPage(const core::PatternCatalogue& catalogue, QWidget* parent = nullptr);

    void applySettings(const core::CorrectionSettings& settings) override;

    [[nodiscard]] bool human() const;
    [[nodiscard]] bool ocr() const;

private:
    QCheckBox* m_human;
    QCheckBox* m_ocr;
};

/// No extra of its own — the shared shape is the whole page.
class CapitalizationPage final : public CorrectionTaskPage {
public:
    explicit CapitalizationPage(const core::PatternCatalogue& catalogue, QWidget* parent = nullptr);

    void applySettings(const core::CorrectionSettings& settings) override;
};

/// D5: the line-break limits, in characters or in ems, and the skip gate.
class LineBreakPage final : public CorrectionTaskPage {
public:
    explicit LineBreakPage(const core::PatternCatalogue& catalogue, QWidget* parent = nullptr);

    void applySettings(const core::CorrectionSettings& settings) override;

    [[nodiscard]] double maxLength() const;
    [[nodiscard]] int maxLines() const;
    /// True for ems, false for characters — the unit chosen. `CorrectionSettings`
    /// carries the choice as a flag; the measure it names is `gui`-only (D5).
    [[nodiscard]] bool useEms() const;

    /// Gaupol's skip gate, each check its own box, both in the unit above.
    [[nodiscard]] bool skipOnLength() const;
    [[nodiscard]] double skipMaxLength() const;
    [[nodiscard]] bool skipOnLines() const;
    [[nodiscard]] int skipMaxLines() const;

private:
    QDoubleSpinBox* m_maxLength;
    QSpinBox* m_maxLines;
    QComboBox* m_unit;
    QCheckBox* m_skipOnLength;
    QDoubleSpinBox* m_skipMaxLength;
    QCheckBox* m_skipOnLines;
    QSpinBox* m_skipMaxLines;
};

} // namespace subedit::gui
