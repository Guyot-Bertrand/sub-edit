#pragma once

#include <subedit/core/format/read_error.hpp>
#include <subedit/core/format/read_result.hpp>
#include <subedit/core/format/subtitle_reader.hpp>

#include <expected>
#include <string_view>

namespace subedit::core {

/// Reads the SubViewer 2.0 format — one flavour of the `.sub` files.
///
/// **Two things it has that the first two formats did not.** A header made of
/// bracketed sections, which comes back verbatim; and a text held on a single
/// line, where `[br]` stands for the line break the file cannot write.
///
/// The layout is a header, then blocks of two lines: `HH:MM:SS.cc,HH:MM:SS.cc`
/// and the text. There is no numbering and no closing marker — a blank line
/// separates the blocks, and nothing depends on it.
///
/// **Where it parts from Gaupol.** Its reader keeps whatever it recognises and
/// says nothing about the rest; a line standing between two blocks disappears
/// without a word. Ours reports it — ADR 0008 — because a line the reader did
/// not understand is exactly what a user needs to be told about.
class SubViewer2Reader final : public SubtitleReader {

public:
    [[nodiscard]] std::expected<ReadResult, ReadError>
    read(std::string_view content) const override;
};

} // namespace subedit::core
