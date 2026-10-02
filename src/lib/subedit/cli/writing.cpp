#include <subedit/cli/writing.hpp>
#include <subedit/core/format/subtitle_file.hpp>
#include <subedit/core/format/write_error.hpp>
#include <subedit/core/io/atomic_write.hpp>
#include <subedit/core/io/file_system.hpp>
#include <subedit/core/wording/formats.hpp>

#include <expected>
#include <string>
#include <utility>

namespace subedit::cli {

std::expected<std::size_t, Failure> writeSubtitlesTo(core::FileSystem& files,
                                                     const std::filesystem::path& out,
                                                     core::SubtitleFormat format,
                                                     const core::WriteRequest& request,
                                                     bool dryRun) {
    const std::expected<std::string, core::WriteError> written =
        core::writeSubtitles(format, request);
    if (!written)
        return std::unexpected(
            Failure{idOf(written.error().kind), std::string{reasonOf(written.error().kind)}});

    if (dryRun)
        return written->size();

    if (const std::expected<void, core::FileError> saved =
            core::writeAtomically(files, out, *written);
        !saved)
        return std::unexpected(
            Failure{idOf(saved.error().kind),
                    out.string() + ": " + std::string{reasonOfWriting(saved.error().kind)}});

    return written->size();
}

} // namespace subedit::cli
