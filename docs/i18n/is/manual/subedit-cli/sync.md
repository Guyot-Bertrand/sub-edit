# `sync`

<!-- machine-translated -->
> Þessi síða er vélþýðing sem enginn lesandi tungumálsins hefur enn farið yfir.

```
subedit-cli sync (--video FILE | --audio FILE)
                 [--audio-track N]
                 [--language CODE]
                 [--start TIME] [--end TIME]
                 [--model FILE]
                 [--vad-model FILE]
                 [--engine NAME]
                 [--keep-audio FILE]
                 [--min-match PERCENT]
                 [--doubt-gap MS]
                 [--report FILE]
                 [-r]
                 [--output FILE | --output-dir DIR | --in-place] [--dry-run]
                 <files>...
```

**Tímastillir skjátextaskrá eftir hljóði myndskeiðs.** Tilvikið sem hún er til fyrir er `.srt`-skráin sem sótt er á netinu,
færð til í upphafi og í lokin, sem ein tilfærsla nær ekki að laga: hvers skjátexta er leitað í því sem sagt er, og
hann tekur sér stað þar sem hann er sagður.

Hún hlustar á hljóðið með umritunarvél — sömu vél og hjá [`transcribe`](transcribe.md) — og **ber orð hvers skjátexta
saman við orðin sem hún heyrði**. Ekkert er þýtt: skjátextarnir verða að vera á tungumálinu sem talað er.

**Ekkert af því sem hún þarf fylgir forritinu**: vélin, líkan og `ffmpeg` eru þau sem [Uppsetning umritunartækjanna](install-transcription.md)
lýsir, og skortur á þeim bilar ekkert — skipunin segir hvað vantar, með tilvísun á þá síðu, skrifar ekkert
og lýkur með kóða sem er ekki núll.

- **Aðeins upphaf og endir breytast.** Aldrei textinn, stíllinn, fjöldi skjátexta eða röð þeirra.
- **Skjátexti sem hljóðið gefur ekki heldur tímum sínum** — nafnalisti sem enginn les upp, lag, lína á öðru tungumáli.
  **Engin staða er nokkurn tíma fundin upp, og engin er giskuð út frá nágrönnunum**: línan er látin vera þar sem skráin hafði hana,
  og skipunin segir frá því. Það er *ekki* bilun, og skilakóðinn er `0`.
- **Skjátexti telst fundinn þegar að minnsta kosti helmingur orða hans heyrist** (`--min-match`, `alignment.min-match`), og
  að minnsta kosti eitt. Há- og lágstafir, broddstafir, greinarmerki og merkingar skipta ekki máli; orð sem er einum staf frá löngu orði telst heyrt.
- **Endirinn er þar sem síðasta orðið endar.** Vélin tímasetur orð til um það bil, nokkrum hundruðustu úr sekúndu á undan
  eigin enda skjátextans, svo skjátexti getur komið út örlítið styttri en hann var.
- **Skjátexti sem myndi rekast á þann næsta er styttur þangað sem sá næsti byrjar.** Sá sem skarast samt við
  þann næsta, sem styttingin getur ekki læknað, er látinn vera og sagt frá því.
- **Skrá þar sem enginn skjátexti fannst er hafnað og ekki skrifuð**: það er næstum örugglega rangt hljóð eða
  rangt tungumál.
