#include <subedit/core/wording/counts.hpp>

#include <cstddef>
#include <string>
#include <string_view>

namespace subedit::core {

std::string noticeOfCorrection(std::size_t corrected, std::size_t removed) {
    return "Edited " + std::to_string(corrected) + " and removed " + std::to_string(removed) +
           " subtitles";
}

std::string noMentionToRemove() {
    return "no mention to remove";
}

std::string noticeOfMentionsRemoved(std::size_t cleaned, std::size_t removed) {
    return countOf(cleaned, "subtitle") + " cleaned, " + std::to_string(removed) + " removed";
}

std::string shiftBeforeTheOrigin(std::size_t number) {
    return "subtitle " + std::to_string(number) +
           " would start before the origin, which no subtitle file can hold";
}

std::string noDictionaryFor(std::string_view language) {
    return "no dictionary for " + std::string{language};
}

std::string countOf(std::size_t count, std::string_view noun) {
    std::string text = std::to_string(count) + " " + std::string{noun};
    if (count != 1) {
        text += "s";
    }
    return text;
}

std::string notFound(std::string_view pattern) {
    return "\"" + std::string{pattern} + "\" not found";
}

std::string nothingToChange() {
    return "nothing to change";
}

std::string noticeOfItalics(std::size_t count, bool italic) {
    return countOf(count, "subtitle") + (italic ? " put in italics" : " taken out of italics");
}

std::string noticeOfRecase(std::size_t count) {
    return countOf(count, "subtitle") + " recased";
}

std::string noticeOfDialogueDashes(std::size_t count, bool dashed) {
    return countOf(count, "subtitle") + (dashed ? " dashed" : " undashed");
}

std::string noticeOfReplaceAll(std::size_t count) {
    // Not `countOf`, which adds an « s »: « match » takes « es ».
    return "replaced " + std::to_string(count) + (count == 1 ? " match" : " matches");
}

std::string noticeOfReplaceAll(std::size_t count, std::size_t projects) {
    return noticeOfReplaceAll(count) + " in " + std::to_string(projects) +
           (projects == 1 ? " project" : " projects");
}

std::string noticeOfSplit(std::size_t count) {
    return "split " + countOf(count, "subtitle") + " into a new project";
}

} // namespace subedit::core
