# Encoding detection

What the project's detection answers, on the two corpora that can be put to it,
and **where it stops knowing**.

**A rate, not "detection works".** A detection is a classifier: it has a
success rate, not a result that is right or wrong — that is what issue #290
wrote into `score-encoding-detection.py`, before there was a detection of our
own. There has been one since issue #296, and here are its figures.

## How to replay it

```console
$ make score
```

It has been a step of `check-local` since issue #311: the score is replayed on
every pull request, and **a departure from the record below shows**. A score
that falls fails, a score that rises invites `make score-record`, an unchanged
score stays silent. It is not a threshold: the number compared is the one of
the last measurement, never a bar someone has set.

The rest is run by hand, and the detector is an argument:

```console
$ cmake --build build/dev --target subedit_detect_encoding
$ ./src/scripts/score-encoding-detection.py \
      --detector './build/dev/bin/subedit_detect_encoding {}' --by-length --private
```

`--private` adds the private corpus, absent from any machine that does not have
it — the record reads without it, less complete. No gate reads it.

## Labelled corpus

`src/test/data/encodages/`, made by `encoding-fixtures.py`: the table that
writes them says which encoding each was written in, so the label comes with
the file.

<!-- generated record: do not edit by hand -->

    labelled corpus: 12/13

Recorded on version 0.9.6, on 2026-09-05.

| Fixture | Written in | Read as |
| :------ | :--------- | :------ |
| `cp1250-court.srt` | `cp1250` | `cp1252` |

<!-- end of record -->

**Four fixtures came in with issue #310**, and 9/9 became 12/13 without the
corpus having become harder than it should have been: it was measuring the easy
case. Nine monolingual fixtures of one hundred and fifty to four hundred bytes
said nothing about a single line of dialogue, nor about a bilingual file.

**Three short ones** — `latin1-court`, `cp1250-court`, `koi8-r-court` — of a
single cue. **One bilingual**, `koi8-r-rare`, of twenty cues of English and one
of Russian: that is the one that cost the most, and the next section is devoted
to it.

**Two of the thirteen have no demonstrable right answer.** `latin1.srt` first —
with no byte in the `0x80–0x9f` range, Latin-1 and CP1252 are the same
encoding, and counting it right is a convention. `cp1250-court.srt` next, which
none of the detectors tried reads right: eighteen bytes of Czech are not enough
to separate Central Europe from Western Europe. ICU weighs `windows-1252` and
`windows-1250` at exactly the same confidence there (47 each), and ranks the
first one first: the right answer is among its candidates, tied with the wrong
one. Choosing the tie in favour of CP1250 would misread Western text, which is
the commoner case. It is counted wrong, and it stays — that is what gives the
record something to measure.

## Where the detection stops knowing

The rate above is **one** figure over thirteen files. It does not say *where*
the case becomes hard, and issue #310 was born of that. `--by-length` makes a
graded corpus — fixed seed, sixty draws per row, nothing committed to the
repository — and hands it to the same detector.

Three rates are counted there, and they do not say the same thing. **Exact**:
the answer names the encoding it was written in. **Text**: the file read back
under the answer is the file written — which forgives a confusion without
consequence. **Script**: the text read back keeps the alphabet of the text
written — which forgives only mojibake within the same family.

The two tables below are recorded on version 0.9.6, **after**
[ADR 0028](../adr/0028-peser-les-lignes-qui-discriminent.md); the figures from
before are in the ADR, since it is the ADR they decided.

### By length — the whole file is accented

Exactness rate, by number of cues:

| Cues | 1 | 2 | 3 | 5 | 8 | 13 | 21 |
| :--- | --: | --: | --: | --: | --: | --: | --: |
| Latin-1 | 100 % | 100 % | 100 % | 100 % | 100 % | 100 % | 100 % |
| CP1250 | **20 %** | 53 % | 70 % | 85 % | 93 % | 95 % | 97 % |
| KOI8-R | 100 % | 100 % | 100 % | 100 % | 100 % | 100 % | 100 % |

**The script is right on all twenty-one rows**, at 100 % — not one answer
leaves the Latin family for Latin text, nor the Cyrillic one for Cyrillic. The
cliff the issue had seen is there, on CP1250, and it is of another nature than
announced: it is **the confusion between Latin encodings growing worse as the
text gets shorter**, not the switch to another script.

**It is not bytes that are missing, it is high bytes.** A cue of CP1250 is
fifty-six bytes of which two are above `0x7f`; one of KOI8-R is fifty-eight of
which nineteen, and it is recognised. Cyrillic writes every letter high, Czech
writes only its diacritics there.

**That column fell a few points with ADR 0028**, and that is the price it
names: removing the index and timing lines divides by three what is weighed on
a file of five cues. The choice between two Latin encodings loses by it, the
choice of the alphabet loses nothing.

### Rare text — a long file, almost all ASCII

This is the other end, and it is the one that made ADR 0028. A file of two
hundred or six hundred cues of English, of which only a few carry another
alphabet — a Russian film subtitled with English cards, a translator's note in
a fansub.

