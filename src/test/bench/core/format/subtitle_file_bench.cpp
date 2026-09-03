// The three measurements the spec asks for, and the reference the later
// phases will be compared against: reading a file of several thousand
// subtitles, writing the same, and converting its frame rate.
//
// The document is generated rather than read from the corpus. A benchmark that
// depended on a file would measure the disk as much as the code, and a fixture
// of that size has no business in the repository — the shape is what matters,
// and the shape is reproduced here.

#include <subedit/core/format/read_error.hpp>
#include <subedit/core/format/read_result.hpp>
#include <subedit/core/format/subtitle_file.hpp>
#include <subedit/core/format/subtitle_writer.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/model/subtitle_format.hpp>
#include <subedit/core/time/timestamp.hpp>

#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_test_macros.hpp>

#include <expected>
#include <string>
#include <vector>

namespace {

using subedit::core::ReadError;
using subedit::core::ReadResult;
using subedit::core::readSubtitles;
using subedit::core::Subtitle;
using subedit::core::SubtitleFormat;
using subedit::core::Timestamp;
using subedit::core::WriteRequest;
using subedit::core::writeSubtitles;

/// Roughly a feature film: two subtitles every five seconds for three hours.
constexpr int kSubtitleCount = 4000;
constexpr int kStepMilliseconds = 2500;
constexpr int kDurationMilliseconds = 2000;

/// Builds a SubRip document of `kSubtitleCount` subtitles.
[[nodiscard]] std::string generateSubRip() {
    std::string content;
    content.reserve(static_cast<std::size_t>(kSubtitleCount) * 80);

    for (int index = 0; index < kSubtitleCount; ++index) {
        const int start = index * kStepMilliseconds;
        content += std::to_string(index + 1);
        content += '\n';
        content += Timestamp::fromMilliseconds(start).format(subedit::core::DecimalMark::Comma);
        content += " --> ";
        content += Timestamp::fromMilliseconds(start + kDurationMilliseconds)
                       .format(subedit::core::DecimalMark::Comma);
        content += '\n';
        content += "Une réplique de longueur ordinaire,\nsur deux lignes.\n\n";
    }
    return content;
}

[[nodiscard]] std::vector<Subtitle> parsedOnce(const std::string& content) {
    std::expected<ReadResult, ReadError> result = readSubtitles(content);
    if (!result.has_value())
        return {};
    return std::move(result->subtitles);
}

} // namespace

TEST_CASE("reading and writing a full-length file", "[benchmark]") {
    const std::string content = generateSubRip();
    const std::vector<Subtitle> subtitles = parsedOnce(content);

    BENCHMARK("lecture de 4000 sous-titres") {
        return readSubtitles(content);
    };

    BENCHMARK("écriture de 4000 sous-titres") {
        return writeSubtitles(SubtitleFormat::SubRip, WriteRequest{.subtitles = subtitles})
            .value_or(std::string{});
    };
}

// The frame-rate conversion of a full-length file used to be measured here, by
// the round trip through the frames. Phase 2 decided against that path — ADR
// 0013 — so the measurement was of an operation that does not exist. It moved
// to `core/edit/operations_bench.cpp`, where it goes through the command that
// was actually written.
