// The records of a batch that writes: what each operation counts, and the kind
// of each way a file can come to nothing.

#include <subedit/cli/aligning.hpp>
#include <subedit/cli/conversion.hpp>
#include <subedit/cli/destination.hpp>
#include <subedit/cli/frame_rate_conversion.hpp>
#include <subedit/cli/hearing_impaired.hpp>
#include <subedit/cli/index_grammar.hpp>
#include <subedit/cli/records.hpp>
#include <subedit/cli/reporter.hpp>
#include <subedit/cli/shifting.hpp>
#include <subedit/cli/transforming.hpp>
#include <subedit/core/format/diagnostic.hpp>
#include <subedit/core/io/in_memory_file_system.hpp>
#include <subedit/core/model/encoding.hpp>
#include <subedit/core/time/duration.hpp>
#include <subedit/core/time/frame_rate.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <optional>
#include <sstream>
#include <string>
#include <vector>

using Catch::Matchers::ContainsSubstring;
using subedit::cli::alignAll;
using subedit::cli::convertAll;
using subedit::cli::convertFrameRateAll;
using subedit::cli::Destination;
using subedit::cli::ExitCode;
using subedit::cli::Failure;
using subedit::cli::Reference;
using subedit::cli::removeHearingImpairedIn;
using subedit::cli::Reporter;
using subedit::cli::shiftAll;
using subedit::cli::shiftOntoGridAll;
using subedit::cli::Transform;
using subedit::cli::transformAll;
using subedit::core::Duration;
using subedit::core::FileErrorKind;
using subedit::core::InMemoryFileSystem;
using subedit::core::SubtitleFormat;
using subedit::core::Timestamp;

namespace {

const std::string kTwo = "1\n00:00:01,000 --> 00:00:03,500\nFirst.\n\n"
                         "2\n00:00:04,000 --> 00:00:06,200\nSecond.\n";

/// What a run said: the records on one side, the narration on the other.
struct Said {
    ExitCode code;
    std::string records;
    std::string errors;
};

template<typename Run>
Said run(InMemoryFileSystem& files, const Run& body) {
    std::ostringstream errors;
    std::ostringstream records;
    const Reporter reporter = Reporter{errors, 0}.withRecords(records).forCommand("test");
    return Said{.code = body(files, reporter), .records = records.str(), .errors = errors.str()};
}

Destination out(std::size_t count = 1) {
    return Destination::from("", "out", false, count).value();
}

} // namespace

TEST_CASE("every failure kind and every warning kind has its identifier", "[cli][records]") {
    using subedit::cli::idOf;
    using subedit::core::DiagnosticKind;
    using subedit::core::ReadErrorKind;
    using subedit::core::WriteErrorKind;

    CHECK(idOf(DiagnosticKind::IgnoredLine) == "ignored-line");
    CHECK(idOf(DiagnosticKind::MalformedTimestamp) == "malformed-timestamp");
    CHECK(idOf(DiagnosticKind::MissingNumbering) == "missing-numbering");
    CHECK(idOf(DiagnosticKind::InconsistentNumbering) == "inconsistent-numbering");
    CHECK(idOf(DiagnosticKind::TextBeforeAnyTimestamp) == "text-before-any-timestamp");
    CHECK(idOf(DiagnosticKind::UnknownBlock) == "unknown-block");
    CHECK(idOf(DiagnosticKind::UnknownEventField) == "unknown-event-field");
    CHECK(idOf(DiagnosticKind::AssumedFrameRate) == "assumed-frame-rate");
    CHECK(idOf(DiagnosticKind::DeducedEnds) == "deduced-ends");
    CHECK(idOf(DiagnosticKind::MixedNewlines) == "mixed-newlines");
    CHECK(idOf(DiagnosticKind::GuessedEncoding) == "guessed-encoding");
    CHECK(idOf(DiagnosticKind::MarkOverridesEncoding) == "mark-overrides-encoding");
    CHECK(idOf(FileErrorKind::NotFound) == "not-found");
    CHECK(idOf(FileErrorKind::PermissionDenied) == "permission-denied");
    CHECK(idOf(FileErrorKind::Io) == "io");
    CHECK(idOf(ReadErrorKind::Undecodable) == "undecodable");
    CHECK(idOf(ReadErrorKind::NoSubtitleFound) == "no-subtitle-found");
    CHECK(idOf(ReadErrorKind::UnknownFormat) == "unknown-format");
    CHECK(idOf(WriteErrorKind::Unencodable) == "unencodable");
}

