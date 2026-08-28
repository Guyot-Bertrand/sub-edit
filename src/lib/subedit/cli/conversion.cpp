#include <subedit/cli/batch.hpp>
#include <subedit/cli/conversion.hpp>
#include <subedit/cli/destination.hpp>
#include <subedit/cli/diagnostics.hpp>
#include <subedit/cli/reporter.hpp>
#include <subedit/core/format/open_error.hpp>
#include <subedit/core/format/project_file.hpp>
#include <subedit/core/format/read_error.hpp>
#include <subedit/core/io/atomic_write.hpp>
#include <subedit/core/io/file_system.hpp>
#include <subedit/core/wording.hpp>

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <expected>
#include <filesystem>
#include <string>

namespace subedit::cli {

namespace {

using core::SubtitleFormat;

/// Converts one file. Returns true when it was written.
bool convertFile(core::FileSystem& files,
                 const std::string& path,
                 SubtitleFormat target,
                 const WriteShape& shape,
                 const Destination& destination,
                 const Reporter& reporter) {
    const std::expected<core::OpenedFile, core::OpenError> opened = core::openProject(files, path);
    if (!opened) {
        reporter.failed(path + ": " + std::string{reasonOf(opened.error())});
        return false;
    }

    const core::SourceFile& source = opened->project.sourceFile();

    // Empty means "as the source had it": the model kept both so that a
    // conversion would not throw them away.
    const core::Newline newline = shape.newline.value_or(source.newline);
    const core::Utf8Bom bom =
        shape.bom.value_or(source.hadUtf8Bom ? core::Utf8Bom::Present : core::Utf8Bom::Absent);

    const core::WriteRequest request{
        .subtitles = opened->project.subtitles(),
        .document = core::Document::Main,
        .newline = newline,
        .header = source.header,
    };
    const std::string written = core::writeSubtitles(target, request, bom);

    const std::filesystem::path out = destination.pathFor(path, extensionOf(target));
    const std::expected<void, core::FileError> saved = core::writeAtomically(files, out, written);
    if (!saved) {
        reporter.failed(path + ": " + out.string() + ": " +
                        std::string{reasonOf(saved.error().kind)});
        return false;
    }

    reporter.say(3,
                 path + ": " + std::to_string(opened->bytes) + " bytes read, " +
                     std::to_string(written.size()) + " written");
    sayDiagnostics(reporter, path, opened->diagnostics);
    reporter.say(2,
                 path + ": " + std::string{nameOf(source.format)} + " -> " +
                     std::string{nameOf(target)} + ", UTF-8, " +
                     (bom == core::Utf8Bom::Present ? "BOM" : "no BOM") + ", " +
                     std::string{nameOf(newline)} + " line endings");
    reporter.say(1,
                 path + ": " + core::countOf(opened->project.subtitles().size(), "subtitle") +
                     " written as " + std::string{nameOf(target)} + " -> " + out.string());
    return true;
}

} // namespace

bool wouldMisname(const std::vector<std::string>& paths, SubtitleFormat target) {
    const std::string_view wanted = extensionOf(target);
    return std::ranges::any_of(paths, [wanted](const std::string& path) {
        std::string extension = std::filesystem::path{path}.extension().string();
        std::ranges::transform(extension, extension.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });
        return extension != wanted;
    });
}

ExitCode convertAll(core::FileSystem& files,
                    const std::vector<std::string>& paths,
                    SubtitleFormat target,
                    const WriteShape& shape,
                    const Destination& destination,
                    const Reporter& reporter) {
    std::size_t done = 0;
    for (const std::string& path : paths) {
        if (convertFile(files, path, target, shape, destination, reporter)) {
            ++done;
        }
    }
    return tally(reporter, "converted", done, paths.size());
}

} // namespace subedit::cli
