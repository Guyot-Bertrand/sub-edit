#include <subedit/core/format/file_system.hpp>
#include <subedit/core/format/real_file_system.hpp>

#include <expected>
#include <filesystem>
#include <fstream>
#include <ios>
#include <iterator>
#include <string>
#include <string_view>
#include <system_error>

namespace subedit::core {

namespace {

[[nodiscard]] std::unexpected<FileError>
failure(FileErrorKind kind, const std::filesystem::path& path, std::string_view reason) {
    return std::unexpected(FileError{
        .kind = kind,
        .detail = path.string() + " : " + std::string{reason},
    });
}

[[nodiscard]] std::unexpected<FileError> failure(const std::error_code& code,
                                                 const std::filesystem::path& path) {
    return failure(fileErrorKindOf(code), path, code.message());
}

} // namespace

bool RealFileSystem::exists(const std::filesystem::path& path) const {
    std::error_code code;
    // The non-throwing overload: the absence of a file is an answer, not an
    // incident.
    return std::filesystem::exists(path, code) && !code;
}

std::expected<std::string, FileError>
RealFileSystem::readFile(const std::filesystem::path& path) const {
    std::ifstream file{path, std::ios::binary};
    if (!file)
        return failure(exists(path) ? FileErrorKind::Io : FileErrorKind::NotFound,
                       path,
                       "ouverture impossible");

    std::string content{std::istreambuf_iterator<char>{file}, std::istreambuf_iterator<char>{}};
    if (file.bad())
        return failure(FileErrorKind::Io, path, "lecture interrompue");

    return content;
}

std::expected<void, FileError> RealFileSystem::writeFile(const std::filesystem::path& path,
                                                         std::string_view content) {
    std::ofstream file{path, std::ios::binary | std::ios::trunc};
    if (!file)
        return failure(FileErrorKind::Io, path, "ouverture en écriture impossible");

    file.write(content.data(), static_cast<std::streamsize>(content.size()));
    file.close();
    if (!file)
        return failure(FileErrorKind::Io, path, "écriture interrompue");

    return {};
}

std::expected<void, FileError> RealFileSystem::rename(const std::filesystem::path& from,
                                                      const std::filesystem::path& to) {
    std::error_code code;
    std::filesystem::rename(from, to, code);
    if (code)
        return failure(code, from);
    return {};
}

std::expected<void, FileError> RealFileSystem::remove(const std::filesystem::path& path) {
    std::error_code code;
    if (!std::filesystem::remove(path, code)) {
        if (code)
            return failure(code, path);
        return failure(FileErrorKind::NotFound, path, "fichier absent");
    }
    return {};
}

} // namespace subedit::core
