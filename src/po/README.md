# Message catalogues

| File | What it is |
| :--- | :--------- |
| `subedit.pot` | the template: every literal passed to `translate`, `translateIn` or `translatePlural` in `src/lib` and `src/exe`. Generated, never edited by hand |
| `<lang>.po` | one catalogue per language, merged from the template |
| `LINGUAS` | the languages that are **compiled and installed**, one per line |

A `.po` that is not in `LINGUAS` stays in the repository without being built or shipped: a language ships when it is
complete and has been reviewed by someone who reads it (spec 15, D5).

## Workflow

```console
$ make pot        # rewrite subedit.pot from the code
$ make po         # merge the template into every language of LINGUAS
$ make pot-check  # fail when subedit.pot is not what the code produces (part of check-local)
```

`make pot` is reproducible: same sources, same bytes. Sources are sorted, locations name the file only (a line number
would change at every insertion), and `POT-Creation-Date` is the template value.

## Build and install

`cmake/Translations.cmake` compiles each language of `LINGUAS` with `msgfmt --check` into
`<build>/share/subedit/locale/<lang>/LC_MESSAGES/subedit.mo`, and installs it under
`<prefix>/share/subedit/locale/<lang>/LC_MESSAGES/`. The `.deb` and the `.rpm` carry the same tree.

The gettext tools (`msgfmt`, `xgettext`, `msgmerge`) are build-time tools only; `./src/scripts/setup-toolchain.sh`
installs them. Nothing of gettext is needed at run time: the program reads the `.mo` itself (ADR 0042).
