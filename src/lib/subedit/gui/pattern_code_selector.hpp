#pragma once

// The script/language/country widget of a task page — issue #505, task 7.

#include <subedit/core/text/correction_pattern.hpp>

#include <QWidget>

#include <string>
#include <string_view>

class QComboBox;

namespace subedit::core {
class PatternCatalogue;
} // namespace subedit::core

namespace subedit::gui {

/// The script, language and country a task page's patterns are chosen for —
/// three combos cascading over what `catalogue` carries for `kind`, never
/// over an external locale list (Task 5).
class PatternCodeSelector final : public QWidget {
    Q_OBJECT

public:
    PatternCodeSelector(const core::PatternCatalogue& catalogue,
                        core::PatternKind kind,
                        QWidget* parent = nullptr);

    [[nodiscard]] std::string code() const;
    void setCode(std::string_view code);

signals:
    void codeChanged();

private:
    void refreshLanguages();
    void refreshCountries();

    const core::PatternCatalogue* m_catalogue;
    core::PatternKind m_kind;
    QComboBox* m_script;
    QComboBox* m_language;
    QComboBox* m_country;
};

} // namespace subedit::gui
