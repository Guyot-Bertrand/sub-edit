#include <subedit/cli/destination.hpp>
#include <subedit/core/io/file_system.hpp>

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <system_error>
#include <unordered_map>
#include <utility>

namespace subedit::cli {

namespace {

/// A path as a comparison sees it: absolute and without `.` or `..`.
[[nodiscard]] std::filesystem::path spelledAbsolute(const std::filesystem::path& path) {
    std::error_code code;
    const std::filesystem::path absolute = std::filesystem::absolute(path, code);
    return (code ? path : absolute).lexically_normal();
}

/// What two names of one file have in common even on a case-insensitive system.
///
/// Comparing every destination with every input would be quadratic in the size
/// of the batch; two names of one file share this key, and a bucket holds the
/// handful that do.
[[nodiscard]] std::string bucketOf(const std::filesystem::path& path) {
    std::string key = path.filename().string();
    std::ranges::transform(
        key, key.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return key;
}

} // namespace

bool sameFile(const core::FileSystem& files,
              const std::filesystem::path& first,
              const std::filesystem::path& second) {
    return spelledAbsolute(first) == spelledAbsolute(second) || files.equivalent(first, second);
}

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

Destination Destination::withRoots(std::vector<std::filesystem::path> roots) const {
    Destination destination = *this;
    destination.m_roots = std::move(roots);
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

    // Below a root, the path from it; anywhere else, the name alone.
    std::filesystem::path relative = input.filename();
    for (const std::filesystem::path& root : m_roots) {
        const std::filesystem::path below = input.lexically_relative(root);
        if (!below.empty() && *below.begin() != ".." && *below.begin() != ".") {
            relative = below;
            break;
        }
    }
    if (!extension.empty()) {
        relative.replace_extension(extension);
    }
    return m_outputDir / relative;
}

std::expected<std::vector<Job>, std::string>
Destination::plan(const core::FileSystem& files,
                  const std::vector<std::string>& inputs,
                  std::string_view extension) const {
    std::vector<Job> jobs;
    jobs.reserve(inputs.size());
    for (const std::string& input : inputs) {
        jobs.push_back(Job{.input = input, .output = pathFor(input, extension)});
    }

    // Destinations seen so far, then every input, by `bucketOf` their name.
    std::unordered_map<std::string, std::vector<std::size_t>> destinations;
    std::unordered_map<std::string, std::vector<std::size_t>> sources;
    for (std::size_t i = 0; i < jobs.size(); ++i) {
        sources[bucketOf(jobs[i].input)].push_back(i);
    }

    for (std::size_t i = 0; i < jobs.size(); ++i) {
        const Job& job = jobs[i];
        const std::string bucket = bucketOf(job.output);

        for (const std::size_t earlier : destinations[bucket]) {
            if (sameFile(files, jobs[earlier].output, job.output)) {
                return std::unexpected{job.output.string() + ": would be written by both " +
                                       jobs[earlier].input + " and " + job.input};
            }
        }
        destinations[bucket].push_back(i);

        if (m_inPlace) {
            continue;
        }
        for (const std::size_t other : sources[bucket]) {
            if (sameFile(files, jobs[other].input, job.output)) {
                return std::unexpected{job.output.string() + ": written for " + job.input +
                                       ", but is itself the input " + jobs[other].input +
                                       ": use --in-place to write over the inputs"};
            }
        }
    }
    return jobs;
}

} // namespace subedit::cli
