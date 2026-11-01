# `sync`

<!-- machine-translated -->
> Tato stránka je strojovým překladem a dosud ji nezkontroloval žádný čtenář tohoto jazyka.

```
subedit-cli sync (--video FILE | --audio FILE)
                 [--audio-track N]
                 [--language CODE]
                 [--start TIME] [--end TIME]
                 [--model FILE]
                 [--vad-model FILE]
                 [--engine NAME]
                 [--keep-audio FILE]
                 [--method METHOD]
                 [--min-match PERCENT]
                 [--doubt-gap MS]
                 [--report FILE]
                 [-r]
                 [--output FILE | --output-dir DIR | --in-place] [--dry-run]
                 <files>...
```

**Přečasuje soubor titulků podle zvuku filmu.** Existuje pro případ souboru `.srt` staženého z internetu,
posunutého na začátku i na konci, který jediný posun nevrátí na místo. Dělají to dvě metody a mezi nimi vybírá
`--method` (nebo nastavení `alignment.method`):

- **`speech`, výchozí, položí soubor na místa, kde se mluví.** Ptá se zvuku jen na to, kde je řeč, a hledá posun,
  při němž titulky s ní souhlasí, úsek filmu po úseku: stálý posun, drift, opožděná druhá polovina i soubor načasovaný
  pro jinou snímkovou frekvenci (25 místo 23.976 a podobně) se opraví. **Na jazyce nezávisí** — titulky mohou být
  francouzsky a film anglicky —, nepotřebuje žádný přepis a trvá vteřiny. Potřebuje `ffmpeg`, program
  `whisper-vad-speech-segments`, který je součástí whisper.cpp, a model detekce řeči (`--vad-model`, nastavení
  `transcription.vad-model`).
- **`words` porovnává slova každého titulku se slovy, která nástroj slyšel.** Poslouchá přepisovacím nástrojem —
  stejným jako [`transcribe`](transcribe.md) — a každý titulek zaujme místo, kde se říkají jeho slova. Nic se
  nepřekládá: titulky musí být v jazyce, kterým se mluví. Umí umístit i jediný titulek, který je mimo, v souboru, který
  je jinak správně, a trvá tolik, co přepis.

**Co `speech` nedělá, se řekne při každém běhu**, na chybovém výstupu i v záznamu: opravuje soubor **jako celek**,
nikdy titulek po titulku. Titulek, který je sám o sobě mimo v souboru, který je jinak správně, nebo titulek zobrazený
nad tichem (karta kapitoly, popisek překládající něco na obrazovce), se nehledá a může být stále špatně a první
minuty mohou zůstat mimo, když film začíná hudbou nebo titulky. Zkontrolujte výsledek, nebo použijte `--method words`,
když je text v jazyce, kterým se mluví.

**Nic z toho, co potřebuje, se s programem nedodává**: programy, model a `ffmpeg` jsou ty z
[Instalace nástrojů pro přepis](install-transcription.md) a jejich nepřítomnost nic nerozbije — příkaz řekne,
co chybí, s odkazem na tuto stránku, nic nezapíše a skončí s nenulovým kódem.

- **Mění se jen začátky a konce.** Nikdy text, styl, počet titulků ani jejich pořadí.
- **Titulek, který zvuk nedá, si ponechá své časy** — titulky, které nikdo neříká, píseň, řádek v jiném jazyce.
  **Žádná pozice se nikdy nevymýšlí a žádná se neodhaduje podle sousedů**: řádek zůstane tam, kde ho soubor měl,
  a příkaz to řekne. *Není* to selhání a návratový kód je `0`.
- **U `words` se titulek považuje za nalezený, když se slyší alespoň polovina jeho slov** (`--min-match`,
  `alignment.min-match`), a alespoň jedno. Velká a malá písmena, diakritika, interpunkce a značky nevadí; slovo, které
  se od dlouhého slova liší o jedno písmeno, se počítá jako slyšené. `speech` umístí každý titulek a nebere žádnou
  z voleb `--language`, `--model`, `--engine` ani `--min-match`.
- **Konec je tam, kde končí poslední slovo.** Nástroj časuje slova přibližně, o několik setin sekundy před vlastním
  koncem titulku, takže titulek může vyjít o něco kratší, než byl.
- **Titulek, který by zasáhl do dalšího, se zkrátí tam, kde další začíná.** Ten, který se s dalším stále překrývá,
  což zkrácení nevyřeší, se ponechá a oznámí.
- **Soubor, který k zvuku nesedí, se odmítne a nezapíše**: u `words` nebyl nalezen žádný z titulků, což je téměř vždy
  špatný zvuk nebo špatný jazyk — `speech` na jazyce nezávisí; u `speech` mají titulky a řeč příliš málo společného,
  což je téměř vždy špatný film.
