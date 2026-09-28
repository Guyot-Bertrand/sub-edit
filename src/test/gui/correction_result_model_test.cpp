#include <subedit/core/text/correction_run.hpp>
#include <subedit/gui/correction_result_model.hpp>

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
