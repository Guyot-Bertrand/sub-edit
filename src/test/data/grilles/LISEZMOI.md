# Fixtures on a known grid

Fifteen generated SubRip files, whose text and meaning are not read: what
is read in them is **whether the positions fall on a frame grid, and
which one**. This is the data phase 16 needs, and which a subtitle file never declares.

**Do not edit them by hand, and do not take them on trust.**
[`src/scripts/subtitle-fixtures.py`](../../../scripts/subtitle-fixtures.py)
carries the table of what each fixture is, and rebuilds each file to
compare it with the one on disk:

```console
$ ./src/scripts/subtitle-fixtures.py --check      # the disk, against the table
$ ./src/scripts/subtitle-fixtures.py --generate   # rebuild them
$ ./src/scripts/subtitle-fixtures.py --measure    # what they give
```

`--check` runs in `make check-local`. That is where the guarantee lies: a
file of one hundred and seventy-six lines is unreadable in a diff, and no one
will reread these 128 KB.

## What each fixture is

| Fixture | Rate | Span | Offset | Lines | Weight |
| :------ | :-------- | ------: | -------: | --------: | ----: |
| `grille-23-976.srt` | `24000/1001` | 10 min | — | 176 | 10,734 B |
| `grille-24.srt` | `24/1` | 10 min | — | 176 | 10,734 B |
| `grille-25.srt` | `25/1` | 10 min | — | 170 | 10,367 B |
| `grille-29-97.srt` | `30000/1001` | 10 min | — | 174 | 10,636 B |
| `grille-30.srt` | `30/1` | 10 min | — | 176 | 10,763 B |
| `grille-50.srt` | `50/1` | 10 min | — | 171 | 10,494 B |
| `grille-59-94.srt` | `60000/1001` | 10 min | — | 166 | 10,190 B |
| `grille-60.srt` | `60/1` | 10 min | — | 170 | 10,440 B |
| `grille-absurde.srt` | `263/10` | 10 min | — | 177 | 10,810 B |
| `grille-24-decalee.srt` | `24/1` | 10 min | +2,999 ms | 168 | 10,242 B |
| `grille-24-courte.srt` | `24/1` | 10 s | — | 37 | 2,152 B |
| `grille-24-fins-calculees.srt` | `24/1` **on the starts** | 10 min | — | 170 | 10,364 B |
| `sans-grille.srt` | **none** | 10 min | — | 166 | 10,242 B |
| `melange-groupe.srt` | `30000/1001` | 10 min | — | 168 | 10,264 B |
| `melange-disperse.srt` | `25/1` | 10 min | — | 171 | 10,454 B |

Each position is `round(n × 1000 / R)`, rounded to the nearest with halves
going up, plus the offset. The step between two lines is irregular —
two to five seconds — and drawn from a congruential generator written in the
script: a constant step would make the phases periodic, and the file at an
absurd rate would come out concentrated on a candidate it does not touch.

**Starts and ends are both on the grid — except for one.**
In a real file, only the starts are: a *cue-out* is often
computed by a reading-speed rule, which makes the ends fall
between 55 and 100. `grille-24-fins-calculees.srt` is that fixture, added in
phase 10; the other fourteen keep their ends on the grid, and it is the contrast
that makes the first one useful.

`--measure` renders the comparison as a three-column table: the candidate
that the starts designate, what they are worth there, and what the ends are worth there.
Fourteen files carry the same number twice; the fifteenth reads 99.9
on the starts and 52.4 on the ends.

## What they give

Phase concentration on the eight candidates, in percent. Redoable with
`--measure`: these figures are measured on these files, not reported.

