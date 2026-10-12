#include <subedit/core/i18n/messages.hpp>
#include <subedit/core/wording/counts.hpp>

#include <cstddef>
#include <string>
#include <string_view>
#include <utility>

namespace subedit::core {

std::string noticeOfCorrection(std::size_t corrected, std::size_t removed) {
    // TRANSLATORS: %1 subtitles were edited and %2 removed. English says
    // « subtitles » whatever the counts; the form is chosen by %2.
    return substitute(translatePlural("Edited %1 and removed %2 subtitles",
                                      "Edited %1 and removed %2 subtitles",
                                      removed),
                      {std::to_string(corrected), std::to_string(removed)});
}

std::string noMentionToRemove() {
    return std::string{translate("no mention to remove")};
}

std::string noticeOfMentionsRemoved(std::size_t cleaned, std::size_t removed) {
    return substitute(translatePlural("%1 subtitle cleaned, %2 removed",
                                      "%1 subtitles cleaned, %2 removed",
                                      cleaned),
                      {std::to_string(cleaned), std::to_string(removed)});
}

std::string shiftBeforeTheOrigin(std::size_t number) {
    return substitute(
        translate("subtitle %1 would start before the origin, which no subtitle file can hold"),
        {std::to_string(number)});
}

std::string noDictionaryFor(std::string_view language) {
    // TRANSLATORS: %1 is a language code, such as en_GB.
    return substitute(translate("no dictionary for %1"), {language});
}

std::string countOf(std::size_t count, Counted noun) {
    const std::string number = std::to_string(count);
    switch (noun) {
    case Counted::Subtitle:
        return substitute(translatePlural("%1 subtitle", "%1 subtitles", count), {number});
    case Counted::File:
        return substitute(translatePlural("%1 file", "%1 files", count), {number});
    case Counted::Run:
        return substitute(translatePlural("%1 run", "%1 runs", count), {number});
    }
    std::unreachable();
}

std::string notFound(std::string_view pattern) {
    // TRANSLATORS: %1 is what the user searched for.
    return substitute(translate("\"%1\" not found"), {pattern});
}

std::string nothingToChange() {
    return std::string{translate("nothing to change")};
}

std::string noticeOfItalics(std::size_t count, bool italic) {
    return substitute(
        italic ? translatePlural("%1 subtitle put in italics", "%1 subtitles put in italics", count)
               : translatePlural("%1 subtitle taken out of italics",
                                 "%1 subtitles taken out of italics",
                                 count),
        {std::to_string(count)});
}

std::string noticeOfRecase(std::size_t count) {
    return substitute(translatePlural("%1 subtitle recased", "%1 subtitles recased", count),
                      {std::to_string(count)});
}

std::string noticeOfDialogueDashes(std::size_t count, bool dashed) {
    return substitute(dashed
                          ? translatePlural("%1 subtitle dashed", "%1 subtitles dashed", count)
                          : translatePlural("%1 subtitle undashed", "%1 subtitles undashed", count),
                      {std::to_string(count)});
}

std::string noticeOfReplaceAll(std::size_t count) {
    return substitute(translatePlural("replaced %1 match", "replaced %1 matches", count),
                      {std::to_string(count)});
}

std::string noticeOfReplaceAll(std::size_t count, std::size_t projects) {
    // TRANSLATORS: %1 is « replaced 4 matches », %2 the number of projects.
    return substitute(translatePlural("%1 in %2 project", "%1 in %2 projects", projects),
                      {noticeOfReplaceAll(count), std::to_string(projects)});
}

std::string noticeOfSplit(std::size_t count) {
    return substitute(translatePlural("split %1 subtitle into a new project",
                                      "split %1 subtitles into a new project",
                                      count),
                      {std::to_string(count)});
}

std::string noticeOfSort(std::size_t moved) {
    if (moved == 0)
        return std::string{translate("already in order")};
    return substitute(translatePlural("%1 subtitle moved", "%1 subtitles moved", moved),
                      {std::to_string(moved)});
}

std::string refusalOfSplit(std::size_t cutAt, std::size_t before) {
    return substitute(translate("Cannot split at subtitle %1: subtitle %2 would fall before the "
                                "start of the video. Cut somewhere else."),
                      {std::to_string(cutAt), std::to_string(before)});
}

} // namespace subedit::core
