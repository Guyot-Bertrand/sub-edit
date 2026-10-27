#pragma once

// The alignment cases of `src/test/data/alignment/`, read, and a way to measure an answer against
// their truth.
//
// The truth is **laid down** by `src/scripts/alignment-cases.py`, never drawn from a method's
// answer: this file only reads it. What it measures is how far an engine's answer is from where
// each line is really said, which is the figure a change of method shows.

#include <subedit/core/time/timestamp.hpp>
#include <subedit/core/transcription/alignment_engine.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <optional>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace subedit::test {

/// Where a line is really said, or nothing when it is said nowhere.
struct TruePlace {
    core::Timestamp start;
    core::Timestamp end;
};

struct AlignmentCase {
    std::string name;

    /// The lines of `given.srt`, in order, and the times they were given.
    std::vector<std::string> lines;
    std::vector<TruePlace> given;

    /// For each line, where it is really said.
    std::vector<std::optional<TruePlace>> truth;

    /// What an engine would have heard.
    std::filesystem::path heard;
};

namespace detail {

[[nodiscard]] inline std::string readText(const std::filesystem::path& path) {
    std::ifstream file{path, std::ios::binary};
    std::ostringstream text;
    text << file.rdbuf();
    return text.str();
}

[[nodiscard]] inline core::Timestamp stampOf(const std::string& text) {
    const std::optional<core::Timestamp> read = core::Timestamp::parse(text);
    return read.value_or(core::Timestamp::fromMilliseconds(0));
}

} // namespace detail

/// The case called `name`: its file, its truth and where its heard words are.
[[nodiscard]] inline AlignmentCase loadAlignmentCase(const std::string& name) {
    const std::filesystem::path directory =
        std::filesystem::path{SUBEDIT_TEST_DATA_DIR} / "alignment" / name;
    AlignmentCase made{
        .name = name, .lines = {}, .given = {}, .truth = {}, .heard = directory / "heard.json"};

    // `given.srt`: blocks of a number, a range and a line, written by the script.
    std::istringstream srt{detail::readText(directory / "given.srt")};
    std::string line;
    while (std::getline(srt, line)) {
        if (line.empty() || line.find("-->") != std::string::npos ||
            std::isdigit(line.front()) != 0)
            continue;
        made.lines.push_back(line);
    }
    srt.clear();
    srt.seekg(0);
    while (std::getline(srt, line)) {
        const std::size_t arrow = line.find(" --> ");
        if (arrow == std::string::npos)
            continue;
        made.given.push_back(
            {detail::stampOf(line.substr(0, arrow)), detail::stampOf(line.substr(arrow + 5))});
    }

    std::istringstream truth{detail::readText(directory / "truth.txt")};
    while (std::getline(truth, line)) {
        if (line == "-") {
            made.truth.emplace_back(std::nullopt);
            continue;
        }
        const std::size_t space = line.find(' ');
        made.truth.emplace_back(
            TruePlace{core::Timestamp::fromMilliseconds(std::stoll(line.substr(0, space))),
                      core::Timestamp::fromMilliseconds(std::stoll(line.substr(space + 1)))});
    }
    return made;
}

/// How far an answer is from the truth of a case.
struct AlignmentQuality {
    /// For each line: how far the start of the answer is from the true start, in milliseconds —
    /// nothing for a line the truth says nowhere, or that the engine did not find.
    std::vector<std::optional<std::int64_t>> errors;

    /// Lines the truth places and the engine found.
    std::size_t found = 0;

    /// Lines the truth places and the engine did not find: **honest**, and counted, not hidden.
    std::size_t missed = 0;

    /// Lines the truth says nowhere and the engine gave a place to anyway: the one thing an
    /// alignment must not do (it never invents a position).
    std::size_t invented = 0;

    std::int64_t worst = 0;
    std::int64_t median = 0;
};

[[nodiscard]] inline AlignmentQuality measure(const std::vector<core::AlignedLine>& answer,
                                              const AlignmentCase& wanted) {
    AlignmentQuality quality;
    std::vector<std::int64_t> distances;
    for (std::size_t line = 0; line < wanted.truth.size(); ++line) {
        const bool answered = line < answer.size() && answer[line].start.has_value();
        if (!wanted.truth[line]) {
            if (answered)
                ++quality.invented;
            quality.errors.emplace_back(std::nullopt);
            continue;
        }
        if (!answered) {
            ++quality.missed;
            quality.errors.emplace_back(std::nullopt);
            continue;
        }
        ++quality.found;
        const std::int64_t distance =
            std::abs((*answer[line].start - wanted.truth[line]->start).milliseconds());
        distances.push_back(distance);
        quality.errors.emplace_back(distance);
    }
    if (!distances.empty()) {
        std::ranges::sort(distances);
        quality.worst = distances.back();
        quality.median = distances[distances.size() / 2];
    }
    return quality;
}

} // namespace subedit::test
