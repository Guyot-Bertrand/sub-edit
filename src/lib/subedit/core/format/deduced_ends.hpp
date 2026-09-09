#pragma once

#include <subedit/core/format/read_result.hpp>

namespace subedit::core {

/// Gives the subtitles of an endless format the ends it never carried, and
/// says that it happened — ADR 0029.
///
/// **Two of the nine hold one position per line.** LRC writes
/// `[00:12.34]text`, TMPlayer writes `00:00:12:text`: the start of a line and
/// nothing else. The end of a subtitle is therefore the start of the next one,
/// and the last one is given five seconds. It is Gaupol's rule, the same line
/// in `lrc.py` and in `tmplayer.py`.
///
/// **The diagnostic is the whole of what we add to Gaupol, which deduces in
/// silence.** A user looking at a filled End column has nothing telling them
/// that not one of those values came from their file; correcting one would be
/// correcting an invention.
///
/// It goes at the head of the list, because it is about the file rather than
/// about a place in it — like the guessed encoding and the assumed frame rate
/// before it.
///
/// Does nothing to a reading that found no subtitle: there is no end to work
/// out, and the reader is about to refuse the file anyway.
void deduceEnds(ReadResult& result);

} // namespace subedit::core
