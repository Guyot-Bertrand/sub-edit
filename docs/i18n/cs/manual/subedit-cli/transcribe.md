# `transcribe`

<!-- machine-translated -->
> Tato stránka je strojovým překladem a dosud ji nezkontroloval žádný čtenář tohoto jazyka.

```
subedit-cli transcribe --output FILE
                       [--to FORMAT]
                       [--audio-track N]
                       [--language CODE]
                       [--start TIME] [--end TIME]
                       [--suppress-notes | --keep-notes]
                       [--model FILE]
                       [--engine NAME]
                       [--keep-audio FILE]
                       [--raw]
                       <input>

subedit-cli transcribe --list-languages [--model FILE] [--engine NAME]
```

Zapíše **řeč videa nebo zvukového souboru jako soubor titulků**. Zvuk se připraví,
předá nástroji, který zjistí, co se říká a kdy, a kusy, které nástroj vytvoří, se sloučí do
titulků, jež dodrží danou dobu trvání a délku.

**Nic z toho, co potřebuje, se s programem nedodává**: nástroj (whisper.cpp), model a `ffmpeg`
si nainstaluje uživatel, který programu řekne, kde model leží. [Instalace nástrojů pro
přepis](install-transcription.md) říká, kde každý z nich získat a jaký model zvolit. **Jejich nepřítomnost
nic nerozbije**: příkaz řekne, co chybí, s odkazem na tuto stránku, nic nezapíše
a skončí s nenulovým kódem.

- **Vstupem je video nebo zvukový soubor.** Z videa `ffmpeg` vyextrahuje jednu zvukovou stopu; bez
  `ffmpeg` se zvukový soubor, který nástroj čte tak, jak je (`wav`, `mp3`, `flac`, `ogg`), přesto
  přepíše, ale video ne, a ani žádný soubor, když se požaduje stopa.
- **Vždy jen jeden soubor** a cílem zápisu je `--output`: neexistuje `--output-dir` ani
  `--in-place`, protože výsledkem je nový soubor, a ne přepsání.
- **Jazyk se zjistí**, pokud ho `--language` nevynutí, a použitý jazyk se vypíše. Model
  určený jen pro angličtinu nic nezjistí a nepřepíše nic jiného: jazyk, který nemůže přijmout, se odmítne
  *před* zahájením jakékoli práce a uvede se mez modelu.
- **Jazyk se zapisuje dvěma písmeny — `fr`, `en`, `de` —, nebo třípísmenným kódem, který soubor uvádí, `fra`,
  `eng`, a ten se čte jako dvoupísmenný.** Cokoli jiného se odmítne před zahájením jakékoli práce. **Zjišťování
  poslouchá jen prvních třicet sekund**: film, který začíná hudbou nebo tichem, může být považován za
  jiný jazyk a nástroj pak zapíše řeč *přeloženou* do tohoto jazyka — francouzský film považovaný
  za anglický vyjde anglicky, nebo jako `[SPEAKING FRENCH]`. Když jazyk znáte, zadejte ho.
