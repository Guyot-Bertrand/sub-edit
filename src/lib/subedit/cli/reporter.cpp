#include <subedit/cli/json.hpp>
#include <subedit/cli/reporter.hpp>

#include <ostream>
#include <utility>

namespace subedit::cli {

void Reporter::say(int atLeast, std::string_view line) const {
    if (m_level >= atLeast) {
        *m_errors << line << '\n';
    }
}

Reporter Reporter::withRecords(std::ostream& out) const {
    Reporter copy = *this;
    copy.m_records = &out;
    return copy;
}

Reporter Reporter::withTextOutput(std::ostream& out) const {
    Reporter copy = *this;
    copy.m_text = &out;
    return copy;
}

void Reporter::result(std::string_view text) const {
    if (m_text != nullptr && m_records == nullptr) {
        *m_text << text;
    }
}

Reporter Reporter::forCommand(std::string command) const {
    Reporter copy = *this;
    copy.m_command = std::move(command);
    return copy;
}

void Reporter::record(const Json& record) const {
    if (m_records != nullptr) {
        *m_records << record.dump() << '\n';
    }
}

void Reporter::failed(std::string_view line) const {
    *m_errors << line << '\n';
}

} // namespace subedit::cli
