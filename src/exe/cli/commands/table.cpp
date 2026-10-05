#include "table.hpp"

#include <array>

#include "adjust.hpp"
#include "append.hpp"
#include "convert.hpp"
#include "correct.hpp"
#include "dialogue_dashes.hpp"
#include "framerate.hpp"
#include "hearing_impaired.hpp"
#include "inspect.hpp"
#include "italics.hpp"
#include "letter_case.hpp"
#include "replace.hpp"
#include "shift.hpp"
#include "snap.hpp"
#include "sort.hpp"
#include "split_file.hpp"
#include "transform.hpp"

namespace subedit::cli {

std::span<const Command> commands() {
    static constexpr std::array kTable{
        Command{.name = "inspect", .declare = declareInspect},
        Command{.name = "convert", .declare = declareConvert},
        Command{.name = "shift", .declare = declareShift},
        Command{.name = "transform", .declare = declareTransform},
        Command{.name = "framerate", .declare = declareFrameRate},
        Command{.name = "snap", .declare = declareSnap},
        Command{.name = "hearing-impaired", .declare = declareHearingImpaired},
        Command{.name = "adjust", .declare = declareAdjust},
        Command{.name = "replace", .declare = declareReplace},
        Command{.name = "case", .declare = declareCase},
        Command{.name = "italics", .declare = declareItalics},
        Command{.name = "dialogue-dashes", .declare = declareDialogueDashes},
        Command{.name = "sort", .declare = declareSort},
        Command{.name = "correct", .declare = declareCorrect},
        Command{.name = "append", .declare = declareAppend},
        Command{.name = "split-file", .declare = declareSplitFile},
    };
    return kTable;
}

} // namespace subedit::cli
