#include <subedit/core/io/file_system.hpp>
#include <subedit/core/text/correction_pattern.hpp>
#include <subedit/core/text/pattern_catalogue.hpp>

#include <algorithm>
#include <array>
#include <charconv>
#include <cstddef>
#include <expected>
#include <filesystem>
#include <functional>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <system_error>
#include <type_traits>
#include <utility>
#include <vector>

namespace subedit::core {

namespace {

constexpr std::array<PatternKind, 4> kKinds{PatternKind::CommonError,
                                            PatternKind::Capitalization,
                                            PatternKind::HearingImpaired,
                                            PatternKind::LineBreak};

constexpr std::string_view kBom = "\xEF\xBB\xBF";
constexpr std::string_view kWhitespace = " \t\r\n\v\f";

using Diagnostics = std::vector<PatternDiagnostic>;

[[nodiscard]] std::string_view stripped(std::string_view text) {
    const std::size_t first = text.find_first_not_of(kWhitespace);
    if (first == std::string_view::npos)
        return {};
    return text.substr(first, text.find_last_not_of(kWhitespace) - first + 1);
}

/// `re.sub(r"\\0(?!\d)", "", value)`: Gaupol's guard against GKeyFile and msgfmt
/// « repairing » a replacement (its issue #70) is a `\0` no digit follows, and
/// it disappears on reading. `\040` is a space, and stays.
[[nodiscard]] std::string withoutNullGuards(std::string_view value) {
    std::string out;
    for (std::size_t at = 0; at < value.size(); ++at) {
        const bool isNull = value[at] == '\\' && at + 1 < value.size() && value[at + 1] == '0';
        const bool digitFollows =
            at + 2 < value.size() && value[at + 2] >= '0' && value[at + 2] <= '9';
        if (isNull && !digitFollows) {
            ++at;
        } else {
            out += value[at];
        }
    }
    return out;
}

/// A list field: split on `;`, empty items dropped — `Human;OCR;`.
[[nodiscard]] std::vector<std::string> listOf(std::string_view value) {
    std::vector<std::string> items;
    while (!value.empty()) {
        const std::size_t end = std::min(value.find(';'), value.size());
        if (end > 0)
            items.emplace_back(value.substr(0, end));
        value.remove_prefix(std::min(end + 1, value.size()));
    }
    return items;
}

// --- The lines of a pattern file --------------------------------------------

struct Field {
    std::string value;
    int line;
};

/// A record as the file wrote it: a header and `Key=Value` lines.
struct RawRecord {
    int line = 0;
    std::map<std::string, Field, std::less<>> fields;
};

/// Cuts a file into records, as `_read_patterns_from_file` does.
///
/// A line whose first non-blank character is `#` is a comment — and only such a
/// line: the `#` of a musical note in the middle of an expression stays. Every
/// line is stripped. A line opening with `[` starts a record, whatever it says.
/// Any other line is `Key=Value`, the key losing the leading underscore the
/// files used to mark translatable fields with.
[[nodiscard]] std::vector<RawRecord>
recordsOf(std::string_view content, const std::filesystem::path& file, Diagnostics& diagnostics) {
    if (content.starts_with(kBom))
        content.remove_prefix(kBom.size());

    std::vector<RawRecord> records;
    int number = 0;
    while (!content.empty()) {
        const std::size_t end = std::min(content.find('\n'), content.size());
        const std::string_view line = stripped(content.substr(0, end));
        content.remove_prefix(std::min(end + 1, content.size()));
        ++number;

        if (line.empty() || line.front() == '#')
            continue;
        if (line.front() == '[') {
            records.push_back(RawRecord{.line = number, .fields = {}});
            continue;
        }
        const std::size_t equals = line.find('=');
        if (equals == std::string_view::npos) {
            diagnostics.push_back({PatternProblem::MalformedLine, file, number, std::string{line}});
            continue;
        }
        if (records.empty()) {
            diagnostics.push_back(
                {PatternProblem::FieldOutsideRecord, file, number, std::string{line}});
            continue;
        }
        std::string_view key = line.substr(0, equals);
        if (key.starts_with('_'))
            key.remove_prefix(1);
        records.back().fields.insert_or_assign(
            std::string{key},
            Field{.value = withoutNullGuards(line.substr(equals + 1)), .line = number});
    }
    return records;
}

// --- A record into a pattern ------------------------------------------------

/// What reading one record needs to say where it went wrong.
class RecordReading {

public:
    RecordReading(const RawRecord& record, const std::filesystem::path& file, Diagnostics& out)
        : m_record(record), m_file(file), m_out(out) {}

