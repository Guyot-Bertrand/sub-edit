#include <subedit/core/format/read_error.hpp>
#include <subedit/core/format/read_result.hpp>
#include <subedit/core/format/subtitle_file.hpp>
#include <subedit/core/io/real_file_system.hpp>
#include <subedit/core/model/subtitle.hpp>

#include <catch2/catch_test_macros.hpp>

#include <expected>
#include <filesystem>
#include <grid_fixtures.hpp>
#include <string>

namespace subedit::test {

namespace {

[[nodiscard]] core::ReadResult openOrFail(std::string_view name) {
    const std::filesystem::path path =
        std::filesystem::path{SUBEDIT_TEST_DATA_DIR} / "grilles" / name;

    const core::RealFileSystem files;
    const std::expected<std::string, core::FileError> bytes = files.readFile(path);
    if (!bytes.has_value())
        FAIL("fixture de grille introuvable : " + path.string());

    std::expected<core::ReadResult, core::ReadError> result = core::readSubtitles(*bytes);
    if (!result.has_value())
        FAIL("fixture de grille illisible : " + path.string());

    return *std::move(result);
}

} // namespace

core::Project gridProject(std::string_view name, core::FrameRate rate) {
    core::Project project;
    project.setSubtitles(openOrFail(name).subtitles);
    project.setFrameRate(rate);
    return project;
}

std::vector<core::Timestamp> gridStarts(std::string_view name) {
    const core::ReadResult result = openOrFail(name);

    std::vector<core::Timestamp> starts;
    starts.reserve(result.subtitles.size());
    for (const core::Subtitle& subtitle : result.subtitles)
        starts.push_back(subtitle.start);
    return starts;
}

} // namespace subedit::test
