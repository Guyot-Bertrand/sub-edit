#include <subedit/core/io/temporary_directory.hpp>

#include <cstdlib>
#include <string>
#include <system_error>

namespace subedit::core {

TemporaryDirectory::TemporaryDirectory(const char* stem) {
    std::error_code error;
    const std::filesystem::path parent = std::filesystem::temp_directory_path(error);
    if (error)
        return;
    std::string pattern = (parent / (std::string{stem} + "-XXXXXX")).string();
    if (::mkdtemp(pattern.data()) != nullptr)
        m_path = pattern;
}

TemporaryDirectory::~TemporaryDirectory() {
    std::error_code ignored;
    if (!m_path.empty())
        std::filesystem::remove_all(m_path, ignored);
}

} // namespace subedit::core
