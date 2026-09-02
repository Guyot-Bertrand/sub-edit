#include <subedit/core/format/diagnostic.hpp>
#include <subedit/core/format/read_error.hpp>
#include <subedit/core/format/read_result.hpp>
#include <subedit/core/format/subtitle_reader.hpp>
#include <subedit/core/model/encoding.hpp>
#include <subedit/core/model/source_file.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/model/subtitle_format.hpp>

#include <catch2/catch_test_macros.hpp>

#include <expected>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace {

using subedit::core::ByteOrderMark;
using subedit::core::Diagnostic;
using subedit::core::DiagnosticKind;
using subedit::core::Encoding;
using subedit::core::Newline;
using subedit::core::ReadError;
using subedit::core::ReadErrorKind;
using subedit::core::ReadResult;
using subedit::core::Severity;
using subedit::core::SourceFile;
using subedit::core::Subtitle;
using subedit::core::SubtitleFormat;
using subedit::core::SubtitleReader;

/// A reader that answers according to what it is handed, to show the contract
/// without waiting for a real format.
class ContrivedReader final : public SubtitleReader {

public:
    [[nodiscard]] std::expected<ReadResult, ReadError>
    read(std::string_view content) const override {
        if (content.empty())
            return std::unexpected(ReadError{
                .kind = ReadErrorKind::NoSubtitleFound,
                .detail = "aucun bloc reconnaissable",
            });

        ReadResult result;
        result.subtitles.push_back(Subtitle{.mainText = std::string{content}});
        if (content.front() == '?')
            result.diagnostics.push_back(Diagnostic{
                .severity = Severity::Warning,
                .line = 1,
                .kind = DiagnosticKind::IgnoredLine,
                .detail = {},
            });
        return result;
    }
};

} // namespace

TEST_CASE("a fresh read result is empty and assumes nothing", "[format][read]") {
    const ReadResult result;

    CHECK(result.subtitles.empty());
    CHECK(result.header.empty());
    CHECK(result.newline == Newline::Lf);
    CHECK(result.encoding == Encoding::utf8(ByteOrderMark::Absent));
    CHECK(result.diagnostics.empty());
}

TEST_CASE("a diagnostic says where and what, without a sentence", "[format][diagnostic]") {
    // A category rather than a message: it can be translated, and a test can
    // assert on it without comparing prose that will be reworded.
    const Diagnostic diagnostic{
        .severity = Severity::Recovered,
        .line = 12,
        .kind = DiagnosticKind::MissingNumbering,
        .detail = "bloc 3",
    };

    CHECK(diagnostic.severity == Severity::Recovered);
    CHECK(diagnostic.line == 12);
    CHECK(diagnostic.kind == DiagnosticKind::MissingNumbering);
    CHECK(diagnostic.detail == "bloc 3");
}

TEST_CASE("a line number is counted from one, as an editor shows it", "[format][diagnostic]") {
    const Diagnostic diagnostic{.severity = Severity::Warning,
                                .line = 1,
                                .kind = DiagnosticKind::TextBeforeAnyTimestamp,
                                .detail = {}};

    CHECK(diagnostic.line == 1);
}

TEST_CASE("what can be read is returned, with its diagnostics", "[format][read]") {
    // ADR 0008: an anomaly the reader recovers from does not stop the reading.
    const ContrivedReader reader;

    const std::expected<ReadResult, ReadError> result = reader.read("?du texte");

    REQUIRE(result.has_value());
    CHECK(result->subtitles.size() == 1);
    REQUIRE(result->diagnostics.size() == 1);
    CHECK(result->diagnostics.front().kind == DiagnosticKind::IgnoredLine);
}

TEST_CASE("what cannot be read at all is a failure, not a diagnostic", "[format][read]") {
    // The strict line of ADR 0008: nothing recognisable is a failure; anything
    // recovered from is a diagnostic. Returning an empty result with a warning
    // would let the caller silently lose a file.
    const ContrivedReader reader;

    const std::expected<ReadResult, ReadError> result = reader.read("");

    REQUIRE_FALSE(result.has_value());
    CHECK(result.error().kind == ReadErrorKind::NoSubtitleFound);
    CHECK(result.error().detail == "aucun bloc reconnaissable");
}

TEST_CASE("a source file records the shape the reader saw", "[format][read]") {
    // What a project has to remember to write itself back out unchanged: the
    // format among the rest. Before ADR 0018 it was the one thing a SourceFile
    // did not carry, and a window that opens then saves needs it.
    const ReadResult result{
        .format = SubtitleFormat::WebVtt,
        .header = "WEBVTT - le titre",
        .newline = Newline::CrLf,
        .encoding = Encoding::utf8(ByteOrderMark::Present),
    };

    const SourceFile source = sourceFileOf(result, "quelque/part/film.vtt");

    CHECK(source.format == SubtitleFormat::WebVtt);
    CHECK(source.header == "WEBVTT - le titre");
    CHECK(source.newline == Newline::CrLf);
    CHECK(source.encoding == Encoding::utf8(ByteOrderMark::Present));
    // L'option entière plutôt que son contenu : clang-tidy ne reconnaît pas le
    // REQUIRE de Catch2 comme une vérification, et comparer l'option se passe
    // de tout accès — en affirmant davantage, au passage.
    CHECK(source.path == std::filesystem::path{"quelque/part/film.vtt"});
}

TEST_CASE("a source file with no path is one that was never on disk", "[format][read]") {
    const SourceFile source = sourceFileOf(ReadResult{}, {});

    CHECK_FALSE(source.path.has_value());
}
