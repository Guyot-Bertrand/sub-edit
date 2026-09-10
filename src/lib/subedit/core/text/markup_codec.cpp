#include <subedit/core/text/markup.hpp>
#include <subedit/core/text/markup_codec.hpp>
#include <subedit/core/text/markup_vocabulary.hpp>

namespace subedit::core {

WritableMarkup keepWritable(const StyledText& runs, const StyleAbilities& abilities) {
    WritableMarkup kept;

    for (const StyledRun& run : runs) {
        Style writable = run.style;
        for (const StyleAttribute attribute : kStyleAttributes) {
            if (writable.carries(attribute) && !abilities.can(attribute)) {
                writable = writable.without(attribute);
                ++kept.dropped;
            }
        }
        appendRun(kept.runs, run.text, writable);
    }

    return kept;
}

} // namespace subedit::core
