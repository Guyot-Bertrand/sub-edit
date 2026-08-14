#include <subedit/cli/destination.hpp>

namespace subedit::cli {

std::expected<Destination, std::string> Destination::from(std::string_view output,
                                                          std::string_view outputDir,
                                                          bool inPlace,
                                                          std::size_t inputCount) {
    const int given = static_cast<int>(!output.empty()) + static_cast<int>(!outputDir.empty()) +
                      static_cast<int>(inPlace);

    if (given == 0) {
        return std::unexpected{
            std::string{"no destination given: use --output, --output-dir or --in-place"}};
    }
    if (given > 1) {
        return std::unexpected{
            std::string{"--output, --output-dir and --in-place exclude one another"}};
    }
    if (!output.empty() && inputCount > 1) {
        return std::unexpected{std::string{
            "--output names one file but several were given: use --output-dir instead"}};
    }

    Destination destination;
    destination.m_output = output;
    destination.m_outputDir = outputDir;
    destination.m_inPlace = inPlace;
    return destination;
}

std::filesystem::path Destination::pathFor(const std::filesystem::path& input,
                                           std::string_view extension) const {
    if (m_inPlace) {
        return input;
    }
    if (!m_output.empty()) {
        return m_output;
    }

    std::filesystem::path name = input.filename();
    if (!extension.empty()) {
        name = input.stem();
        name += extension;
    }
    return m_outputDir / name;
}

} // namespace subedit::cli
