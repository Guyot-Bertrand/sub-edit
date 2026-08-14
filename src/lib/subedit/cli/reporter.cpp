#include <subedit/cli/reporter.hpp>

#include <ostream>

namespace subedit::cli {

void Reporter::say(int atLeast, std::string_view line) const {
    if (m_level >= atLeast) {
        *m_errors << line << '\n';
    }
}

void Reporter::failed(std::string_view line) const {
    *m_errors << line << '\n';
}

} // namespace subedit::cli
