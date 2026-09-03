#include <subedit/cli/writing.hpp>
#include <subedit/core/format/subtitle_file.hpp>
#include <subedit/core/format/write_error.hpp>
#include <subedit/core/io/atomic_write.hpp>
#include <subedit/core/io/file_system.hpp>
#include <subedit/core/wording.hpp>

#include <expected>
#include <string>
#include <utility>

namespace subedit::cli {

std::expected<std::size_t, std::string> writeSubtitlesTo(core::FileSystem& files,
                                                         const std::filesystem::path& out,
                                                         core::SubtitleFormat format,
                                                         const core::WriteRequest& request) {
    const std::expected<std::string, core::WriteError> written =
        core::writeSubtitles(format, request);
    if (!written)
        return std::unexpected(std::string{reasonOf(written.error().kind)});

    if (const std::expected<void, core::FileError> saved =
            core::writeAtomically(files, out, *written);
        !saved)
        return std::unexpected(out.string() + ": " + std::string{reasonOf(saved.error().kind)});

    return written->size();
}

} // namespace subedit::cli
