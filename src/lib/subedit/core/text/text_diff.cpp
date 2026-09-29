#include <subedit/core/text/text_diff.hpp>

#include <algorithm>
#include <cstddef>

namespace subedit::core {

namespace {

/// `text` split into its codepoints, each kept as the UTF-8 bytes it was
/// written in — spans are rebuilt by joining runs of these, never by slicing
/// a multi-byte sequence in two. A malformed lead byte is read as one byte,
/// the same width `decodeToUtf8` would already have refused upstream.
[[nodiscard]] std::vector<std::string_view> codepointsOf(std::string_view text) {
    // The UTF-8 lead byte announces how many bytes follow it, in its own
    // high bits: `110xxxxx` for two, `1110xxxx` for three, `11110xxx` for
    // four — the mask keeps exactly as many high bits as the pattern names.
    constexpr unsigned kTwoByteMask = 0xE0U;
    constexpr unsigned kTwoByteLead = 0xC0U;
    constexpr unsigned kThreeByteMask = 0xF0U;
    constexpr unsigned kThreeByteLead = 0xE0U;
    constexpr unsigned kFourByteMask = 0xF8U;
    constexpr unsigned kFourByteLead = 0xF0U;

    std::vector<std::string_view> points;
    std::size_t i = 0;
    while (i < text.size()) {
        const auto lead = static_cast<unsigned char>(text[i]);
        std::size_t length = 1;
        if ((lead & kTwoByteMask) == kTwoByteLead)
            length = 2;
        else if ((lead & kThreeByteMask) == kThreeByteLead)
            length = 3;
        else if ((lead & kFourByteMask) == kFourByteLead)
            length = 4;
        length = std::min(length, text.size() - i);
        points.push_back(text.substr(i, length));
        i += length;
    }
    return points;
}

/// `points[at]`, joined back into whole spans of matched and changed runs.
[[nodiscard]] std::vector<DiffSpan> spansOf(const std::vector<std::string_view>& points,
                                            const std::vector<bool>& changedAt) {
    std::vector<DiffSpan> spans;
    std::size_t start = 0;
    while (start < points.size()) {
        std::size_t end = start;
        while (end < points.size() && changedAt[end] == changedAt[start])
            ++end;
        std::string text;
        for (std::size_t i = start; i < end; ++i)
            text += points[i];
        spans.push_back(DiffSpan{.text = std::move(text), .changed = changedAt[start]});
        start = end;
    }
    return spans;
}

} // namespace

TextDiff diffTexts(std::string_view original, std::string_view proposed) {
    const std::vector<std::string_view> left = codepointsOf(original);
    const std::vector<std::string_view> right = codepointsOf(proposed);

    // Longest common subsequence by dynamic programming — a subtitle line is
    // a handful of words, so the O(n*m) table costs nothing here.
    const std::size_t rows = left.size() + 1;
    const std::size_t cols = right.size() + 1;
    std::vector<std::vector<int>> lcs(rows, std::vector<int>(cols, 0));
    for (std::size_t i = 1; i < rows; ++i) {
        for (std::size_t j = 1; j < cols; ++j) {
            lcs[i][j] = left[i - 1] == right[j - 1] ? lcs[i - 1][j - 1] + 1
                                                    : std::max(lcs[i - 1][j], lcs[i][j - 1]);
        }
    }

    std::vector<bool> leftChanged(left.size(), true);
    std::vector<bool> rightChanged(right.size(), true);
    std::size_t i = left.size();
    std::size_t j = right.size();
    while (i > 0 && j > 0) {
        if (left[i - 1] == right[j - 1]) {
            leftChanged[i - 1] = false;
            rightChanged[j - 1] = false;
            --i;
            --j;
        } else if (lcs[i - 1][j] >= lcs[i][j - 1]) {
            --i;
        } else {
            --j;
        }
    }

    return TextDiff{.original = spansOf(left, leftChanged),
                    .proposed = spansOf(right, rightChanged)};
}

} // namespace subedit::core