- **Hægt er að hlusta á hluta myndskeiðsins einan**, með `--start` og `--end`, eins og hjá
  [`transcribe`](transcribe.md#að-umrita-hluta): aðeins skjátextanna sem **byrja** á bilinu er leitað og
  þeir snertir, hinir eru látnir vera eins og þeir eru og eru ekki taldir.
- **Hlustað er á hljóðið einu sinni fyrir hverja skrá sem gefin er.** Til að tímastilla nokkrar skrár eftir sama myndskeiði
  þarf að bíða eftir hverri.

<!-- example: subedit-cli sync --help -->
```console
$ subedit-cli sync --help
Re-time the subtitles of a file on the audio of a film
Usage: subedit-cli sync [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to re-time

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --video FILE                Film to listen to
  --audio FILE                Audio file to listen to, instead of a film
  --audio-track N             Audio track of a film, counted from 0; the first by default
  --start TIME                Where to begin, as 2:00, 1:02:03 or seconds; the beginning by default
  --end TIME                  Where to stop, as 2:00, 1:02:03 or seconds; the end by default
  --language CODE             Language of the speech, two letters: fr, en, de; detected from the first 30 seconds by default
  --model FILE                Model file; the transcription.model setting by default
  --vad-model FILE            Voice-activity model that skips silences; the transcription.vad-model setting by default
  --engine NAME               Engine that listens; the transcription.engine setting by default
  --keep-audio FILE           Keep the extracted audio in this file
  --min-match PERCENT         Share of a subtitle's words that must be heard, in percent; the alignment.min-match setting by default
  --doubt-gap MS              How far a subtitle may move from its neighbours' shift before it is flagged, in milliseconds; the alignment.doubt-gap setting by default
  --report FILE               Write the subtitles to check to this file
  --output TEXT               File to write, for a single input
  --output-dir TEXT           Directory to write into
  --in-place                  Write back over the inputs
  --dry-run                   Work out and say what would be written, and write nothing
```

## Röksemdir og valkostir

| Valkostur | Nauðsynlegt | Gildi | Sjálfgefið |
| :----- | :------- | :----- | :------ |
| `<files>` | já | skjátextaskrárnar sem á að tímastilla; með `-r`, möppur | — |
| `--recursive`, `-r` | nei | rofi | slökkt — sjá [Vinnsla á möpputré](lots.md) |
| `--video` | annað hvort `--video` eða `--audio` | myndskeiðið sem á að hlusta á | — |
| `--audio` | annað hvort `--video` eða `--audio` | hljóðskrá til að hlusta á, í stað myndskeiðs | — |
| `--audio-track` | nei | heiltala frá 0: staðan meðal hljóðrása myndskeiðsins | `0`, sú fyrsta |
| `--language` | nei | tveir stafir, ISO 639-1 kóði: `en`, `fr` | stillingin `transcription.language`; tóm: greint |
| `--start`, `--end` | nei | tími: `2:00`, `1:02:03`, `2:00.5`, eða fjöldi sekúndna; `--end` verður að koma á eftir `--start` | allt hljóðið |
| `--model` | nei | líkanskráin | stillingin `transcription.model` |
| `--vad-model` | nei | raddvirknilíkan, sem lætur vélina sleppa þögnum; með því fylgja orðin talinu nánar | stillingin `transcription.vad-model` |
| `--engine` | nei | heiti vélarinnar sem hlustar: `whisper.cpp` | stillingin `transcription.engine` |
| `--keep-audio` | nei | skrá til að geyma útdregna hljóðið í; mappan hennar er búin til ef hana vantar | hljóðið er hvergi geymt |
| `--min-match` | nei | heiltala frá 1 til 100: hlutfall orða skjátexta sem þarf að heyrast, í prósentum | stillingin `alignment.min-match`, `50` |
| `--doubt-gap` | nei | heiltala millisekúndna, 0 eða meira: hve langt tilfærsla skjátexta má víkja frá tilfærslu nágranna sinna áður en hann er merktur | stillingin `alignment.doubt-gap`, `2000` |
| `--report` | nei | skrá til að skrifa skjátextana sem á að athuga í, einn í hverja línu: númer skjátextans, tabulator og ástæðan; ein skjátextaskrá í einu | ekkert er skrifað |
| `--output` / `--output-dir` / `--in-place` | **eitt af þremur**, nema með `--dry-run` | sjá [Ræsing](invocation.md#áfangastaðurinn) | — |
| `--dry-run` | nei | rofi: reiknar og segir frá, skrifar ekkert — sjá [Að sjá áður en skrifað er](invocation.md#að-sjá-áður-en-skrifað-er) | slökkt |

Áfangastaðurinn — `--output`, `--output-dir`, `--in-place`, `--dry-run` — og `-r` eru þau sömu og hjá hinum skipununum sem
endurskrifa skrá: sjá [áfangastaðinn](invocation.md#áfangastaðurinn). Skráin heldur sniði sínu, kóðun sinni og
línulokum sínum.

**Stillingarnar** eru lesnar úr stillingaskránni — sjá [stillingaskrána](invocation.md#stillingaskráin) — og hver
á sinn valkost hér. Vélin, líkanið og raddvirknilíkanið eru þau sömu og hjá `transcribe`.

| Lykill | Merking | Sjálfgefið |
| :-- | :------ | :------ |
| `alignment.engine` | stillivélin; ein er til, `transcript-matching`, sem hlustar og ber saman | `transcript-matching` |
| `alignment.min-match` | hlutfall orða skjátexta sem þarf að heyrast til að hann teljist fundinn, í prósentum | `50` |
| `alignment.doubt-gap` | hve langt, í millisekúndum, tilfærsla skjátexta má víkja frá tilfærslu nágranna sinna áður en hann er merktur | `2000` |

## Hvað er merkt

Skipunin segir hvaða skjátexta á að athuga, og hvers vegna. Skjátexti er merktur fyrir **fyrsta** af þessum atriðum sem á við:

| Hvers vegna | Merking |
| :-- | :------ |
| not found in the audio, left as it was | færri en `--min-match` af orðum hans heyrðust: hann heldur tímum sínum |
| only partly heard | fundinn, en færri en 80 % orða hans heyrðust |
| moved differently from its neighbours | fundinn, en tilfærsla hans víkur meira en `--doubt-gap` frá miðgildi tilfærslu skjátextanna tveggja sitt hvorum megin |

Skjátexti getur líka verið **styttur að upphafi næsta skjátexta**, eða **skarast enn við næsta skjátexta**.

Tvær tölur segja sögu skrárinnar í heild. **Miðgildi tilfærslunnar** er miðja þess hve langt skjátextarnir færðust: skrá sem var
einfaldlega of sein sést hér, og hvergi annars staðar. **Reki** er miðgildi tilfærslu síðasta tíunda hluta
skjátextanna að frádregnu miðgildi þess fyrsta: hann er nálægt núlli fyrir fasta tilfærslu, og mikill fyrir skrá sem var of sein um
sífellt meira eftir því sem leið á myndskeiðið.

## Úttak

**Hefðbundið úttak** — ekkert: niðurstaðan er skráin sem var skrifuð.

| Stig | Hvað bætist við á villuúttakið |
| :---- | :------------------------------- |
| 1 | `<file>: N of M subtitles placed on the audio, K to check; median shift X s, drift Y s -> <destination>` |
| 2 | skrefið og framvindan, eins og hjá [`transcribe`](transcribe.md#úttak) |

### Á JSON-sniði

Með [`--format json`](invocation.md#úttak-sem-skriftur-geta-lesið) ber hlutur skrárinnar `counts`: `subtitles`
(leitað að), `placed` (fengið stað), `moved` (sem tímar breyttust hjá), `partial`, `out_of_line`, `not_found`,
`median_shift_ms` og `drift_ms`; og `flagged`, lista af `{"subtitle": N, "kind": "…"}` þar sem númerið er
staða skjátextans í skránni og `kind` er eitt af `not-found`, `partial`, `out-of-line`, `cut-back` og `overlaps`.
Auðkennin eru lofuð; setningarnar ekki.

## Dæmi

```bash
subedit-cli sync --video film.mkv --model small.bin --vad-model silero.bin \
    --report to-check.txt --output film.synced.srt film.srt
```

## Skilakóðar

Þeir sömu og hjá tólinu: `0` ef skráin var skrifuð, hvort sem skjátextar voru merktir eða ekki, `2` ef það var ekki hægt — tól eða líkan vantar, misheppnuð útdráttur eða hlustun, enginn skjátexti fannst —, `1` við notkunarvillu, eins og hvorki
`--video` né `--audio` (eða bæði), valkostur utan marka sinna, eða bil sem ekki er hægt að nota.

## Villur

| Orsök | Skilaboð |
| :---- | :------ |
| hvorki `--video` né `--audio`, eða bæði | `sync needs the audio to listen to: give --video or --audio, and not both` |
| ekkert vélarforrit fannst, eða það sem var stillt er ekki til | `the transcription engine was not found`, svo tilvísunin í leiðbeiningarnar |
| ekkert líkan gefið, eða það er ekki til | `no model was given, or the model file is not there`, svo tilvísunin í leiðbeiningarnar |
| `ffmpeg` fannst ekki fyrir myndskeið | `ffmpeg was not found, so the audio of a video cannot be extracted`, svo tilvísunin |
| rás sem myndskeiðið hefur ekki | `the video has no audio track N`, svo rásirnar sem það hefur |
| tungumál sem líkanið getur ekki umritað | `the model cannot transcribe the language xx` |
| vél sem enginn skráði | `no transcription engine is called NAME`, eða `there is no alignment engine called NAME` |
| enginn skjátexti fannst | `none of the N subtitles was found in the audio: is it the right audio, and the right language?` |
| `--min-match` utan 1 til 100, `--doubt-gap` neikvætt | notkunarvilla valkostsins |
| `--report` með fleiri en einni skjátextaskrá | `--report names one file: give one subtitle file when you use it` |