TEST_CASE("a failure without a kind is a refusal", "[cli][records]") {
    const Failure plain{"it did not work"};
    const Failure named{"no-grid", "nothing to bring back"};

    CHECK(plain.kind == "refused");
    CHECK(plain.message == "it did not work");
    CHECK(named.kind == "no-grid");
}

TEST_CASE("a warning carries its detail when there is one", "[cli][records]") {
    const std::string line =
        subedit::cli::warningsOf(std::vector<subedit::core::Diagnostic>{
                                     {.severity = subedit::core::Severity::Warning,
                                      .line = 7,
                                      .kind = subedit::core::DiagnosticKind::IgnoredLine,
                                      .detail = "a \"quoted\" line"}})
            .dump();

    // `settled` only when the reader decided; the detail is escaped like any text.
    CHECK(line == R"([{"kind":"ignored-line","line":7,"detail":"a \"quoted\" line"}])");
}

TEST_CASE("a shift counts its subtitles and its amount", "[cli][records]") {
    InMemoryFileSystem files;
    files.addFile("a.srt", kTwo);

    const Said said = run(files, [](InMemoryFileSystem& f, const Reporter& r) {
        return shiftAll(f, {"a.srt"}, std::nullopt, Duration::fromMilliseconds(-500), out(), r);
    });

    CHECK(said.code == ExitCode::Success);
    CHECK(said.records ==
          "{\"schema\":1,\"command\":\"test\",\"file\":\"a.srt\",\"ok\":true,\"dry_run\":false,"
          "\"destination\":\"out/a.srt\",\"counts\":{\"subtitles\":2,\"shifted_by_ms\":-500},"
          "\"warnings\":[]}\n");
}

