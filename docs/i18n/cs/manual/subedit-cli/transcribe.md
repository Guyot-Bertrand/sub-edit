# `transcribe`

<!-- machine-translated -->
> Tato stránka je strojovým překladem a dosud ji nezkontroloval žádný čtenář tohoto jazyka.

```
subedit-cli transcribe --output FILE
                       [--to FORMAT]
                       [--audio-track N]
                       [--language CODE]
                       [--start TIME] [--end TIME]
                       [--model FILE]
                       [--engine NAME]
                       [--keep-audio FILE]
                       [--raw]
                       <input>
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
- **Část filmu lze přepsat samostatně** pomocí `--start` a `--end` — viz
  [Přepis části](#přepis-části). Tak se model vyzkouší na dvou minutách, než se pustí na dvě
  hodiny.
- **Segmenty se sloučí a rozdělí** podle mezí nastavení (`transcription.merge-gap-ms`,
  `transcription.max-duration-ms`, `transcription.max-characters`): dva kusy jedné věty, které jsou
  blízko sebe, tvoří jeden titulek, a kus příliš dlouhý nebo příliš široký se rozdělí u slova. **`--raw`
  zapíše segmenty nástroje tak, jak přišly**, jeden na titulek, což je také způsob, jak vidět, co
  slučování dělá.

<!-- example: subedit-cli transcribe --help -->
```console
$ subedit-cli transcribe --help
Write the speech of a video or an audio file as subtitles
Usage: subedit-cli transcribe [OPTIONS] input

Positionals:
  input TEXT REQUIRED         Video or audio file to transcribe

Options:
  -h,--help                   Print this help message and exit
  --output FILE               Subtitle file to write
  --to TEXT:{srt,vtt,subviewer2,ssa,ass,microdvd,mpl2,tmplayer,lrc}
                              Format to write; the one the extension of --output says by default
  --audio-track N             Audio track of a video, counted from 0; the first by default
  --start TIME                Where to begin, as 2:00, 1:02:03 or seconds; the beginning by default
  --end TIME                  Where to stop, as 2:00, 1:02:03 or seconds; the end by default
  --language CODE             Language of the speech: fr, en; detected by default
  --model FILE                Model file; the transcription.model setting by default
  --engine NAME               Engine; the transcription.engine setting by default
  --keep-audio FILE           Keep the extracted audio in this file
  --raw                       Write the segments as the engine cut them
```

## Argumenty a volby

| Volba | Povinná | Hodnoty | Výchozí |
| :----- | :------- | :----- | :------ |
| `<input>` | ano | video nebo zvukový soubor | — |
| `--output` | ano | soubor titulků k zapsání; jeho adresář se vytvoří, pokud chybí | — |
| `--to` | ne | krátký název formátu: `srt`, `vtt`, `subviewer2`, `ssa`, `ass`, `microdvd`, `mpl2`, `tmplayer`, `lrc` | formát, který určuje přípona `--output`, jinak `srt` |
| `--audio-track` | ne | celé číslo od 0: pořadí mezi zvukovými stopami videa | `0`, první |
| `--language` | ne | dvě malá písmena, kód ISO 639-1: `en`, `fr` | nastavení `transcription.language`; prázdné: zjistí se |
| `--start` | no | čas: `2:00`, `1:02:03`, `2:00.5`, nebo počet sekund, `90` | začátek zvuku |
| `--end` | no | čas zapsaný jako u `--start`; musí následovat po `--start` | konec zvuku |
| `--model` | ne | soubor modelu | nastavení `transcription.model` |
| `--engine` | ne | název nástroje: `whisper.cpp` | nastavení `transcription.engine` |
| `--keep-audio` | ne | soubor, ve kterém se uchová vyextrahovaný zvuk, jako 16bitové mono PCM s frekvencí 16 kHz v souboru WAV; jeho adresář se vytvoří, pokud chybí | zvuk se neuchovává nikde |
| `--raw` | ne | přepínač: zapíše segmenty nástroje bez slučování a bez dělení | vypnuto |

**Nastavení** se čtou ze souboru nastavení jako ta u [`adjust`](adjust.md) — viz
[soubor nastavení](invocation.md#soubor-nastavení) — a každé má zde svou volbu:

| Klíč | Význam | Výchozí |
| :-- | :------ | :------ |
| `transcription.engine` | nástroj | `whisper.cpp` |
| `transcription.executable` | program nástroje, absolutní cesta; prázdné: hledá se v `PATH` | prázdné |
| `transcription.model` | soubor modelu, absolutní cesta | prázdné |
| `transcription.language` | jazyk zvuku, dvě písmena; prázdné: zjistí se | prázdné |
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

## Výstup

**Standardní výstup** — nic: výsledkem je zapsaný soubor.

| Úroveň | Co se přidá na chybový výstup |
| :---- | :-------------------------------- |
| 1 | `<input>: N subtitles transcribed, language: xx -> <output>` |
| 2 | krok a průběh, jeden řádek na každé procento: `extracting the audio…`, pak `transcribing…`, pak `transcribing: 25%, 0:12:30 of 0:50:00, about 38 min left` — procento, dosažená pozice ve zvuku a zbývající čas |
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
| mez rozsahu, která není čas | `"TEXT" is not a time: write it as 2:00, 1:02:03 or 2:00.5, or in seconds` |
| `--end` nenásleduje po `--start` | `the end of the range must come after its start` |
| jakékoli jiné selhání | `the transcription failed`, s tím, co nástroj řekl |

Odkaz na průvodce je `See the manual page "Installing the transcription tools"
(install-transcription).`