- **Část filmu lze poslouchat samostatně** pomocí `--start` a `--end`, jako u
  [`transcribe`](transcribe.md#přepis-části): hledají se a mění jen titulky, které v rozsahu **začínají**, ostatní
  zůstanou, jak jsou, a nepočítají se.
- **Zvuk se poslouchá jednou pro každý zadaný soubor.** Při přečasování několika souborů podle stejného filmu
  počítejte s čekáním u každého z nich.

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
  --method METHOD             How the subtitles are put in line: speech (from when there is speech, whatever the language) or words (from the words heard); the alignment.method setting by default
  --min-match PERCENT         Share of a subtitle's words that must be heard, in percent; the alignment.min-match setting by default
  --doubt-gap MS              How far a subtitle may move from its neighbours' shift before it is flagged, in milliseconds; the alignment.doubt-gap setting by default
  --report FILE               Write the subtitles to check to this file
  --output TEXT               File to write, for a single input
  --output-dir TEXT           Directory to write into
  --in-place                  Write back over the inputs
  --dry-run                   Work out and say what would be written, and write nothing
```

## Argumenty a volby

| Volba | Povinná | Hodnoty | Výchozí |
| :----- | :------- | :----- | :------ |
| `<files>` | ano | soubory titulků k přečasování; s `-r` adresáře | — |
| `--recursive`, `-r` | ne | přepínač | vypnuto — viz [Zpracování stromu](lots.md) |
| `--video` | jedna z voleb `--video` a `--audio` | film, který se má poslouchat | — |
| `--audio` | jedna z voleb `--video` a `--audio` | zvukový soubor k poslechu místo filmu | — |
| `--audio-track` | ne | celé číslo od 0: pořadí mezi zvukovými stopami filmu | `0`, první |
| `--language` | ne | dvě písmena, kód ISO 639-1: `en`, `fr` | nastavení `transcription.language`; prázdné: zjistí se |
| `--start`, `--end` | ne | čas: `2:00`, `1:02:03`, `2:00.5`, nebo počet sekund; `--end` musí následovat po `--start` | celý zvuk |
| `--model` | ne | soubor modelu | nastavení `transcription.model` |
| `--vad-model` | ne | model detekce řeči, který způsobí, že nástroj přeskakuje ticho; s ním slova věrněji sledují řeč | nastavení `transcription.vad-model` |
| `--engine` | ne | název nástroje, který poslouchá: `whisper.cpp` | nastavení `transcription.engine` |
| `--keep-audio` | ne | soubor, ve kterém se uchová vyextrahovaný zvuk; jeho adresář se vytvoří, pokud chybí | zvuk se neuchovává nikde |
| `--method` | ne | `speech`: položí soubor na místa, kde se mluví, v jakémkoli jazyce; `words`: porovná slova s tím, co bylo slyšet, v mluveném jazyce | nastavení `alignment.method`, `speech` |
| `--min-match` | ne | celé číslo od 1 do 100: podíl slov titulku, která se musí slyšet, v procentech | nastavení `alignment.min-match`, `50` |
| `--doubt-gap` | ne | celý počet milisekund, 0 nebo více: o kolik se posun titulku smí lišit od posunu sousedů, než se označí | nastavení `alignment.doubt-gap`, `2000` |
| `--report` | ne | soubor, do něhož se zapíší titulky ke kontrole, jeden na řádek: číslo titulku, tabulátor a důvod; vždy jen jeden soubor titulků | nic se nezapíše |
| `--output` / `--output-dir` / `--in-place` | **jedna ze tří**, kromě `--dry-run` | viz [Spuštění](invocation.md#cíl-zápisu) | — |
| `--dry-run` | ne | přepínač: spočítá a vypíše, nic nezapíše — viz [Náhled před zápisem](invocation.md#náhled-před-zápisem) | vypnuto |

Cíl zápisu — `--output`, `--output-dir`, `--in-place`, `--dry-run` — a `-r` jsou stejné jako u ostatních příkazů,
které přepisují soubor: viz [cíl zápisu](invocation.md#cíl-zápisu). Soubor si ponechá svůj formát, své kódování
a své konce řádků.

**Nastavení** se čtou ze souboru nastavení — viz [soubor nastavení](invocation.md#soubor-nastavení) — a každé
má zde svou volbu. Nástroj, model a model detekce řeči jsou ty z `transcribe`.

| Klíč | Význam | Výchozí |
| :-- | :------ | :------ |
| `alignment.engine` | nástroj zarovnání metody `words`; existuje jeden, `transcript-matching`, který poslouchá a porovnává | `transcript-matching` |
| `alignment.method` | jak se titulky srovnají: `speech` nebo `words` | `speech` |
| `alignment.min-match` | podíl slov titulku, která se musí slyšet, aby se považoval za nalezený, v procentech | `50` |
| `alignment.doubt-gap` | o kolik, v milisekundách, se posun titulku smí lišit od posunu sousedů, než se označí | `2000` |

## Co se označuje

Příkaz řekne, které titulky zkontrolovat a proč. Titulek se označí podle **prvního** z těchto důvodů, který platí:

| Proč | Význam |
| :-- | :------ |
| not found in the audio, left as it was | slyšelo se méně než `--min-match` jeho slov: ponechá si své časy |
| only partly heard | nalezen, ale slyšelo se méně než 80 % jeho slov |
| moved differently from its neighbours | nalezen, ale jeho posun se liší od prostředního posunu dvou titulků na každé straně o více než `--doubt-gap` |

Titulek může být také **zkrácen na začátek dalšího titulku** nebo **se s dalším titulkem stále překrývat**.

U `speech` se umístí každý titulek a může platit jen poslední z těchto důvodů: tam, kde soubor mění svůj posun o více
než `--doubt-gap` z jednoho titulku na další (opožděná druhá polovina), se označí titulky na obou stranách skoku.

Dvě čísla vypovídají o souboru jako celku. **Mediánový posun** je prostřední hodnota toho, o kolik se titulky
posunuly: soubor, který byl prostě opožděný, se pozná tady a nikde jinde. **Drift** je mediánový posun poslední
desetiny titulků minus posun první desetiny: u stálého posunu je blízko nule a velký u souboru, který se
během filmu opožďoval čím dál víc.

## Výstup

**Standardní výstup** — nic: výsledkem je zapsaný soubor.

| Úroveň | Co se řekne na chybovém výstupu |
| :---- | :------------------------------- |
| 1 | `<file>: N of M subtitles placed on the audio, K to check; median shift X s, drift Y s -> <destination>` — a když byla opravena snímková frekvence, poměr; u `speech` **upozornění na to, co nedělá**, na samostatném řádku |
| 2 | krok a průběh, jako u [`transcribe`](transcribe.md#výstup) |

### Ve formátu JSON

S [`--format json`](invocation.md#výstup-čitelný-pro-skript) nese objekt souboru `counts`: `subtitles`
(hledané), `placed` (kterým se dalo místo), `moved` (jejichž časy se změnily), `partial`, `out_of_line`, `not_found`,
`median_shift_ms` a `drift_ms`; a `flagged`, seznam `{"subtitle": N, "kind": "…"}`, kde číslo je
pořadí titulku v souboru a `kind` je jedno z `not-found`, `partial`, `out-of-line`, `cut-back` a `overlaps`.
U `speech` nese `warnings` jeden záznam s `kind` `speech-only`. Identifikátory jsou zaručeny; věty nikoli.

## Příklad

```bash
subedit-cli sync --video film.mkv --vad-model silero.bin \
    --report to-check.txt --output film.synced.srt film.srt
```

Chcete-li umístit každý titulek podle jeho slov, pro text v jazyce, kterým se mluví:

```bash
subedit-cli sync --method words --video film.mkv --model small.bin --vad-model silero.bin \
    --output film.synced.srt film.srt
```

## Návratové kódy

Kódy nástroje: `0`, pokud byl soubor zapsán, ať už s označenými titulky, nebo bez nich, `2`, pokud se zapsat nepodařilo — chybí
nástroj nebo model, selhala extrakce nebo poslech, soubor, který k zvuku nesedí (nenalezen žádný z jeho titulků, nebo příliš málo společného s řečí) —, `1` při chybě použití, například
když chybí `--video` i `--audio` (nebo jsou zadány obě), volba mimo svůj rozsah nebo rozsah, který nelze použít.

## Chyby

| Příčina | Hlášení |
| :---- | :------ |
| ani `--video`, ani `--audio`, nebo obě | `sync needs the audio to listen to: give --video or --audio, and not both` |
| nenalezen žádný program nástroje, nebo nastavený neexistuje | `the transcription engine was not found`, pak odkaz na průvodce |
| nezadán žádný model, nebo neexistuje | `no model was given, or the model file is not there`, pak odkaz na průvodce |
| `ffmpeg` nenalezen pro film | `ffmpeg was not found, so the audio of a video cannot be extracted`, pak odkaz |
| stopa, kterou film nemá | `the video has no audio track N`, pak stopy, které má |
| jazyk, který model neumí přepisovat | `the model cannot transcribe the language xx` |
| nástroj, který nikdo nezaregistroval | `no transcription engine is called NAME`, nebo `there is no alignment engine called NAME` |
| nenalezen žádný z titulků (`words`) | `none of the N subtitles was found in the audio: is it the right audio, and the right language?`, pak `The method based on speech does not depend on the language.` |
| model detekce řeči nezadán, nebo program, který hledá řeč, nenalezen (`speech`) | `Finding where there is speech needs a voice-activity model: name one in the transcription.vad-model setting.` nebo `Finding where there is speech needs the program whisper-vad-speech-segments, which comes with whisper.cpp, and it was not found.`, pak odkaz na průvodce |
| titulky a řeč mají příliš málo společného (`speech`) | `The sound and the subtitles have too little in common to be put in line: is it the right film?` |
| `--min-match` mimo 1 až 100, záporné `--doubt-gap` | chyba použití dané volby |
| `--report` s více než jedním souborem titulků | `--report names one file: give one subtitle file when you use it` |
