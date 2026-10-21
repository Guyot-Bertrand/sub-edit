# Duration fixtures

Nine small files (eight SubRip, one WebVTT), built so that **each pair
of duration constraints decides somewhere**, and each overlap. They serve
to cross-check `subedit-cli adjust` against
[`measure-duration-constraints.py`](../../../scripts/measure-duration-constraints.py)
— issue [#560](https://github.com/Guyot-Bertrand/sub-edit/issues/560), which holds the referral from #407.

**The expected counts are written by hand**, from the definitions, in
[`attendus/json/recoupement-defaut.jsonl`](../attendus/json/recoupement-defaut.jsonl)
(speed 15 characters per second that lengthens, minimum 1.5 s, maximum 6 s, gap 0) and
[`recoupement-ecart.jsonl`](../attendus/json/recoupement-ecart.jsonl) (the same with a gap of
0.5 s, without which a negative room does not exist in a chronological file). Both
tools are checked against them:

```console
$ ./src/scripts/measure-duration-constraints.py --check-fixtures    # the script (make check-local)
$ ./build/dev/bin/subedit_e2e_test "[CLI-ADJUST-06]"               # the core
```

Do not edit them without redoing the calculation by hand: **an expected file replaced by
what the tool writes proves nothing more than its agreement with itself.**

A subtitle's room is `next.start − start − gap`, the need
`length × 1000 / speed` rounded to the millisecond; `S1` is the gap 0, `S2` the gap 0.5 s.
Each file has two subtitles, except `ecart.srt` which has three.

| File | What it isolates | S1 (speed, minimum, gap) | S2 |
| :------ | :------------- | :--------------------------- | :-- |
| `minimum-contre-ecart.srt` | a room of 1.0 s for a minimum of 1.5 s | 0, 1, 0 | 0, 1, 0 |
| `vitesse-contre-ecart.srt` | 30 characters (2.0 s) for a room of 1.8 s | 1, 0, 0 | 1, 1, 0 |
| `vitesse-contre-maximum.srt` | 100 characters (6.667 s) above the maximum of 6 s | 1, 0, 0 | 1, 0, 0 |
| `union.srt` | both at once: **counts as one, not as two** | 1, 0, 0 | 1, 0, 0 |
| `ecart.srt` | two subtitles that start 0.3 s apart | 0, 1, 0 | 1, 1, 1 |
| `balises.srt` | `vitesse-contre-ecart` with tags, which do not count | 1, 0, 0 | 1, 1, 0 |
| `deux-lignes.srt` | a two-line text: the line break counts (31 characters) | 1, 0, 0 | 1, 0, 0 |
| `arrondi.srt` | "Hi": 133.33 ms rounded to 133, for a room of 133 ms | 0, 1, 0 | 1, 1, 1 |
| `lecture.vtt` | `minimum-contre-ecart`, in WebVTT | 0, 1, 0 | 0, 1, 0 |

`arrondi.srt` is the one that found a defect: the script compared in floating point, and counted
a sacrificed speed that the core, which rounds, does not count.
