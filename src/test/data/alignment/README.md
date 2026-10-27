# Alignment cases

A subtitle file **laid at a known fault**, what an engine would have heard in the audio, and the **truth** of where each
line is really said. They exist so that the alignment of phase 23 is held against what is laid down, and never against what
it finds itself.

**Nothing here is edited by hand**: [`src/scripts/alignment-cases.py`](../../../scripts/alignment-cases.py) writes it, from
arithmetic, and the `fixtures` step of the gate replays it (`--check`).

| Case | The fault laid on the file | What the truth says |
| :--- | :------------------------- | :------------------ |
| `offset-plus-2400` | every line 2.4 s late | each line at its real place |
| `offset-minus-900` | every line 0.9 s early | idem |
| `drift` | a delay growing from 0 to 3 s over the file | idem |
| `partial` | the second half 2 s late | idem |
| `extra-line` | 1.8 s late, **and a line that is said nowhere** | `-` for that line: nothing to find |
| `unheard-line` | 1.8 s late, **and a spoken line the engine did not hear** | its real place, though the engine has no words for it |

Each case is a directory:

- `given.srt` — what the person has;
- `heard.json` — what an engine would have written (`whisper-cli -ojf` form) at the **true** times, with the words of the true
  lines, and nothing for a line it did not hear;
- `truth.txt` — for each line of `given.srt`, in order, `start end` in milliseconds where it is really said, or `-` when it is said
  nowhere.

The words are made up: no film's dialogue is carried here. The test helper `alignment_cases.hpp` reads a case and measures an
answer against its truth (how far each found line is, what was missed, what was invented), which is the **figure** a change of method
shows.

**What is not here, on purpose**: the audio. A synthetic voice is not available on every machine, and a recorded one would be a
file nobody can regenerate. The run on a real film is by hand, outside the gate, and is documented with the first adapter, which is
what it needs.
