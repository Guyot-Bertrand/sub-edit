#include <subedit/core/text/correction_run.hpp>
#include <subedit/gui/correction_result_model.hpp>

#include <QSignalSpy>
#include <catch2/catch_test_macros.hpp>

namespace {
using subedit::core::ProposedCorrection;
using subedit::gui::CorrectionResultModel;

std::vector<ProposedCorrection> twoRows() {
    return {ProposedCorrection{.index = subedit::core::SubtitleIndex::fromValue(0),
                               .original = "Bonjour  Marie",
                               .proposed = std::string{"Bonjour Marie"}},
            ProposedCorrection{.index = subedit::core::SubtitleIndex::fromValue(1),
                               .original = "Au revoir",
                               .proposed = std::nullopt}};
}
} // namespace

TEST_CASE("every row is accepted by default", "[gui][correction-result-model]") {
    CorrectionResultModel model{twoRows()};

    CHECK(model.rowCount() == 2);
    CHECK(model.acceptedCorrections().size() == 2);
}

TEST_CASE("unchecking a row's accept box drops it from what is applied",
          "[gui][correction-result-model]") {
    CorrectionResultModel model{twoRows()};

    model.setData(model.index(0, CorrectionResultModel::Accept), Qt::Unchecked, Qt::CheckStateRole);

    const auto accepted = model.acceptedCorrections();
    REQUIRE(accepted.size() == 1);
    CHECK(accepted[0].original == "Au revoir");
}

TEST_CASE("markAll checks or unchecks every row at once", "[gui][correction-result-model]") {
    CorrectionResultModel model{twoRows()};

    model.markAll(false);
    CHECK(model.acceptedCorrections().empty());

    model.markAll(true);
    CHECK(model.acceptedCorrections().size() == 2);
}

TEST_CASE("retouching the proposed text changes what is applied",
          "[gui][correction-result-model]") {
    CorrectionResultModel model{twoRows()};

    model.setData(model.index(0, CorrectionResultModel::Proposed),
                  QStringLiteral("Bonjour, Marie"),
                  Qt::EditRole);

    const auto accepted = model.acceptedCorrections();
    REQUIRE(accepted.size() == 2);
    CHECK(accepted[0].proposed == std::optional<std::string>{"Bonjour, Marie"});
}

TEST_CASE("retouching the proposed text back to the original drops the row",
          "[gui][correction-result-model]") {
    CorrectionResultModel model{twoRows()};

    model.setData(model.index(0, CorrectionResultModel::Proposed),
                  QStringLiteral("Bonjour  Marie"), // exactly the original
                  Qt::EditRole);

    const auto accepted = model.acceptedCorrections();
    REQUIRE(accepted.size() == 1);
    CHECK(accepted[0].original == "Au revoir"); // only the second row remains
}

TEST_CASE("retouching the proposed text also refreshes the original column's diff",
          "[gui][correction-result-model]") {
    CorrectionResultModel model{twoRows()};

    // Original's diff is computed against the current proposed/retouched
    // text, so a retouch moves where "changed" falls in Original too — the
    // view must be told to repaint that cell, not just Proposed.
    const QVariant originalBefore =
        model.data(model.index(0, CorrectionResultModel::Original), Qt::DisplayRole);

    QSignalSpy spy{&model, &CorrectionResultModel::dataChanged};
    // Deliberately not "Bonjour, Marie" (test above): that retouch happens to
    // leave the same single stray space marked changed in Original as the
    // default proposal does, which would pass even without the fix. Replacing
    // "Bonjour" outright shifts what the diff marks as changed in Original.
    model.setData(model.index(0, CorrectionResultModel::Proposed),
                  QStringLiteral("Salut Marie"),
                  Qt::EditRole);

    REQUIRE(spy.count() == 1);
    const QModelIndex topLeft = spy.at(0).at(0).value<QModelIndex>();
    const QModelIndex bottomRight = spy.at(0).at(1).value<QModelIndex>();
    CHECK(topLeft.row() == 0);
    CHECK(topLeft.column() <= static_cast<int>(CorrectionResultModel::Original));
    CHECK(bottomRight.column() >= static_cast<int>(CorrectionResultModel::Original));

    const QVariant originalAfter =
        model.data(model.index(0, CorrectionResultModel::Original), Qt::DisplayRole);
    CHECK(originalBefore != originalAfter);
}
