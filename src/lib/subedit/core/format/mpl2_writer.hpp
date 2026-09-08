#pragma once

#include <subedit/core/format/subtitle_writer.hpp>

#include <string>

namespace subedit::core {

/// Writes MPL2.
///
/// One line per subtitle, no header and no separator: `[10][30]texte`. A file
/// read and written back comes out byte for byte identical, as long as its
/// positions were on a tenth — which they are, since that is all it can write.
class Mpl2Writer final : public SubtitleWriter {

public:
    [[nodiscard]] std::string write(const WriteRequest& request) const override;
};

} // namespace subedit::core
