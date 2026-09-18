#include <subedit/core/command/change.hpp>
#include <subedit/core/edit/rewrite_texts.hpp>

#include <cstddef>

namespace subedit::core {

std::size_t rewrittenCount(const Command& command) {
    std::size_t rewritten = 0;
    for (const Change& change : command.describe())
        rewritten += change.subtitles.count();
    return rewritten;
}

} // namespace subedit::core
