#include <subedit/core/format/deduced_ends.hpp>
#include <subedit/core/format/diagnostic.hpp>
#include <subedit/core/format/read_result.hpp>
#include <subedit/core/time/duration.hpp>

#include <cstddef>

namespace subedit::core {

namespace {

/// What the last subtitle gets, having no next one to end at.
///
/// **Arbitrary, and it stays arbitrary.** It is Gaupol's
/// `duration_seconds = 5`, kept out of iso-functionality; a cleverer rule — the
/// file's median duration, a reading speed — would be less predictable without
/// being any truer, and phase 10 brings duration fitting to whoever wants
/// better.
constexpr Duration kLastDuration = Duration::fromMilliseconds(5000);

} // namespace

void deduceEnds(ReadResult& result) {
    if (result.subtitles.empty())
        return;

    for (std::size_t index = 0; index + 1 < result.subtitles.size(); ++index)
        result.subtitles[index].end = result.subtitles[index + 1].start;
    result.subtitles.back().end = result.subtitles.back().start + kLastDuration;

    result.diagnostics.insert(result.diagnostics.begin(),
                              Diagnostic{
                                  .severity = Severity::Recovered,
                                  .line = kWholeFile,
                                  .kind = DiagnosticKind::DeducedEnds,
                                  .detail = {},
                              });
}

} // namespace subedit::core
