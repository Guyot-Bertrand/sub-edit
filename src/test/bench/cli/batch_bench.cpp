// What a batch costs, against the same work done in one file.
//
// ADR 0039 kept the loop sequential and said when that would be reopened: a
// batch large enough for the loop to show. This is the figure that says it, and
// it is taken on a real disk — what a batch adds to the work of its files is a
// walk, a directory made, and a temporary written then renamed for each one,
// which is the disk's business more than the code's.
//
// **Two shapes of the same four thousand subtitles**: two hundred files of
// twenty in a tree of depth three, and a single file. The difference is the
// price of the batch itself.

#include <subedit/cli/destination.hpp>
#include <subedit/cli/expansion.hpp>
#include <subedit/cli/reporter.hpp>
#include <subedit/cli/shifting.hpp>
#include <subedit/core/io/real_file_system.hpp>
#include <subedit/core/model/subtitle_format.hpp>
#include <subedit/core/time/duration.hpp>
#include <subedit/core/time/timestamp.hpp>

#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <optional>
#include <sstream>
#include <string>
#include <system_error>
#include <vector>

namespace {

using subedit::cli::Destination;
using subedit::cli::expandInputs;
using subedit::cli::Reporter;
using subedit::cli::shiftAll;
using subedit::core::Duration;
using subedit::core::RealFileSystem;
using subedit::core::Timestamp;

constexpr int kDirectories = 5;
constexpr int kSubdirectories = 4;
constexpr int kFilesPerLeaf = 10;
constexpr int kSubtitlesPerFile = 20;

/// A directory that removes itself, and that no other bench run can collide with.
class Workspace {

public:
    Workspace() {
        const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
        m_path =
            std::filesystem::temp_directory_path() / ("subedit-bench-" + std::to_string(stamp));
        std::filesystem::create_directories(m_path);
    }

    Workspace(const Workspace&) = delete;
    Workspace& operator=(const Workspace&) = delete;
    Workspace(Workspace&&) = delete;
    Workspace& operator=(Workspace&&) = delete;

    ~Workspace() {
        std::error_code ignored;
        std::filesystem::remove_all(m_path, ignored);
    }

    [[nodiscard]] std::filesystem::path of(const std::string& name) const { return m_path / name; }

private:
    std::filesystem::path m_path;
};

[[nodiscard]] std::string subRip(int count) {
    std::string content;
    for (int index = 0; index < count; ++index) {
        const int start = index * 2500;
        content += std::to_string(index + 1) + '\n';
        content += Timestamp::fromMilliseconds(start).format(subedit::core::DecimalMark::Comma);
        content += " --> ";
        content +=
            Timestamp::fromMilliseconds(start + 2000).format(subedit::core::DecimalMark::Comma);
        content += "\nUne réplique de longueur ordinaire,\nsur deux lignes.\n\n";
    }
    return content;
}

void write(const std::filesystem::path& path, const std::string& content) {
    std::filesystem::create_directories(path.parent_path());
    std::ofstream{path, std::ios::binary} << content;
}

} // namespace

TEST_CASE("a batch against the same work in one file", "[benchmark]") {
    const Workspace workspace;
    const std::string small = subRip(kSubtitlesPerFile);
    for (int d = 0; d < kDirectories; ++d)
        for (int e = 0; e < kSubdirectories; ++e)
            for (int f = 0; f < kFilesPerLeaf; ++f)
                write(workspace.of("tree/d" + std::to_string(d) + "/e" + std::to_string(e) + "/f" +
                                   std::to_string(f) + ".srt"),
                      small);
    write(workspace.of("single/film.srt"),
          subRip(kDirectories * kSubdirectories * kFilesPerLeaf * kSubtitlesPerFile));

    RealFileSystem files;
    std::ostringstream sink;
    const Reporter quiet{sink, 0};
    const std::string tree = workspace.of("tree").string();
    const std::string single = workspace.of("single/film.srt").string();
    const std::string out = workspace.of("out").string();

    const auto inputs = expandInputs(files, {tree}, true, out, quiet);
    REQUIRE(inputs.has_value());
    const std::size_t count = inputs->paths.size();
    REQUIRE(count == static_cast<std::size_t>(kDirectories * kSubdirectories * kFilesPerLeaf));
    const Destination batch = Destination::from("", out, false, count)->withRoots(inputs->roots);
    const Destination one = Destination::from("", out, false, 1).value();

    BENCHMARK("walk of 200 files in a tree of depth 3") {
        return expandInputs(files, {tree}, true, out, quiet);
    };

    BENCHMARK("shift of 200 files of 20 subtitles") {
        return shiftAll(
            files, inputs->paths, std::nullopt, Duration::fromMilliseconds(1'000), batch, quiet);
    };

    BENCHMARK("shift of one file of 4000 subtitles") {
        return shiftAll(
            files, {single}, std::nullopt, Duration::fromMilliseconds(1'000), one, quiet);
    };
}
