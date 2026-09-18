#include <subedit/core/command/command_kind.hpp>
#include <subedit/core/command/composite_command.hpp>
#include <subedit/core/edit/clipboard.hpp>
#include <subedit/core/edit/insert_command.hpp>
#include <subedit/core/edit/rewrite_texts.hpp>
#include <subedit/core/edit/set_text_command.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/selection.hpp>
#include <subedit/core/model/source_file.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/text/markup_conversion.hpp>

#include <algorithm>
#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace subedit::core {

namespace {

/// What separates two texts in plain form: the blank line of Gaupol.
constexpr std::string_view kSeparator = "\n\n";

} // namespace

ClipboardTexts copyTexts(const Project& project, const Selection& selection, Document document) {
    ClipboardTexts copied{.texts = {}, .format = project.sourceFile().format};
    if (selection.isEmpty())
        return copied;

    const std::size_t first = selection.ranges().front().first.value();
    const std::size_t last = selection.ranges().back().last.value();
    copied.texts.reserve(last - first + 1);

    for (std::size_t value = first; value <= last; ++value) {
        const SubtitleIndex index = SubtitleIndex::fromValue(value);
        if (selection.contains(index))
            copied.texts.emplace_back(project.subtitleAt(index).text(document));
        else
            copied.texts.emplace_back(std::nullopt);
    }
    return copied;
}

std::string plainTextOf(const ClipboardTexts& clipboard) {
    std::string plain;
    for (std::size_t rank = 0; rank < clipboard.texts.size(); ++rank) {
        if (rank > 0)
            plain += kSeparator;
        plain += clipboard.texts[rank].value_or(std::string{});
    }
    return plain;
}

ClipboardTexts textsFromPlain(std::string_view plain) {
    ClipboardTexts read;
    if (plain.empty())
        return read;

    std::size_t from = 0;
    while (true) {
        const std::size_t found = plain.find(kSeparator, from);
        const std::string_view piece = plain.substr(
            from, found == std::string_view::npos ? std::string_view::npos : found - from);

        if (piece.empty())
            read.texts.emplace_back(std::nullopt);
        else
            read.texts.emplace_back(std::string{piece});

        if (found == std::string_view::npos)
            break;
        from = found + kSeparator.size();
    }
    return read;
}

std::unique_ptr<Command>
cutTexts(const Project& project, const Selection& selection, Document document) {
    return rewriteTexts(project, selection, document, CommandKind::Cut, [](const std::string&) {
        return std::string{};
    });
}

PastedTexts pasteTexts(const Project& project,
                       const ClipboardTexts& clipboard,
                       SubtitleIndex at,
                       Document document) {
    PastedTexts pasted;
    const SubtitleFormat target = project.sourceFile().format;
    const bool translates = clipboard.format.has_value() && *clipboard.format != target;

    // The texts as the document will hold them, translated once, here.
    std::vector<std::optional<std::string>> written;
    written.reserve(clipboard.texts.size());
    for (const std::optional<std::string>& text : clipboard.texts) {
        if (!text.has_value() || !translates) {
            written.push_back(text);
            continue;
        }
        ConvertedMarkup converted = convertMarkup(*text, *clipboard.format, target);
        pasted.droppedTags += converted.dropped;
        written.emplace_back(std::move(converted.text));
    }

    const std::size_t count = project.count();
    const std::size_t room = count - std::min(at.value(), count);
    pasted.inserted = written.size() > room ? written.size() - room : 0;

    std::vector<std::unique_ptr<Command>> commands;
    for (std::size_t rank = 0; rank < written.size() - pasted.inserted; ++rank) {
        const SubtitleIndex index = SubtitleIndex::fromValue(at.value() + rank);
        // Bound once: the check and the access then name the same optional,
        // which is what the analysis can follow through an indexed vector.
        std::optional<std::string>& text = written[rank];
        if (!text.has_value() || *text == project.subtitleAt(index).text(document))
            continue;
        commands.push_back(
            std::make_unique<SetTextCommand>(project, index, document, std::move(*text)));
    }

    // **The rows past the end are born with their text**, in one insertion,
    // rather than laid down blank and edited afterwards: a command captures the
    // state it undoes when it is built, and those rows do not exist yet.
    if (pasted.inserted > 0) {
        const SubtitleIndex end = SubtitleIndex::fromValue(count);
        std::vector<Subtitle> rows = InsertCommand::blankSubtitles(project, end, pasted.inserted);
        for (std::size_t rank = 0; rank < rows.size(); ++rank) {
            std::optional<std::string>& text = written[written.size() - pasted.inserted + rank];
            if (text.has_value())
                rows[rank].text(document) = std::move(*text);
        }
        commands.push_back(std::make_unique<InsertCommand>(end, std::move(rows)));
    }

    if (!commands.empty())
        pasted.command =
            std::make_unique<CompositeCommand>(CommandKind::Paste, std::move(commands));
    return pasted;
}

} // namespace subedit::core