    /// The value of `key`, if the record has it.
    [[nodiscard]] std::optional<std::string_view> get(std::string_view key) {
        const auto found = m_record.fields.find(key);
        if (found == m_record.fields.end())
            return std::nullopt;
        m_used.insert(std::string{key});
        return std::string_view{found->second.value};
    }

    /// The value of `key`, which the record cannot do without — nothing, and a
    /// refused record, when it is not there.
    [[nodiscard]] std::optional<std::string_view> need(std::string_view key) {
        std::optional<std::string_view> value = get(key);
        if (!value)
            report(PatternProblem::MissingField, m_record.line, key);
        return value;
    }

    /// The value of `key`, which the record cannot do without **and cannot
    /// leave empty** — a name, an expression.
    [[nodiscard]] std::string_view needText(std::string_view key) {
        const std::optional<std::string_view> value = need(key);
        if (value && value->empty())
            refuseValue(key, "");
        return value.value_or("");
    }

    void refuseValue(std::string_view key, std::string_view value) {
        const auto found = m_record.fields.find(key);
        const int line = found == m_record.fields.end() ? m_record.line : found->second.line;
        report(PatternProblem::InvalidValue, line, std::string{key} + "=" + std::string{value});
    }

    /// Names the keys nothing read: the record is not refused for them.
    void reportUnknownFields() {
        for (const auto& [key, field] : m_record.fields) {
            if (!m_used.contains(key))
                m_out.push_back({PatternProblem::UnknownField, m_file, field.line, key});
        }
    }

    [[nodiscard]] bool refused() const { return m_refused; }

private:
    void report(PatternProblem problem, int line, std::string_view detail) {
        m_refused = true;
        m_out.push_back({problem, m_file, line, std::string{detail}});
    }