| Written in | Cues | Accented | Bytes | ≠ ASCII | Exact | Script |
| :--------- | ---: | -------: | ----: | ------: | ----: | -----: |
| Latin-1 | 600 | 1 | 39 826 | 3 | 100 % | 100 % |
| CP1250 | 200 | 3 | 13 181 | 8 | 67 % | 100 % |
| CP1250 | 200 | 10 | 13 112 | 28 | 95 % | 100 % |
| CP1250 | 600 | 1 | 39 815 | 2 | 10 % | 100 % |
| KOI8-R | 200 | 3 | 13 195 | 56 | **100 %** | **100 %** |
| KOI8-R | 600 | 10 | 39 724 | 190 | **100 %** | **100 %** |

**The two rows in bold were at 0 % in both columns.** Forty kilobytes saved
nothing: one hundred and ninety bytes of Cyrillic in an English file came back
as accented Latin, sixty times out of sixty. The diagnostic "an encoding nothing
declared" was indeed emitted — the best-effort reading of ADR 0008 held — but
it announced `ISO-8859-1` with the same assurance it would have announced the
right encoding.

**The script is now right everywhere**, and exactness stays low only where it
cannot rise: two bytes of Czech in forty kilobytes do not decide between CP1250
and CP1252, and no detector can.

### What the measurement attributes, and to whom

**It is not a limit of the information available**, and that is the finding of
the measurement. On a file of two hundred cues of which ten carry Cyrillic, the
three detectors do not say the same thing:

| Detector | Answer |
| :------- | :----- |
| ours, ICU | `ISO-8859-1` |
| `uchardet`, control | `KOI8-R` |
| `file -b --mime-encoding`, control | `iso-8859-1` |

ICU's ranking on this file puts `ISO-8859-1` at a confidence of 66, labelled
"en", and `KOI8-R` at 2, last of the six candidates. The mass of ASCII dominates
the letter statistics, and the rule "the first candidate that actually decodes"
can do nothing about it: a single-byte encoding decodes everything, so the
first in the ranking always wins.

**The fix fits in one line, and it is ADR 0028.** Submitting to ICU only the
lines that carry a high byte — lines of pure ASCII are identical in every
candidate, they discriminate nothing — turns the ranking of the same file
around: `KOI8-R` at 54, `ISO-8859-1` gone. The two edges that had to be tested
are written there, with what they cost.

### What does not come out of it: a threshold

Issue #310 considered having the detection return `nullopt` below a certain
length, and **the measurement rules it out**, for two reasons.

**A length threshold would miss the case.** The real defect is on files of
forty kilobytes; no lower bound on size sees them.

**And it would make unreadable what reads.** The detection is consulted only
when the bytes are **not** valid UTF-8 — it is the first rule of
`detectEncoding`. Returning `nullopt` makes `readSubtitles` fall back on UTF-8,
which fails by construction: the file no longer opens at all. An imperfect
answer that a diagnostic announces is worth more than a refusal.

## Private corpus — 73 files, without labels

`src/data/`, real files. **It gives no rate**: there is no truth to hold it
against, and inventing one would be worse than having none. What it gives is
what can be checked without a label. No gate reads it.

**It already held 73, and this record counted 71** — it is the walk that
changed, issue #340, not the corpus. It filtered on `.srt` and `.vtt`, so it
could only count `.srt` and `.vtt`; the initialisation of phase 9 concluded on
that basis that the corpus held nothing else. It also holds a MicroDVD and an
Advanced SSA, real, both in UTF-8. It is the defect of #268 — checking with the
tool that does not count — once more, and it was worth two files.

| Answer | our detection | `uchardet`, control |
| :----- | ------------: | ------------------: |
| `utf-8` | 59 | 53 |
| `cp1252` | 11 | 2 |
| `iso8859-1` | 3 | 12 |
| `ascii` | — | 6 |
| files that decode entirely under the answer | **73/73** | 73/73 |

**The two contradict each other on 15 files out of 73**, and it is the most
instructive figure in the table. None of these disagreements concerns anything
other than the Latin family: reading ASCII as Latin-1 changes no character of
the file as long as it is not rewritten.

That 73 answers out of 73 decode the file from end to end **proves next to
nothing**: a single-byte encoding decodes almost any sequence of bytes, and
that is precisely where mojibake comes from. The figure is necessary, it is not
sufficient. The previous section is the demonstration: the bilingual files all
decoded there, and all lost their second alphabet.

## The three rules the detection settles itself

They are in `core/text/encoding.cpp`, and they are what separates our answer
from ICU's.

**The BOM first, always.** It is the only thing a subtitle file declares about
its encoding. A heuristic that came before it would answer a question already
settled.

**Only the lines that discriminate are weighed.** A line of pure ASCII reads
the same under every candidate; submitting it to ICU only weighs in a letter
statistic where it has nothing to say. That is ADR 0028, and the "rare text"
section above is the measurement that decided it.

**Bytes that decode as UTF-8 are UTF-8, and ICU is not consulted.** Measured,
and the gap is the reason for the rule: on `valides/minimal.srt` — two accents
in one hundred and forty bytes — ICU's detector ranks ISO-8859-1 at 81 and
UTF-8 at 80. One point of letter statistics against a structural property, and
the file came back with `Ã©` where it had its accents.
