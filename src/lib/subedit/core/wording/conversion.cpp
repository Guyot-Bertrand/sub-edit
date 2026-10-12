#include <subedit/core/analysis/grid_repair.hpp>
#include <subedit/core/format/degradation.hpp>
#include <subedit/core/i18n/messages.hpp>
#include <subedit/core/wording/analysis.hpp>
#include <subedit/core/wording/conversion.hpp>
#include <subedit/core/wording/counts.hpp>
#include <subedit/core/wording/formats.hpp>

#include <optional>
#include <string>
#include <vector>

namespace subedit::core {

std::string noticeOf(const ConversionLoss& loss, SubtitleFormat from, SubtitleFormat to) {
    if (!loss.isAny())
        return {};

    std::vector<std::string> posts;
    // **The arriving format is named on the one post where it is the news.**
    // « ends are not carried » would leave a reader wondering by what.
    if (loss.ends)
        // TRANSLATORS: %1 is the name of a subtitle format.
        posts.push_back(substitute(translate("ends are not carried by %1"), {nameOf(to)}));
    if (loss.joined > 0)
        posts.push_back(substitute(translatePlural("line breaks were joined in %1 subtitle",
                                                   "line breaks were joined in %1 subtitles",
                                                   loss.joined),
                                   {std::to_string(loss.joined)}));
    if (loss.tags > 0)
        posts.push_back(substitute(translatePlural("%1 tag dropped", "%1 tags dropped", loss.tags),
                                   {std::to_string(loss.tags)}));
    if (loss.header)
        posts.emplace_back(translate("the header was dropped"));
    // Named by what they were, not by what they are not: a reader who wrote
    // cue settings wants to hear « the WebVTT fields », not « some data ».
    if (loss.fields > 0)
        // TRANSLATORS: %1 is the name of a subtitle format, %2 a count.
        posts.push_back(substitute(translatePlural("the %1 fields of %2 subtitle were dropped",
                                                   "the %1 fields of %2 subtitles were dropped",
                                                   loss.fields),
                                   {nameOf(from), std::to_string(loss.fields)}));
    if (loss.precision > 0)
        // TRANSLATORS: %1 is a number of milliseconds.
        posts.push_back(substitute(translate("positions moved by up to %1 ms"),
                                   {std::to_string(loss.precision)}));

    std::string notice;
    for (const std::string& post : posts) {
        if (!notice.empty())
            notice += ", ";
        notice += post;
    }
    return notice;
}

std::string noticeOfPaste(std::size_t inserted,
                          const ConversionLoss& loss,
                          std::optional<SubtitleFormat> from,
                          SubtitleFormat to) {
    std::vector<std::string> posts;
    if (inserted > 0)
        posts.push_back(substitute(translatePlural("inserted %1 subtitle to fit the clipboard",
                                                   "inserted %1 subtitles to fit the clipboard",
                                                   inserted),
                                   {std::to_string(inserted)}));

    // The formats are named here, where `Save As…` has a dialog title to do it:
    // a box that says « 1 tag dropped » after a paste leaves a reader wondering
    // on the way from what to what.
    if (from.has_value()) {
        if (const std::string lost = noticeOf(loss, *from, to); !lost.empty())
            // TRANSLATORS: %1 and %2 are names of subtitle formats, %3 what was
            // lost on the way, such as « 3 tags dropped ».
            posts.push_back(substitute(translate("pasting %1 texts into %2: %3"),
                                       {nameOf(*from), nameOf(to), lost}));
    }

    std::string notice;
    for (const std::string& post : posts) {
        if (!notice.empty())
            notice += "; ";
        notice += post;
    }
    return notice;
}

std::string noticeOfAppend(std::size_t inserted,
                           const ConversionLoss& loss,
                           SubtitleFormat from,
                           SubtitleFormat to) {
    std::vector<std::string> posts;
    if (inserted > 0)
        posts.push_back(
            substitute(translatePlural("appended %1 subtitle", "appended %1 subtitles", inserted),
                       {std::to_string(inserted)}));

    // The same words as a paste of another format, `GUI-CLIP-02`: only what
    // `noticeOf` builds from the loss differs by caller, and it is built once.
    if (const std::string lost = noticeOf(loss, from, to); !lost.empty())
        // TRANSLATORS: %1 and %2 are names of subtitle formats, %3 what was lost
        // on the way, such as « 3 tags dropped ».
        posts.push_back(substitute(translate("%1 into %2: %3"), {nameOf(from), nameOf(to), lost}));

    std::string notice;
    for (const std::string& post : posts) {
        if (!notice.empty())
            notice += "; ";
        notice += post;
    }
    return notice;
}

namespace {

[[nodiscard]] std::string namedConversion(const RateConversion& conversion) {
    // TRANSLATORS: %1 and %2 are frame rates.
    return substitute(translate("converting from %1 to %2 fps"),
                      {nameOf(conversion.input), nameOf(conversion.output)});
}

} // namespace

std::string repairNoticeOf(const GridRepair& repair) {
    if (repair.outcome == RepairOutcome::Found && repair.conversion.has_value() &&
        repair.onto.has_value()) {
        // TRANSLATORS: %1 is « converting from 25 to 24 fps », %2 a frame rate,
        // %3 to %5 percentages.
        return substitute(translate("%1 puts the positions on a %2 fps grid: %3, against %4 as "
                                    "they are and %5 for the next best conversion"),
                          {namedConversion(*repair.conversion),
                           nameOf(*repair.onto),
                           percentOf(repair.concentration),
                           percentOf(repair.asIs),
                           percentOf(repair.runnerUp)});
    }
    if (repair.outcome == RepairOutcome::Tied && repair.conversion.has_value() &&
        repair.rival.has_value()) {
        // TRANSLATORS: %1 and %2 are « converting from 25 to 24 fps », %3 and %4
        // percentages.
        return substitute(translate("%1 and %2 fit equally well (%3 and %4), so none is proposed"),
                          {namedConversion(*repair.conversion),
                           namedConversion(*repair.rival),
                           percentOf(repair.concentration),
                           percentOf(repair.runnerUp)});
    }
    return {};
}

} // namespace subedit::core
