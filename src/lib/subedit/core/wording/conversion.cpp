#include <subedit/core/format/degradation.hpp>
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
        posts.emplace_back("ends are not carried by " + std::string{nameOf(to)});
    if (loss.joined > 0)
        posts.emplace_back("line breaks were joined in " + countOf(loss.joined, "subtitle"));
    if (loss.tags > 0)
        posts.emplace_back(countOf(loss.tags, "tag") + " dropped");
    if (loss.header)
        posts.emplace_back("the header was dropped");
    // Named by what they were, not by what they are not: a reader who wrote
    // cue settings wants to hear « the WebVTT fields », not « some data ».
    if (loss.fields > 0)
        posts.emplace_back("the " + std::string{nameOf(from)} + " fields of " +
                           countOf(loss.fields, "subtitle") + " were dropped");
    if (loss.precision > 0)
        posts.emplace_back("positions moved by up to " + std::to_string(loss.precision) + " ms");

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
        posts.emplace_back("inserted " + countOf(inserted, "subtitle") + " to fit the clipboard");

    // The formats are named here, where `Save As…` has a dialog title to do it:
    // a box that says « 1 tag dropped » after a paste leaves a reader wondering
    // on the way from what to what.
    if (from.has_value()) {
        if (const std::string lost = noticeOf(loss, *from, to); !lost.empty())
            posts.emplace_back("pasting " + std::string{nameOf(*from)} + " texts into " +
                               std::string{nameOf(to)} + ": " + lost);
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
        posts.emplace_back("appended " + countOf(inserted, "subtitle"));

    // The same words as a paste of another format, `GUI-CLIP-02`: only what
    // `noticeOf` builds from the loss differs by caller, and it is built once.
    if (const std::string lost = noticeOf(loss, from, to); !lost.empty())
        posts.emplace_back(std::string{nameOf(from)} + " into " + std::string{nameOf(to)} + ": " +
                           lost);

    std::string notice;
    for (const std::string& post : posts) {
        if (!notice.empty())
            notice += "; ";
        notice += post;
    }
    return notice;
}

} // namespace subedit::core