    const RawRecord& m_record;
    const std::filesystem::path& m_file;
    Diagnostics& m_out;
    std::set<std::string, std::less<>> m_used;
    bool m_refused = false;
};

[[nodiscard]] PatternFlags flagsOf(RecordReading& reading) {
    PatternFlags flags;
    for (const std::string& name : listOf(reading.get("Flags").value_or(""))) {
        if (name == "DOTALL")
            flags.dotAll = true;
        else if (name == "MULTILINE")
            flags.multiline = true;
        else if (name == "IGNORECASE")
            flags.ignoreCase = true;
        else
            reading.refuseValue("Flags", name);
    }
    return flags;
}

/// `Repeat=True` or `Repeat=False`; a record that says nothing does not repeat.
[[nodiscard]] bool repeatOf(RecordReading& reading) {
    const std::optional<std::string_view> value = reading.get("Repeat");
    if (!value || *value == "False")
        return false;
    if (*value != "True")
        reading.refuseValue("Repeat", *value);
    return *value == "True";
}

[[nodiscard]] ErrorClasses classesOf(RecordReading& reading) {
    ErrorClasses classes;
    const std::vector<std::string> names = listOf(reading.need("Classes").value_or(""));
    for (const std::string& name : names) {
        if (name == "Human")
            classes.human = true;
        else if (name == "OCR")
            classes.ocr = true;
        else
            reading.refuseValue("Classes", name);
    }
    if (names.empty() && reading.get("Classes").has_value())
        reading.refuseValue("Classes", "");
    return classes;
}

[[nodiscard]] CapitalizeAt capitalizeOf(RecordReading& reading) {
    const std::optional<std::string_view> value = reading.need("Capitalize");
    if (value == "After")
        return CapitalizeAt::After;
    if (value && *value != "Start")
        reading.refuseValue("Capitalize", *value);
    return CapitalizeAt::Start;
}

template<typename Number>
[[nodiscard]] Number numberOf(RecordReading& reading, std::string_view key) {
    Number number{};
    const std::optional<std::string_view> text = reading.need(key);
    if (!text)
        return number;
    const auto [end, error] = std::from_chars(text->data(), text->data() + text->size(), number);
    if (error != std::errc{} || end != text->data() + text->size())
        reading.refuseValue(key, *text);
    return number;
}

[[nodiscard]] std::
    variant<CommonErrorFields, CapitalizationFields, HearingImpairedFields, LineBreakFields>
    fieldsOf(PatternKind kind, RecordReading& reading) {
    const auto replacement = [&reading] {
        return std::string{reading.get("Replacement").value_or("")};
    };
    if (kind == PatternKind::CommonError) {
        return CommonErrorFields{.classes = classesOf(reading),
                                 .replacement = replacement(),
                                 .repeat = repeatOf(reading)};
    }
    if (kind == PatternKind::Capitalization)
        return CapitalizationFields{.capitalize = capitalizeOf(reading)};
    if (kind == PatternKind::HearingImpaired)
        return HearingImpairedFields{.replacement = replacement()};
    return LineBreakFields{.group = numberOf<int>(reading, "Group"),
                           .penalty = numberOf<double>(reading, "Penalty")};
}

/// A record, read; nothing if it was refused.
[[nodiscard]] std::optional<CorrectionPattern> patternOf(PatternKind kind,
                                                         std::string_view code,
                                                         std::size_t rank,
                                                         const RawRecord& record,
                                                         const std::filesystem::path& file,
                                                         Diagnostics& diagnostics) {
    RecordReading reading{record, file, diagnostics};
    CorrectionPattern pattern;
    pattern.code = std::string{code};
    pattern.rank = rank;
    pattern.name = std::string{reading.needText("Name")};
    pattern.description = std::string{reading.get("Description").value_or("")};
    pattern.expression = std::string{reading.needText("Pattern")};
    pattern.flags = flagsOf(reading);

    const std::optional<std::string_view> policy = reading.get("Policy");
    pattern.replacesSameName = policy && *policy == "Replace";
    if (policy && !pattern.replacesSameName)
        reading.refuseValue("Policy", *policy);
    pattern.skipIn = listOf(reading.get("SkipIn").value_or(""));
    pattern.fields = fieldsOf(kind, reading);

    reading.reportUnknownFields();
    if (reading.refused())
        return std::nullopt;
    return pattern;
}

// --- The activation files ---------------------------------------------------

struct Activation {
    std::string name;
    bool enabled;
};

/// The five entities XML predefines, which is all an attribute of these files
/// has ever needed.
[[nodiscard]] std::string unescaped(std::string_view text) {
    constexpr std::array<std::pair<std::string_view, char>, 5> kEntities{
        {{"&quot;", '"'}, {"&apos;", '\''}, {"&lt;", '<'}, {"&gt;", '>'}, {"&amp;", '&'}}};
    std::string out;
    while (!text.empty()) {
        const auto* const entity = std::ranges::find_if(
            kEntities, [text](const auto& one) { return text.starts_with(one.first); });
        if (entity != kEntities.end()) {
            out += entity->second;
            text.remove_prefix(entity->first.size());
        } else {
            out += text.front();
            text.remove_prefix(1);
        }
    }
    return out;
}

/// The value of attribute `key` inside a start tag, quoted either way.
[[nodiscard]] std::optional<std::string> attributeOf(std::string_view tag, std::string_view key) {
    std::size_t at = 0;
    while ((at = tag.find(key, at)) != std::string_view::npos) {
        const bool startsName = at > 0 && kWhitespace.find(tag[at - 1]) != std::string_view::npos;
        std::size_t after = at + key.size();
        at = after;
        if (!startsName)
            continue;
        after = tag.find_first_not_of(kWhitespace, after);
        if (after == std::string_view::npos || tag[after] != '=')
            continue;
        after = tag.find_first_not_of(kWhitespace, after + 1);
        if (after == std::string_view::npos || (tag[after] != '"' && tag[after] != '\''))
            continue;
        const std::size_t close = tag.find(tag[after], after + 1);
        if (close == std::string_view::npos)
            continue;
        return unescaped(tag.substr(after + 1, close - after - 1));
    }
    return std::nullopt;
}

/// The `<pattern name="…" enabled="…"/>` elements of a `.conf`, as
/// `_read_config_from_file` reads them: a value other than `true` is off.
///
/// **Not an XML parser**, and it does not pretend to be one: it finds the start
/// tags of `pattern` elements, skipping comments. The files are written by
/// Gaupol, one element to a line.
[[nodiscard]] std::vector<Activation>
activationsOf(std::string_view xml, const std::filesystem::path& file, Diagnostics& diagnostics) {
    std::vector<Activation> activations;
    std::size_t at = 0;
    while ((at = xml.find('<', at)) != std::string_view::npos) {
        if (xml.substr(at).starts_with("<!--")) {
            const std::size_t close = xml.find("-->", at);
            at = close == std::string_view::npos ? xml.size() : close + 3;
            continue;
        }
        const std::size_t end = xml.find('>', at);
        const std::string_view tag = xml.substr(at, end == std::string_view::npos ? end : end - at);
        at += 1;
        const bool isPattern = tag.starts_with("<pattern") && tag.size() > 8 &&
                               kWhitespace.find(tag[8]) != std::string_view::npos;
        if (!isPattern)
            continue;

        const std::optional<std::string> name = attributeOf(tag, "name");
        if (!name) {
            const auto line = static_cast<int>(std::ranges::count(xml.substr(0, at), '\n')) + 1;
            diagnostics.push_back({PatternProblem::MalformedActivation, file, line, "name"});
            continue;
        }
        activations.push_back(
            {*name, attributeOf(tag, "enabled").value_or("") == std::string{"true"}});
    }
    return activations;
}

// --- The directories --------------------------------------------------------

/// `Latn-en.common-error` → `Latn-en` and the kind, when `name` ends in `suffix`.
[[nodiscard]] std::optional<std::string> codeBefore(std::string_view name,
                                                    std::string_view suffix) {
    if (!name.ends_with(suffix) || name.size() == suffix.size())
        return std::nullopt;
    return std::string{name.substr(0, name.size() - suffix.size())};
}

[[nodiscard]] std::optional<std::string>
contentOf(const FileSystem& files, const std::filesystem::path& file, Diagnostics& diagnostics) {
    std::expected<std::string, FileError> content = files.readFile(file);
    if (!content) {
        diagnostics.push_back(
            {PatternProblem::FileUnreadable, file, 0, std::move(content.error().detail)});
        return std::nullopt;
    }
    return std::move(*content);
}

void readPatternFile(const FileSystem& files,
                     const std::filesystem::path& file,
                     PatternKind kind,
                     std::string_view code,
                     std::vector<CorrectionPattern>& patterns,
                     Diagnostics& diagnostics) {
    const std::optional<std::string> content = contentOf(files, file, diagnostics);
    if (!content)
        return;
    std::size_t rank = 0;
    for (const RawRecord& record : recordsOf(*content, file, diagnostics)) {
        ++rank;
        if (std::optional<CorrectionPattern> pattern =
                patternOf(kind, code, rank, record, file, diagnostics)) {
            patterns.push_back(std::move(*pattern));
        }
    }
}

/// Lists `directory`. A directory that is not there is a diagnostic only if
/// `mustExist`.
[[nodiscard]] std::vector<std::filesystem::path> filesOf(const FileSystem& files,
                                                         const std::filesystem::path& directory,
                                                         bool mustExist,
                                                         Diagnostics& diagnostics) {
    std::expected<std::vector<std::filesystem::path>, FileError> listed = files.filesIn(directory);
    if (listed)
        return std::move(*listed);
    if (mustExist || listed.error().kind != FileErrorKind::NotFound) {
        diagnostics.push_back(
            {PatternProblem::DirectoryUnreadable, directory, 0, std::move(listed.error().detail)});
    }
    return {};
}

void readPatternFilesOf(const FileSystem& files,
                        const std::vector<std::filesystem::path>& listed,
                        std::vector<CorrectionPattern>& patterns,
                        Diagnostics& diagnostics) {
    for (const std::filesystem::path& file : listed) {
        const std::string name = file.filename().string();
        for (const PatternKind kind : kKinds) {
            const std::optional<std::string> code =
                codeBefore(name, "." + std::string{fileExtensionOf(kind)});
            if (code)
                readPatternFile(files, file, kind, *code, patterns, diagnostics);
        }
    }
}

void applyActivations(const FileSystem& files,
                      const std::vector<std::filesystem::path>& listed,
                      std::vector<CorrectionPattern>& patterns,
                      Diagnostics& diagnostics) {
    for (const std::filesystem::path& file : listed) {
        const std::string name = file.filename().string();
        for (const PatternKind kind : kKinds) {
            const std::optional<std::string> code =
                codeBefore(name, "." + std::string{fileExtensionOf(kind)} + ".conf");
            if (!code)
                continue;
            const std::optional<std::string> xml = contentOf(files, file, diagnostics);
            if (!xml)
                continue;
            for (const Activation& activation : activationsOf(*xml, file, diagnostics)) {
                for (CorrectionPattern& pattern : patterns) {
                    if (pattern.kind() == kind && pattern.code == *code &&
                        pattern.name == activation.name) {
                        pattern.enabled = activation.enabled;
                    }
                }
            }
        }
    }
}

/// `_get_codes`: `Zyyy`, then each prefix of the requested code.
[[nodiscard]] std::vector<std::string> codesFor(std::string_view code) {
    std::vector<std::string> codes{"Zyyy"};
    if (code.empty() || code == "Zyyy")
        return codes;
    for (std::size_t dash = code.find('-'); dash != std::string_view::npos;
         dash = code.find('-', dash + 1)) {
        codes.emplace_back(code.substr(0, dash));
    }
    codes.emplace_back(code);
    return codes;
}

} // namespace

PatternCatalogue readPatternCatalogue(const FileSystem& files,
                                      const std::filesystem::path& shipped,
                                      const std::filesystem::path& user) {
    PatternCatalogue catalogue;
    const std::vector<std::filesystem::path> shippedFiles =
        filesOf(files, shipped, true, catalogue.m_diagnostics);
    readPatternFilesOf(files, shippedFiles, catalogue.m_patterns, catalogue.m_diagnostics);
    if (!user.empty()) {
        readPatternFilesOf(files,
                           filesOf(files, user, false, catalogue.m_diagnostics),
                           catalogue.m_patterns,
                           catalogue.m_diagnostics);
    }
    applyActivations(files, shippedFiles, catalogue.m_patterns, catalogue.m_diagnostics);
    return catalogue;
}

std::vector<const CorrectionPattern*> PatternCatalogue::cascade(PatternKind kind,
                                                                std::string_view code) const {
    const std::vector<std::string> codes = codesFor(code);

    // `get_patterns`: every code's records, but those a requested code excludes.
    std::vector<const CorrectionPattern*> gathered;
    for (const std::string& one : codes) {
        for (const CorrectionPattern& pattern : m_patterns) {
            const bool skipped =
                std::ranges::any_of(pattern.skipIn, [&codes](const auto& excluded) {
                    return std::ranges::find(codes, excluded) != codes.end();
                });
            if (pattern.kind() == kind && pattern.code == one && !skipped)
                gathered.push_back(&pattern);
        }
    }

    // `_filter_patterns`: a name found again is filed after the last one that
    // bears it, or in its place if the newcomer says `Replace`. The removed
    // entries are left as holes until the newcomer is in, as Gaupol does — the
    // position it lands at depends on it.
    std::vector<const CorrectionPattern*> filtered;
    for (const CorrectionPattern* pattern : gathered) {
        std::size_t last = filtered.size();
        for (std::size_t at = 0; at < filtered.size(); ++at) {
            if (filtered[at] == nullptr || filtered[at]->name != pattern->name)
                continue;
            last = at + 1;
            if (pattern->replacesSameName)
                filtered[at] = nullptr;
        }
        filtered.insert(filtered.begin() + static_cast<std::ptrdiff_t>(last), pattern);
        std::erase(filtered, nullptr);
    }
    return filtered;
}

} // namespace subedit::core
