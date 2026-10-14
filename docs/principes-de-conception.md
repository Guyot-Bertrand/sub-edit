# Design principles

Permanent rules that apply to all of `subedit`'s code. They take precedence over
personal habits and over the convenience of the moment. The spec of each
sub-project refers to them rather than repeating them.

## 1. The compiler is the first test

**A loose type is a deferred bug.** Every piece of data that carries meaning has
its own type; validation by the compiler is not bypassed in favour of a generic
container.

Forbidden, however convenient:

- a `map<string, any>` or a `map<string, variant<…>>` as a data model or as
  configuration;
- a bare `int` to represent a duration, a position, a subtitle number or a track
  index;
- a `string` used as an enumeration;
- a structure half of whose fields are valid depending on a "type" field.

The counter-example is in the original project: Gaupol's configuration is a
nested dictionary of some thirty sections whose values are arbitrary, and every
read is a bet on the type. Here it is a typed structure, validated on load, that
cannot be compiled wrong.

Corollaries:

- **strong types** rather than aliases: `Timestamp`, `Duration`, `FrameRate`,
  `SubtitleIndex` are distinct types, not implicitly convertible to one another
  nor to `int64_t`. Mixing up a duration and a position becomes a compile
  error, not a timing bug discovered by the user;
- **`enum class`** for every closed set of values: format, mode, line ending,
  operation target;
- **make invalid states unrepresentable**: prefer `std::optional`,
  `std::variant` and validating constructors over an object that can be
  constructed in an inconsistent state and "initialised" afterwards.

## 2. Explicit memory ownership

The rule fits in one sentence: **at every moment, it must be obvious who owns
what.**

- **Value semantics by default.** A dynamic allocation has to be justified; the
  absence of one does not.
- **No bare `new` or `delete`.** `std::unique_ptr` for polymorphic ownership,
  `std::make_unique` for creation.
- **Bare pointers and references = non-owning observers**, whose lifetime is
  guaranteed by design, never by luck.
- **`std::shared_ptr` only when shared ownership is demonstrated**, not because
  the lifetime is unclear. Unclear ownership gets fixed, not wrapped.
- **Rule of zero.** A class that manages no resource declares no destructor,
  copy constructor or assignment operator. If it declares one, it declares them
  all.
- **Documented exception: Qt.** The parent-child relationship of `QObject` is a
  legitimate ownership mechanism, confined to Qt code and flagged in a comment.

These rules do not rely on vigilance: LeakSanitizer, built into the `asan`
preset, makes the tests fail on any leak, and clang-tidy rejects bare owning
allocations.

## 3. Abstraction where variation is known

SOLID misapplied produces one interface per class, factories everywhere and code
more rigid than the code it replaces. The rule adopted: **an abstraction is
introduced when the variation is real and identified, not on principle.**

In `subedit`, it is so in five places, known from now on:

| Variation point | Why |
| :-------------- | :-- |
| Reading and writing formats | two implementations in phase 1, nine eventually |
| Regular expression engine | PCRE2 versus RE2, not settled |
| Video player | libmpv, settled by [ADR 0020](adr/0020-libmpv-pour-le-lecteur-integre.md); the abstraction stays, it is the backend that is chosen |
| Spell checker | hunspell, nuspell, or a system service |
| File system access | replaceable in tests, without writing to disk |

Everywhere else, a concrete class is enough until proven otherwise. Extracting
an interface later costs little; dragging along a useless indirection costs at
every reading of the code.

The other principles, applied without dogma:

- **Single responsibility** — a long file is a signal, not a fault in itself.
  The question is not the number of lines but: can what this unit does be
  described in one sentence without an "and"?
- **Liskov substitution** — an implementation that has to throw "not supported"
  signals a badly cut interface. The case will come up with formats: not all of
  them carry a header, handle the same tags, or all work in time. It is the
  model that must express it, not an exception at run time.
- **Dependency inversion** — the core knows neither Qt, nor the disk, nor the
  network. This constraint is already checked by the CI.
- **DRY is about knowledge, not characters.** Two fragments identical by
  coincidence stay separate; factoring out a duplicated business rule, on the
  other hand, is mandatory.

## 4. Testability by construction

- All the useful code lives in libraries; executables contain only wiring.
  Something that cannot be tested is the sign of a misplaced boundary.
- External dependencies — clock, file system, processes, video player — go
  through an injected abstraction, so that a test needs neither disk, nor
  network, nor waiting.
- A pure function is preferable to a method that mutates shared state, even
  when the latter is shorter to write.
- **A test writes only into a sandbox it owns**, never into the repository nor
  into the home directory of whoever runs it. A test that writes elsewhere
  damages what it finds and — more insidiously — becomes dependent on what was
  there: it passes for whoever never ran the binary and fails for whoever ran
  it once. The `Scratch` harness covers the repository, a relocated
  `XDG_CONFIG_HOME` covers the configuration, and two checks —
  `check-untracked.sh`, `check-config-home.sh` — refuse whatever gets past them.
- **A location is received, not looked up.** It is the same rule as the
  injection above, applied to a path: a type that resolves on its own where it
  lives can only be tested by letting it write there. The only code that
  resolves a standard location is the wiring, and it fits in a function that can
  be called — see `gui::userSettingsPath()` and ADR 0022.

## 5. Performance: measure, do not assume

Performance is a cross-cutting goal, which does not license speculative
optimisation.

- Choosing the right data structure from the start is a matter of design, not
  optimisation: it is what is expected.
- Any optimisation that **complicates the code** must be backed by a benchmark,
  versioned in `src/test/bench/`, showing the gain.
- Avoiding gratuitous copies is a matter of hygiene: `string_view` and `span` as
  parameters, `move` when the transfer is real, allocations outside loops.
- No performance figure is announced without a measurement to back it.
