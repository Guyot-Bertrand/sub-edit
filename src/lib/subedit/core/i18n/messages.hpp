#pragma once

// What the rest of the program calls to speak — ADR 0042.
//
// `translate("Open")` returns the installed catalogue's word for it, and the text itself when
// there is no catalogue, which is how English works: nothing is installed. A message is the
// English text (D2), so a missing translation degrades to something readable.
//
// **The catalogue is installed once, at start-up, before any thread runs**: the functions
// return views into it, and replacing it while one is held would leave that view dangling.
// A test that wants another language installs one and puts English back.

#include <subedit/core/i18n/catalogue.hpp>

#include <cstdint>
#include <initializer_list>
#include <memory>
#include <string>
#include <string_view>
#include <utility>

namespace subedit::core {

/// Makes `catalogue` the one that `translate` consults; null puts English back. `language` is the
/// name of the catalogue's language (`fr`, `pt_BR`), for whoever must speak of languages in it:
/// empty for English.
void installCatalogue(std::shared_ptr<const Catalogue> catalogue, std::string language = {});

/// The name of the language the installed catalogue is in, empty for English.
[[nodiscard]] std::string_view interfaceLanguageName();

/// The translation of `text`, or `text`.
[[nodiscard]] std::string_view translate(std::string_view text);

/// The translation of `text` in `context`, or `text`. A context separates two identical
/// English words that translate differently; it is not a way of grouping messages.
[[nodiscard]] std::string_view translateIn(std::string_view context, std::string_view text);

/// The form that `n` selects, or — with no translation — `singular` for one and `plural` for
/// anything else. A sentence with a count never concatenates an `s`.
[[nodiscard]] std::string_view
translatePlural(std::string_view singular, std::string_view plural, std::uint64_t n);

/// Marks `text` for the catalogue and returns it as it is: the message of a constant table, which
/// cannot call `translate` before the catalogue is installed. Whoever shows it calls
/// `translate` on it then. `update-pot.sh` and the translation scanner read this call as they
/// read `translate`.
[[nodiscard]] constexpr std::string_view deferred(std::string_view text) {
    return text;
}

/// `%1`..`%9` replaced by the arguments, in one pass: an argument that itself contains `%2`
/// is not expanded again. A `%` not followed by a digit from 1 to 9, and a number beyond the
/// arguments, stay as written. Positional so that a translation may reorder them.
[[nodiscard]] std::string substitute(std::string_view text,
                                     std::initializer_list<std::string_view> arguments);

/// While one lives, `translate`, `translateIn` and `translatePlural` answer **in English** on
/// this thread, whatever catalogue is installed — the catalogue itself is left alone, so a view
/// it handed out earlier stays valid.
///
/// **For text that must not follow the language** (spec 15, D3): the `message` of a failure in
/// `--format json` is the same bytes in every language, yet it is the very sentence the error
/// stream shows translated. Rather than a second, English copy of every sentence, the sentence
/// is built twice — once as usual, once under this — by `inSourceLanguage`. Scopes nest.
class SourceLanguage {

public:
    SourceLanguage();
    ~SourceLanguage();

    SourceLanguage(const SourceLanguage&) = delete;
    SourceLanguage& operator=(const SourceLanguage&) = delete;
    SourceLanguage(SourceLanguage&&) = delete;
    SourceLanguage& operator=(SourceLanguage&&) = delete;
};

/// What `words()` says in English: the sentence a builder makes, built under `SourceLanguage`.
/// The builder must be pure — it is called for its words, and may already have been called once
/// for the translated ones.
template<typename Words>
[[nodiscard]] std::string inSourceLanguage(Words&& words) {
    const SourceLanguage english;
    return std::string{std::forward<Words>(words)()};
}

} // namespace subedit::core