| Fixture | 23.976 | 24 | 25 | 29.97 | 30 | 50 | 59.94 | 60 |
| :------ | -----: | -: | -: | ----: | -: | -: | ----: | -: |
| `grille-23-976.srt` | **99.9** | 4.6 | 3.1 | 5.1 | 4.7 | 6.7 | 9.1 | 1.9 |
| `grille-24.srt` | 2.6 | **99.9** | 7.6 | 9.0 | 6.3 | 2.7 | 3.6 | 10.2 |
| `grille-25.srt` | 8.6 | 10.1 | **100** | 5.0 | 8.5 | **100** | 9.9 | 2.7 |
| `grille-29-97.srt` | 7.5 | 16.1 | 6.2 | **99.8** | 1.8 | 10.3 | **99.4** | 1.8 |
| `grille-30.srt` | 8.4 | 14.4 | 5.0 | 2.6 | **99.9** | 6.5 | 0.1 | **99.5** |
| `grille-50.srt` | 1.6 | 9.6 | 15.8 | 6.2 | 2.1 | **100** | 2.3 | 7.3 |
| `grille-59-94.srt` | 1.6 | 4.1 | 5.0 | 4.8 | 0.8 | 3.8 | **99.4** | 2.7 |
| `grille-60.srt` | 15.1 | 8.8 | 7.1 | 6.4 | 1.3 | 8.2 | 2.3 | **99.5** |
| `grille-absurde.srt` | 9.2 | 8.3 | 15.3 | 2.1 | 13.2 | 6.2 | 3.8 | 9.0 |
| `grille-24-decalee.srt` | 3.7 | **99.9** | 11.8 | 2.6 | 9.5 | 5.4 | 8.6 | 4.7 |
| `grille-24-courte.srt` | *92.7* | **99.9** | 3.9 | 10.9 | 22.2 | 10.1 | 27.2 | 8.6 |
| `grille-24-fins-calculees.srt` | 1.1 | **99.9** | 5.5 | 7.0 | 3.8 | 7.6 | 11.0 | 19.9 |
| `sans-grille.srt` | 3.2 | 2.9 | 7.3 | 5.0 | 5.8 | 12.5 | 5.2 | 4.0 |
| `melange-groupe.srt` | 3.4 | 2.7 | 2.6 | *65.1* | 1.8 | 1.7 | *66.4* | 1.1 |
| `melange-disperse.srt` | 4.1 | 7.6 | *75.4* | 8.1 | 2.6 | *84.9* | 2.9 | 11.1 |

## What each one tests

**The eight perfect grids** say that the deduction finds the right
candidate. They also say that **the harmonic ambiguity is real**: a
25 grid is included in a 50 grid, and the file comes out at 100 on both.
The only ambiguous pairs of the normalised set are those where one
is the integer multiple of the other — 25 and 50, 29.97 and 59.94, 30 and 60. No
others: 24 and 60 are in a ratio of two and a half, so only one frame in two
coincides.

**`grille-absurde.srt`** says that **failure is loud**. A regular file,
but on a grid that is none of the eight, stays under 16 everywhere: it is an
unequivocal "I don't know", and not a wrong answer. This is the
property that authorises using the deduction, where a file-name heuristic
would never have offered it.

**`grille-24-decalee.srt`** says that **the phase is not waste**. Its
positions are those of `grille-24.srt` translated by 2,999 ms — an offset
that Gaupol applies as is — and the grid stays at 99.9. A first version
of the method looked for a *zero-phase* grid and scored a file like this one
at zero.

**`grille-24-courte.srt`** says that **telling 23.976 from 24 takes
span**. The two grids drift by a millisecond per second of film,
and a frame lasts 41.7: the phase on the wrong candidate makes a
full turn every forty-two seconds. Over ten seconds, it covers
only a quarter, and 23.976 stays at 92.7 against 99.9. The deduction must return
the span it had in view, and not a concentration alone.

The file is **tight** — thirty-seven lines in ten seconds — and this is
deliberate. At normal dialogue density, ten seconds give only three
lines, and three points say nothing: the concentration of any candidate
is already worth `1/√3` there. Chance is not ambiguity.

**`melange-groupe.srt`** is the **grouped** partial case: two thirds on a
29.97 grid, the last third passed through an affine transformation that destroys the
grid. This is what an assembled file, or a re-timed section, produces. The
concentration falls to 65, between the clean grid and noise — and saying "two thirds of your starts
are on a 29.97 grid" is true without helping anyone. This
file exists so that one must say **which ones**.

**`melange-disperse.srt`** is the **scattered** partial case: one position in
five moved by one to ten frames, as a manual fix in the
table would do. The concentration falls into the same band, and that is precisely what
makes the case hard: the two files look alike by the figure and do not look alike at all by the
cause.

**`grille-24-fins-calculees.srt`** says that **the starts are enough**. Its starts
are those of a 24 grid; its ends are computed at fifteen characters per
second, with a floor of a second and a half and a frame of gap before the
next start — the two Gaupol defaults, laid down in the script. It does not
claim to be the duration adjustment that phase 10 will write: it produces
off-grid ends in a plausible and reproducible way.

The day the deduction started reading the ends, **it would fall and the
other fourteen would stay green.** That is its whole reason for being.

**`sans-grille.srt`** says that **failure is loud, and this no longer rests on a
corpus that nobody owns**. Its positions are arbitrary milliseconds,
and the best of the eight candidates rises there to 12.5 — far from the
fifty where the partial band opens. The roadmap asserted that a file written in
milliseconds without a grid "stays under a few percent on the
eight candidates"; it asserted it from three files of the private corpus,
and that was testimony. This is a measurement.

**It is not the same question as `grille-absurde.srt`.** That one is
perfectly regular, on a grid of 26.3 frames per second, and it tests
"none of the eight". This one is regular on nothing, and it tests "no
grid" — the false positive, which makes sense only at the size of a real file.
The noise floor is `1/√n`: seven and a half percent at one hundred and seventy-six
positions, and two thousand draws of arbitrary starts never push
the best of the eight above twenty-four.