- **Část filmu lze přepsat samostatně** pomocí `--start` a `--end` — viz
  [Přepis části](#přepis-části). Tak se model vyzkouší na dvou minutách, než se pustí na dvě
  hodiny.
- **Jazyky, které nástroj nabízí, lze vypsat** pomocí `--list-languages` — viz
  [Výpis jazyků](#výpis-jazyků).
- **Zvuky se ve výchozím stavu nezapisují jako poznámky**: nástroj má příkaz to nedělat (`--suppress-notes`; jeho vlastní volba je
  `--suppress-nst`), takže `(Sonnerie de téléphone)` nebo `[MUSIC PLAYING]` se v titulcích neobjeví, ani v závorkách, ani v hranatých závorkách.
  `--keep-notes` je zapíše, pro titulky pro neslyšící a nedoslýchavé. Když jsou poznámky zakázány, nástroj zapíše `...` pro úsek bez řeči
  a **následné zpracování zahodí kousky, které nemají žádné písmeno ani číslici** — `...`, `…`, `♪` —
  před slučováním a dělením; `--raw` je ponechá tak, jak kousky nástroje přišly.
- **Segmenty se sloučí a rozdělí** podle mezí nastavení (`transcription.merge-gap-ms`,
  `transcription.max-duration-ms`, `transcription.max-characters`): dva kusy jedné věty, které jsou
  blízko sebe, tvoří jeden titulek, a kus příliš dlouhý nebo příliš široký se rozdělí — nejprve na konci věty, pak u čárky, středníku nebo dvojtečky a uprostřed věty jen tehdy, když není interpunkce, u které by se dalo dělit —, přičemž se vyhne řezu, který zanechá titulek příliš malý na čtení. Tři kroky probíhají za sebou, každý nad celým přepisem: zahození, sloučení, rozdělení. Titulek, který kousek začíná, si ponechá jeho začátek, a ten, který ho končí, jeho konec. **`--raw`
  zapíše segmenty nástroje tak, jak přišly**, jeden na titulek, což je také způsob, jak vidět, co
  slučování dělá.

<!-- example: subedit-cli transcribe --help -->
```console
$ subedit-cli transcribe --help
Write the speech of a video or an audio file as subtitles
Usage: subedit-cli transcribe [OPTIONS] [input]

Positionals:
  input TEXT                  Video or audio file to transcribe

Options:
  -h,--help                   Print this help message and exit
  --output FILE               Subtitle file to write
  --to TEXT:{srt,vtt,subviewer2,ssa,ass,microdvd,mpl2,tmplayer,lrc}
                              Format to write; the one the extension of --output says by default
  --audio-track N             Audio track of a video, counted from 0; the first by default
  --start TIME                Where to begin, as 2:00, 1:02:03 or seconds; the beginning by default
  --end TIME                  Where to stop, as 2:00, 1:02:03 or seconds; the end by default
  --language CODE             Language of the speech, two letters: fr, en, de; detected from the first 30 seconds by default
  --suppress-notes            Do not write sounds as notes, such as (phone ringing); the transcription.suppress-notes setting by default
  --keep-notes                Write sounds as notes, against the setting
  --model FILE                Model file; the transcription.model setting by default
  --engine NAME               Engine; the transcription.engine setting by default
  --keep-audio FILE           Keep the extracted audio in this file
  --list-languages            List the languages the engine offers for the model, and transcribe nothing
  --raw                       Write the segments as the engine cut them
```

## Argumenty a volby

| Volba | Povinná | Hodnoty | Výchozí |
| :----- | :------- | :----- | :------ |
| `<input>` | ano, kromě `--list-languages` | video nebo zvukový soubor | — |
| `--output` | ano, kromě `--list-languages` | soubor titulků k zapsání; jeho adresář se vytvoří, pokud chybí | — |
| `--to` | ne | krátký název formátu: `srt`, `vtt`, `subviewer2`, `ssa`, `ass`, `microdvd`, `mpl2`, `tmplayer`, `lrc` | formát, který určuje přípona `--output`, jinak `srt` |
| `--audio-track` | ne | celé číslo od 0: pořadí mezi zvukovými stopami videa | `0`, první |
| `--language` | ne | dvě písmena, kód ISO 639-1: `en`, `fr`; třípísmenný kód, například `fra`, se čte jako `fr` | nastavení `transcription.language`; prázdné: zjistí se |
| `--suppress-notes` | ne | příznak: nezapisovat zvuky jako poznámky; odmítnuto u nástroje, kterému to nelze říci | nastavení `transcription.suppress-notes` |
| `--keep-notes` | ne | příznak: zapsat je, proti nastavení; opak `--suppress-notes`, a obě společně se odmítnou | nastavení `transcription.suppress-notes` |
| `--list-languages` | ne | příznak: vypsat jazyky, které nástroj nabízí pro model, a nic nepřepisovat | vypnuto |
| `--start` | no | čas: `2:00`, `1:02:03`, `2:00.5`, nebo počet sekund, `90` | začátek zvuku |
| `--end` | no | čas zapsaný jako u `--start`; musí následovat po `--start` | konec zvuku |
| `--model` | ne | soubor modelu | nastavení `transcription.model` |
| `--engine` | ne | název nástroje: `whisper.cpp` | nastavení `transcription.engine` |
| `--keep-audio` | ne | soubor, ve kterém se uchová vyextrahovaný zvuk, jako 16bitové mono PCM v souboru WAV, s vzorkovací frekvencí, kterou nástroj žádá (16 kHz pro whisper.cpp); jeho adresář se vytvoří, pokud chybí, a název může nést `{rate}`, nahrazené touto frekvencí v hertzích (`film.{rate}.wav` se zapíše jako `film.16000.wav`) | zvuk se neuchovává nikde |
| `--raw` | ne | přepínač: zapíše segmenty nástroje bez slučování a bez dělení | vypnuto |

**Nastavení** se čtou ze souboru nastavení jako ta u [`adjust`](adjust.md) — viz
[soubor nastavení](invocation.md#soubor-nastavení) — a každé má zde svou volbu:

| Klíč | Význam | Výchozí |
| :-- | :------ | :------ |
| `transcription.engine` | nástroj | `whisper.cpp` |
| `transcription.executable` | program nástroje, absolutní cesta; prázdné: hledá se v `PATH` | prázdné |
| `transcription.model` | soubor modelu, absolutní cesta | prázdné |
| `transcription.language` | jazyk zvuku, dvě písmena; prázdné: zjistí se | prázdné |
| `transcription.suppress-notes` | nezapisovat zvuky jako poznámky | `true` |
| `transcription.merge-gap-ms` | části jedné věty bližší než tento počet milisekund se spojí | `400` |
| `transcription.max-duration-ms` | nejdelší doba, po kterou titulek zůstává na obrazovce, v milisekundách | `7000` |
| `transcription.max-characters` | největší počet znaků titulku, zalomení řádků se nepočítají | `84` |

## Volba zvuku

Video může nést několik zvukových stop — originál, komentář, jiný jazyk. `--audio-track`
bere **pořadí mezi zvukovými stopami** souboru, od 0, v pořadí, v jakém je soubor uvádí. Stopa,
kterou video nemá, se odmítne, **spolu se stopami, které má**, aby další volání
bylo správné:

```bash
subedit-cli transcribe --audio-track 4 --output film.srt --model small.bin film.mkv
```

```text
film.mkv: the video has no audio track 4
audio track 0: fra, Original
audio track 1: eng, Commentary
```

## Přepis části

`--start` a `--end` omezí práci na část filmu mezi nimi. Lze zadat jen jedno: samotné
`--start` jde až do konce, samotné `--end` začíná od začátku.

```bash
subedit-cli transcribe --start 2:00 --end 3:30 --output test.srt --model large-v3.bin film.mkv
```

- **Čas se píše jako `M:SS`, `H:MM:SS`, obojí s desetinnou částí za tečkou (`2:00.5`), nebo jako počet
  sekund (`90`, `90.5`)**. Čárka se odmítne, jako všude v nástroji.
- **Titulky zachovají čas filmu**: věta řečená ve filmu v 2:10 se zapíše na `00:02:10`, ne
  na `00:00:10`. Soubor lze přiložit na celý film tak, jak je.
- **Okraje jsou přibližné**: nástroj pracuje s okny po třiceti sekundách, takže to, co vrátí, se
  poté ořízne na rozsah; věta, která přesahuje okraj, se na něm přeřízne.
- **Nic nezačne, pokud rozsah nelze použít**: mez, která není čas, nebo `--end`, které nenásleduje po
  `--start`, je chyba použití, která problém pojmenuje, nic nezapíše a nespustí ani `ffmpeg`, ani
  nástroj.

Zvuk celého filmu se stále extrahuje: rozsah ušetří přepis, který je mnohem delší.

## Výpis jazyků

`--list-languages` vypíše jazyky, které nástroj nabízí **pro model**, jeden na řádek: kód, který bere `--language`, tabulátor a název
jazyka v jazyce rozhraní. Nepotřebuje ani vstup, ani výstup a nic nepřepisuje.

```bash
subedit-cli transcribe --list-languages --model ggml-small.bin
```

```text
en	English
zh	Chinese
de	German
…
```

Seznam závisí na nástroji a na modelu: asi sto jazyků pro vícejazyčný model whisper.cpp, jen `en` pro model pouze pro angličtinu. Nástroj, který své jazyky vypsat nemůže, nevypíše žádný. Nezadaný nebo neexistující model je chyba s odkazem na průvodce; pouze
textová podoba, s [`--format json`](invocation.md#výstup-čitelný-pro-skript) se nezapisuje nic.

## Výstup

**Standardní výstup** — nic: výsledkem je zapsaný soubor.

| Úroveň | Co se přidá na chybový výstup |
| :---- | :-------------------------------- |
| 1 | `<input>: N subtitles transcribed, language: xx -> <output>` |
| 2 | krok a průběh, jeden řádek na každé procento: `extracting the audio…`, pak `transcribing…`, pak `transcribing: 25%, 0:12:30 of 0:50:00, about 38 min left, elapsed: 0:05:12` — procento, dosažená pozice ve zvuku, zbývající čas a uplynulý čas; **řádek se zopakuje každých deset sekund, když nepřišlo nic nového**, aby dlouhé čekání nebylo považováno za zamrznutí |
| 3 | a každý nový text, který nástroj získá, `last text: …` |

**Procento je pozice nástroje ve zvuku, ne počet jeho oken**: nástroj průběžně zapisuje, co
přepsal, a příkaz čte, kam to došlo, takže číslo se hýbe od první
minuty. (Vlastní průběh nástroje přichází v krocích po 5 % zvuku, po dokončení okna o třiceti sekundách:
samotný může u dlouhého filmu s velkým modelem na procesoru zůstat na 0 % čtvrt hodiny.) **Zbývající
čas zpočátku mlčí**: uvede se, jakmile je hotovo dost práce pro extrapolaci, a řídí se rychlostí
poslední minuty, ne průměrem celého běhu.

### Ve formátu JSON

S [`--format json`](invocation.md#výstup-čitelný-pro-skript) (globální volba, zapsaná před
podpříkazem) nese objekt souboru použitý jazyk v `language`, `counts.subtitles` (zapsané
titulky) a `counts.segments` (kusy nástroje, z nichž vznikly) a `audio`, když
`--keep-audio` zvuk uchoval.

## Upozornění

Jedno upozornění se vypisuje na první úrovni, před prací, protože mění, co lze čekat:

- **S modelem jen pro angličtinu**: `This model is for English only: whatever language the film
  speaks, the result will be English, or notes in brackets. Choose a multilingual model in the preferences.` Takovým
  modelem je ten, jehož název souboru končí na `.en`.

## Návratové kódy

Kódy nástroje: `0`, pokud byl soubor zapsán, `2`, pokud se zapsat nepodařilo — chybí nástroj nebo model,
jazyk, který model nepřijme, selhala extrakce nebo přepis —, `1` při chybě použití, například u
nástroje, který nikdo nezaregistroval, nebo u rozsahu, který nelze použít.

## Chyby

| Příčina | Hlášení |
| :---- | :------ |
| nenalezen žádný program nástroje, nebo nastavený neexistuje | `the transcription engine was not found`, pak odkaz na průvodce |
| nezadán žádný model, nebo neexistuje | `no model was given, or the model file is not there`, pak odkaz na průvodce |
| `ffmpeg` nenalezen pro video | `ffmpeg was not found, so the audio of a video cannot be extracted`, pak odkaz |
| stopa, kterou video nemá | `the video has no audio track N`, pak stopy, které má |
| jazyk, který model neumí přepisovat | `the model cannot transcribe the language xx` |
| žádný jazyk a model, který žádný nezjistí | `the model cannot detect the language: give one` |
| nástroj, který nikdo nezaregistroval | `no transcription engine is called NAME` |
| chybějící `<input>` nebo `--output` | `input is required` nebo `--output is required` |
| `--suppress-notes` s `--keep-notes` | `--suppress-notes and --keep-notes say opposite things; give one of them` |
| `--suppress-notes` u nástroje, kterému to nelze říci | `the engine NAME cannot be told not to write sounds as notes` |
| jazyk, který není kód | `"TEXT" is not a language code: write two letters, such as fr, en, de or es` |
| mez rozsahu, která není čas | `"TEXT" is not a time: write it as 2:00, 1:02:03 or 2:00.5, or in seconds` |
| `--end` nenásleduje po `--start` | `the end of the range must come after its start` |
| jakékoli jiné selhání | `the transcription failed`, s tím, co nástroj řekl |

Odkaz na průvodce je `See the manual page "Installing the transcription tools"
(install-transcription).`
