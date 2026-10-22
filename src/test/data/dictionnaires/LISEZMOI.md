# A trial dictionary, for joining and splitting words

`zz.dic` and `zz.aff`: **six words and nothing else**, in Hunspell format, for a language
(`zz`) that no machine owns. The end-to-end cases of `correct --tasks join-words,split-words`
drop them into `hunspell/` under the `ENCHANT_CONFIG_DIR` the harness has moved: Enchant
finds them through its Hunspell engine, and **no test depends on a dictionary installed on the machine
that runs it**. Issue [#569](https://github.com/Guyot-Bertrand/sub-edit/issues/569).

| File | What it is |
| :------ | :----------- |
| `zz.dic` | the number of words, then `hello`, `there`, `world`, `how`, `are`, `you` — one per line |
| `zz.aff` | the encoding (UTF-8) and the order of letters Hunspell tries to propose a correction |

**What these cases prove, and what they do not.** They prove that the command line
opens a dictionary through Enchant, hands it to the join and the split, and that what they
return reaches the file — with expected files written by hand, from Gaupol's rule:
`hel lo` is `hello` alone when only one direction spells; `hellothere` is `hello there`
alone when only one suggestion is the word with a space. **They prove nothing about a real
dictionary** — the quality of `fr_FR` is not our business — and **they assume Enchant's
Hunspell engine**, which is that of most installations: without it, they fail and say so.