TEST_CASE("a shift before the origin is refused with its kind", "[cli][records]") {
    InMemoryFileSystem files;
    files.addFile("a.srt", kTwo);

    const Said said = run(files, [](InMemoryFileSystem& f, const Reporter& r) {
        return shiftAll(f, {"a.srt"}, std::nullopt, Duration::fromMilliseconds(-9'000), out(), r);
    });

    CHECK(said.code == ExitCode::AllFailed);
    CHECK_THAT(said.records,
               ContainsSubstring(R"("ok":false,"error":{"kind":"before-the-origin")"));
    CHECK_THAT(said.errors, ContainsSubstring("a.srt: "));
}

TEST_CASE("a shift onto a grid that is not there is refused with its kind", "[cli][records]") {
    InMemoryFileSystem files;
    files.addFile("a.srt", kTwo);

    const Said said = run(files, [](InMemoryFileSystem& f, const Reporter& r) {
        return shiftOntoGridAll(f, {"a.srt"}, std::nullopt, out(), r);
    });

    CHECK(said.code == ExitCode::AllFailed);
    CHECK_THAT(said.records, ContainsSubstring(R"("kind":"no-grid")"));
}

TEST_CASE("a transform counts its subtitles, and refuses with a kind of its own",
          "[cli][records]") {
    const auto at = [](std::size_t number, int milliseconds) {
        return Reference{.number = number, .target = Timestamp::fromMilliseconds(milliseconds)};
    };
    const auto transformed =
        [&](const Reference& first, const Reference& last, const std::string& content) {
            InMemoryFileSystem files;
            files.addFile("a.srt", content);
            return run(files, [&](InMemoryFileSystem& f, const Reporter& r) {
                return transformAll(
                    f, {"a.srt"}, std::nullopt, Transform::between(first, last).value(), out(), r);
            });
        };

    CHECK_THAT(transformed(at(1, 1'000), at(2, 4'000), kTwo).records,
               ContainsSubstring(R"("counts":{"subtitles":2})"));
    CHECK_THAT(transformed(at(1, 1'000), at(9, 4'000), kTwo).records,
               ContainsSubstring(R"("kind":"beyond-the-end")"));
    CHECK_THAT(transformed(at(2, 1'000), at(1, 9'000), kTwo).records,
               ContainsSubstring(R"("kind":"before-the-origin")"));
    // Two subtitles that start together define no transform.
    const std::string together =
        "1\n00:00:01,000 --> 00:00:02,000\nA.\n\n2\n00:00:01,000 --> 00:00:02,000\nB.\n";
    CHECK_THAT(transformed(at(1, 1'000), at(2, 2'000), together).records,
               ContainsSubstring(R"("kind":"no-transform")"));
}

TEST_CASE("the other operations count what they did", "[cli][records]") {
    InMemoryFileSystem files;
    files.addFile("a.srt",
                  "1\n00:00:01,010 --> 00:00:02,020\n[Door] Hello.\n\n"
                  "2\n00:00:03,000 --> 00:00:04,000\nBye.\n");

    const Said align = run(files, [](InMemoryFileSystem& f, const Reporter& r) {
        return alignAll(f,
                        {"a.srt"},
                        std::nullopt,
                        subedit::core::FrameRate{subedit::core::StandardFrameRate::Fps25},
                        out(),
                        r);
    });
    CHECK_THAT(align.records, ContainsSubstring(R"("counts":{"subtitles":2,"moved":)"));
    CHECK_THAT(align.records, ContainsSubstring(R"(,"furthest_ms":)"));

    const Said retime = run(files, [](InMemoryFileSystem& f, const Reporter& r) {
        return convertFrameRateAll(
            f,
            {"a.srt"},
            std::nullopt,
            subedit::core::FrameRate{subedit::core::StandardFrameRate::Fps25},
            subedit::core::FrameRate{subedit::core::StandardFrameRate::Fps24},
            out(),
            r);
    });
    CHECK_THAT(retime.records, ContainsSubstring(R"("counts":{"subtitles":2})"));

    const Said cleaned = run(files, [](InMemoryFileSystem& f, const Reporter& r) {
        return removeHearingImpairedIn(f, {"a.srt"}, std::nullopt, out(), r);
    });
    CHECK_THAT(cleaned.records, ContainsSubstring(R"("counts":{"cleaned":1,"removed":0})"));

    InMemoryFileSystem plain;
    plain.addFile("a.srt", kTwo);
    const Said nothing = run(plain, [](InMemoryFileSystem& f, const Reporter& r) {
        return removeHearingImpairedIn(f, {"a.srt"}, std::nullopt, out(), r);
    });
    CHECK_THAT(nothing.records, ContainsSubstring(R"("counts":{"cleaned":0,"removed":0})"));
}

TEST_CASE("a conversion counts what it lost, post by post", "[cli][records]") {
    InMemoryFileSystem files;
    files.addFile("a.srt", "1\n00:00:01,000 --> 00:00:03,000\n<i>sur deux</i>\nlignes\n");

    const Said said = run(files, [](InMemoryFileSystem& f, const Reporter& r) {
        return convertAll(
            f, {"a.srt"}, subedit::core::ReadingChoices{}, SubtitleFormat::Lrc, {}, out(), r);
    });

    CHECK(said.code == ExitCode::Success);
    CHECK_THAT(
        said.records,
        ContainsSubstring(
            R"("destination":"out/a.lrc","counts":{"subtitles":1,"lost_ends":1,"joined_lines":1,"lost_tags":)"));
    CHECK_THAT(said.records,
               ContainsSubstring(R"(,"lost_header":0,"lost_fields":0,"furthest_ms":)"));
}

TEST_CASE("a conversion that cannot write says why, with a kind", "[cli][records]") {
    InMemoryFileSystem latin;
    latin.addFile("a.srt", "1\n00:00:01,000 --> 00:00:03,000\nUn caf\xE9.\n");

    const Said mark = run(latin, [](InMemoryFileSystem& f, const Reporter& r) {
        return convertAll(f,
                          {"a.srt"},
                          subedit::core::ReadingChoices{},
                          SubtitleFormat::SubRip,
                          {.bom = subedit::core::ByteOrderMark::Present},
                          out(),
                          r);
    });
    CHECK_THAT(mark.records, ContainsSubstring(R"("kind":"no-byte-order-mark")"));

    // A file counted in frames needs a rate, and these positions fall on no grid.
    InMemoryFileSystem silent;
    silent.addFile(
        "a.srt", "1\n00:00:01,013 --> 00:00:03,000\nA.\n\n2\n00:00:04,071 --> 00:00:05,000\nB.\n");
    const Said frames = run(silent, [](InMemoryFileSystem& f, const Reporter& r) {
        return convertAll(
            f, {"a.srt"}, subedit::core::ReadingChoices{}, SubtitleFormat::MicroDvd, {}, out(), r);
    });
    CHECK_THAT(frames.records, ContainsSubstring(R"("kind":"no-frame-rate")"));
}

TEST_CASE("a write that fails is a record of failure, with the kind of the refusal",
          "[cli][records]") {
    InMemoryFileSystem files;
    files.addFile("a.srt", kTwo);
    files.failNextWrite(FileErrorKind::PermissionDenied);

    const Said said = run(files, [](InMemoryFileSystem& f, const Reporter& r) {
        return shiftAll(f, {"a.srt"}, std::nullopt, Duration::fromMilliseconds(500), out(), r);
    });

    CHECK(said.code == ExitCode::AllFailed);
    CHECK_THAT(said.records,
               ContainsSubstring(R"("ok":false,"error":{"kind":"permission-denied")"));
    CHECK_THAT(said.records, ContainsSubstring("cannot be written: permission denied"));
}

TEST_CASE("a failure on a path that is not UTF-8 keeps the warning", "[cli][records]") {
    const std::string record =
        subedit::cli::failureRecord(
            "shift", "caf\xE9.srt", "caf\xE9.srt: does not exist", "not-found")
            .dump();

    CHECK_THAT(record, ContainsSubstring("\"file\":\"caf\xEF\xBF\xBD.srt\""));
    CHECK_THAT(record, ContainsSubstring(R"("warnings":[{"kind":"path-not-utf8"}]})"));
}

TEST_CASE("a reporter without records writes none, and says nothing different", "[cli][records]") {
    std::ostringstream errors;
    const Reporter plain{errors, 1};

    CHECK_FALSE(plain.recording());
    plain.record(subedit::cli::Json::object().set("a", 1));
    CHECK(errors.str().empty());

    std::ostringstream records;
    const Reporter recording = plain.withRecords(records).forCommand("shift");
    CHECK(recording.recording());
    CHECK(recording.command() == "shift");
    recording.record(subedit::cli::Json::object().set("a", 1));
    CHECK(records.str() == "{\"a\":1}\n");
    CHECK(errors.str().empty());
}

TEST_CASE("a group of counts is an object of its own", "[cli][records][CLI-ADJUST-05]") {
    const std::vector<subedit::cli::Count> counts{
        {"subtitles", 4},
        {"sacrificed", std::vector<subedit::cli::Count>{{"speed", 1}, {"gap", 0}}}};

    CHECK(subedit::cli::countsOf(counts).dump() ==
          R"({"subtitles":4,"sacrificed":{"speed":1,"gap":0}})");
}
