// The shared wording of the core follows the installed language — issue #662.
//
// One case per family of `core/wording/`, under the Russian fixture `wording.po`: a sentence
// is looked up in the catalogue, its arguments land where the translation puts them, and a
// count picks one of three forms. Everything else in `wording_test.cpp` runs in English, with
// no catalogue, and says the same words it said before the sentences were wrapped.

#include <subedit/core/analysis/anomaly.hpp>
#include <subedit/core/analysis/grid_verdict.hpp>
#include <subedit/core/config/theme.hpp>
#include <subedit/core/edit/join_repeats.hpp>
#include <subedit/core/edit/search.hpp>
#include <subedit/core/edit/translation.hpp>
#include <subedit/core/format/degradation.hpp>
#include <subedit/core/i18n/messages.hpp>
#include <subedit/core/io/file_system.hpp>
#include <subedit/core/model/encoding.hpp>
#include <subedit/core/model/subtitle_format.hpp>
#include <subedit/core/model/subtitle_index.hpp>
#include <subedit/core/text/common_errors.hpp>
#include <subedit/core/text/pattern_catalogue.hpp>
#include <subedit/core/time/frame_rate.hpp>
#include <subedit/core/wording/analysis.hpp>
#include <subedit/core/wording/conversion.hpp>
#include <subedit/core/wording/correction.hpp>
#include <subedit/core/wording/counts.hpp>
#include <subedit/core/wording/editing.hpp>
#include <subedit/core/wording/formats.hpp>
#include <subedit/core/wording/settings.hpp>
#include <subedit/core/wording/translation.hpp>
#include <subedit/core/wording/video.hpp>

#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <utility>

using namespace subedit::core;

namespace {

/// Installs the Russian fixture for the length of a test, and puts English back afterwards,
/// whatever happened in it.
class InRussian {
public:
    InRussian() {
        auto catalogue =
            Catalogue::fromFile(std::string(SUBEDIT_COMPILED_I18N_DIR) + "/wording.mo");
        REQUIRE(catalogue.has_value());
        installCatalogue(std::make_shared<const Catalogue>(std::move(*catalogue)));
    }

    InRussian(const InRussian&) = delete;
    InRussian& operator=(const InRussian&) = delete;
    InRussian(InRussian&&) = delete;
    InRussian& operator=(InRussian&&) = delete;

    ~InRussian() { installCatalogue(nullptr); }
};

} // namespace

TEST_CASE("the analysis family speaks the installed language", "[wording][i18n]") {
    const InRussian russian;
    CHECK(nameOf(AnomalyKind::EndBeforeStart) == "заканчивается раньше, чем начинается");
    CHECK(statementOf(Anomaly{.kind = AnomalyKind::EndBeforeStart,
                              .index = SubtitleIndex::fromNumber(4)}) ==
          "субтитр 4 заканчивается раньше, чем начинается");
    // The translation puts the verdict before the rate.
    CHECK(gridStatusOf(GridVerdict::Partial, FrameRate{StandardFrameRate::Fps25}) ==
          "Сетка (частичная): 25 кадр/с");
}

TEST_CASE("the conversion family speaks the installed language", "[wording][i18n]") {
    const InRussian russian;
    CHECK(noticeOf(ConversionLoss{.tags = 1}, SubtitleFormat::SubRip, SubtitleFormat::WebVtt) ==
          "удалён 1 тег");
    CHECK(noticeOf(ConversionLoss{.tags = 3}, SubtitleFormat::SubRip, SubtitleFormat::WebVtt) ==
          "удалено 3 тега");
    CHECK(noticeOf(ConversionLoss{.tags = 5}, SubtitleFormat::SubRip, SubtitleFormat::WebVtt) ==
          "удалено 5 тегов");
    // Format names are never translated (spec 15, D3).
    CHECK(noticeOfAppend(0,
                         ConversionLoss{.tags = 2},
                         SubtitleFormat::AdvancedSubStationAlpha,
                         SubtitleFormat::SubRip) == "Advanced SSA в SubRip: удалено 2 тега");
}

TEST_CASE("the correction family speaks the installed language", "[wording][i18n]") {
    const InRussian russian;
    CHECK(reasonOf(FailureKind::TimedOut) == "превышено время ожидания");
    CHECK(describe(PatternDiagnostic{.problem = PatternProblem::MalformedLine,
                                     .file = "/share/patterns/Zyyy.common-error",
                                     .line = 4,
                                     .detail = {}}) ==
          "Zyyy.common-error, строка 4 (неверная строка)");
}

TEST_CASE("the counts family speaks the installed language, in three forms", "[wording][i18n]") {
    const InRussian russian;
    CHECK(noticeOfSort(0) == "уже по порядку");
    CHECK(noticeOfSort(1) == "перемещён 1 субтитр");
    CHECK(noticeOfSort(3) == "перемещено 3 субтитра");
    CHECK(noticeOfSort(5) == "перемещено 5 субтитров");
    CHECK(noticeOfSort(21) == "перемещён 21 субтитр");
    CHECK(countOf(2, Counted::File) == "2 файла");
    CHECK(countOf(11, Counted::File) == "11 файлов");
}

TEST_CASE("the editing family speaks the installed language", "[wording][i18n]") {
    const InRussian russian;
    CHECK(reasonOf(PatternError{.kind = PatternError::Kind::Empty}) == "нечего искать");
    CHECK(noticeOfJoin(JoinReport{.chains = 1, .joined = 1}) ==
          "объединён 1 повторяющийся субтитр");
    CHECK(noticeOfJoin(JoinReport{.chains = 1, .joined = 4}) ==
          "объединено 4 повторяющихся субтитра");
    CHECK(noticeOfJoin(JoinReport{.chains = 3, .joined = 12}) ==
          "объединено 12 повторяющихся субтитров");
}

TEST_CASE("the formats family speaks the installed language", "[wording][i18n]") {
    const InRussian russian;
    CHECK(reasonOf(FileErrorKind::NotFound) == "не существует");
    CHECK(encodingStatusOf(Encoding::utf8(ByteOrderMark::Absent)) == "Кодировка: UTF-8, без BOM");
    // What the catalogue lacks stays English.
    CHECK(encodingStatusOf(Encoding::utf8(ByteOrderMark::Present)) == "Кодировка: UTF-8, BOM");
}

TEST_CASE("the settings family speaks the installed language", "[wording][i18n]") {
    const InRussian russian;
    CHECK(nameOf(Theme::Dark) == "Тёмная");
    CHECK(settingsNotWritten() == "не удалось записать настройки");
}

TEST_CASE("the translation family speaks the installed language", "[wording][i18n]") {
    const InRussian russian;
    CHECK(noticeOf(TranslationOutcome{.attached = 1}) == "перевод: 1 строка привязана");
    CHECK(noticeOf(TranslationOutcome{.attached = 22, .outOfOrder = 5}) ==
          "перевод: 22 строки привязаны; 5 строк не по порядку");
}

TEST_CASE("the video family speaks the installed language", "[wording][i18n]") {
    const InRussian russian;
    const std::filesystem::path film{"/films/le-canot.mkv"};
    CHECK(videoStatusOf(std::nullopt) == "Нет видео");
    // The translation puts the rate before the film.
    CHECK(videoStatusOf(film, FrameRate{StandardFrameRate::Fps25}) ==
          "Видео (25 кадр/с): le-canot.mkv");
}
