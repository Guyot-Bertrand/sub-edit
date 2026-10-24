# `transcribe`

<!-- machine-translated -->
> Ez az oldal gépi fordítás, amelyet a nyelv egyik olvasója még nem nézett át.

```
subedit-cli transcribe --output FILE
                       [--to FORMAT]
                       [--audio-track N]
                       [--language CODE]
                       [--model FILE]
                       [--engine NAME]
                       [--keep-audio FILE]
                       [--raw]
                       <input>
```

**Egy videó vagy egy hangfájl beszédét írja ki feliratfájlként.** A hangot előkészíti,
átadja egy motornak, amely megtalálja, mi hangzik el és mikor, a motor által felvágott darabokat pedig
olyan feliratokká vonja össze, amelyek betartanak egy időtartamot és egy hosszt.

**Semmit, amire szüksége van, nem szállít a program**: a motort (whisper.cpp), egy modellt és az `ffmpeg`-et
a felhasználó telepíti, és ő mondja meg, hol van a modell. [A leiratozó
eszközök telepítése](install-transcription.md) megmondja, hol található mindegyik, és melyik modellt érdemes választani. **Hiányuk
nem tör el semmit**: a parancs megmondja, mi hiányzik, az oldalra mutató hivatkozással, nem ír semmit,
és nullától különböző kóddal lép ki.

- **A bemenet egy videó vagy egy hangfájl.** Videóból az `ffmpeg` egy hangsávot nyer ki; `ffmpeg` nélkül
  egy olyan hangfájlt, amelyet a motor magában is beolvas (`wav`, `mp3`, `flac`, `ogg`), még le lehet
  iratozni, de videót nem, és semmilyen fájlt sem, ha hangsávot kérnek.
- **Egyszerre csak egy fájl**, és a cél az `--output`: nincs `--output-dir` vagy
  `--in-place`, mert az eredmény új fájl, nem átírás.
- **A nyelvet felismeri**, hacsak a `--language` ki nem kényszeríti, és a használt nyelvet kiírja. Egy
  csak angolra készült modell semmit sem ismer fel, és mást nem iratoz le: a nyelvet, amelyet nem tud
  fogadni, *még a munka megkezdése előtt* elutasítja, megnevezve a modell korlátját.
- **A szakaszokat egyesíti és vágja** a beállítások korlátai szerint (`transcription.merge-gap-ms`,
  `transcription.max-duration-ms`, `transcription.max-characters`): egy mondat két közeli darabja
  egy felirat, a túl hosszú vagy túl széles darabot pedig szónál vágja el. **A `--raw`
  úgy írja ki a motor szakaszait, ahogy megjöttek**, szakaszonként egy feliratot, és így látható az is, mit
  csinál az egyesítés.

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
  --language CODE             Language of the speech: fr, en; detected by default
  --model FILE                Model file; the transcription.model setting by default
  --engine NAME               Engine; the transcription.engine setting by default
  --keep-audio FILE           Keep the extracted audio in this file
  --raw                       Write the segments as the engine cut them
```

## Argumentumok és kapcsolók

| Kapcsoló | Kötelező | Értékek | Alapértelmezés |
| :----- | :----- | :------ | :----- |
| `<input>` | igen | egy videó vagy egy hangfájl | — |
| `--output` | igen | a kiírandó feliratfájl; a könyvtára létrejön, ha hiányzik | — |
| `--to` | nem | egy formátum rövid neve: `srt`, `vtt`, `subviewer2`, `ssa`, `ass`, `microdvd`, `mpl2`, `tmplayer`, `lrc` | az `--output` kiterjesztése által megadott formátum, különben `srt` |
| `--audio-track` | nem | 0-tól induló egész szám: a hely a videó hangsávjai között | `0`, az első |
| `--language` | nem | két kisbetű, egy ISO 639-1 kód: `en`, `fr` | a `transcription.language` beállítás; üres: felismeri |
| `--model` | nem | a modellfájl | a `transcription.model` beállítás |
| `--engine` | nem | egy motor neve: `whisper.cpp` | a `transcription.engine` beállítás |
| `--keep-audio` | nem | egy fájl, amelyben a kinyert hang megmarad, 16 kHz-es mono 16 bites PCM-ként egy WAV-fájlban; a könyvtára létrejön, ha hiányzik | a hang sehol sem marad meg |
| `--raw` | nem | kapcsoló: a motor szakaszainak kiírása egyesítés és vágás nélkül | ki |

A **beállítások** a beállításfájlból olvasódnak, mint az [`adjust`](adjust.md) beállításai — lásd
[A beállításfájl](invocation.md#a-beállításfájl) —, és mindegyiknek itt megvan a saját kapcsolója:

| Kulcs | Jelentés | Alapértelmezés |
| :-- | :------ | :------ |
| `transcription.engine` | a motor | `whisper.cpp` |
| `transcription.executable` | a motor programja, abszolút elérési út; üres: a `PATH`-ban keresi | üres |
| `transcription.model` | a modellfájl, abszolút elérési út | üres |
| `transcription.language` | a hang nyelve, két betű; üres: felismeri | üres |
| `transcription.merge-gap-ms` | egy mondat ennyi ezredmásodpercnél közelebbi darabjai egyesülnek | `400` |
| `transcription.max-duration-ms` | a leghosszabb idő, amíg egy felirat a képernyőn marad, ezredmásodpercben | `7000` |
| `transcription.max-characters` | egy felirat legtöbb karaktere, a sortöréseket nem számítva | `84` |

## A hang kiválasztása

Egy videó több hangsávot is hordozhat — az eredetit, egy kommentárt, egy másik nyelvet. Az `--audio-track`
a **hangsávok közötti helyet** várja a fájlban, 0-tól, abban a sorrendben, ahogy a fájl felsorolja őket.
A videóban nem létező hangsávot elutasítja, **a meglévőkkel együtt**, hogy a következő hívás
a helyes legyen:

```bash
subedit-cli transcribe --audio-track 4 --output film.srt --model small.bin film.mkv
```

```text
film.mkv: the video has no audio track 4
audio track 0: fra, Original
audio track 1: eng, Commentary
```

## Kimenet

**Szabványos kimenet** — semmi: az eredmény a kiírt fájl.

| Szint | Mi kerül még a hibakimenetre |
| :---- | :-------------------------------- |
| 1 | `<input>: N subtitles transcribed, language: xx -> <output>` |
| 2 | a motor előrehaladása, `transcribing: 25%`, ahogy jelenti — nagyjából harminc másodpercnyi hangonként |

### JSON-ban

A [`--format json`](invocation.md#szkript-által-olvasható-kimenet) kapcsolóval (a globális kapcsoló, az
alparancs előtt írva) a fájl objektuma a használt nyelvet a `language` mezőben, a `counts.subtitles`
(a kiírt feliratok) és a `counts.segments` (a motor darabjai, amelyekből készültek) mezőt, valamint
az `audio` mezőt hordozza, ha a `--keep-audio` megőrizte a hangot.

## Kilépési kódok

Az eszköz kódjai: `0`, ha a fájl kiíródott, `2`, ha nem sikerült — hiányzó eszköz vagy modell, a modell
által nem fogadható nyelv, sikertelen kinyerés vagy leiratozás —, `1` használati hiba esetén, például
egy motornál, amelyet senki sem regisztrált.

## Hibák

| Ok | Üzenet |
| :---- | :------ |
| nincs motorprogram, vagy a beállított nincs ott | `the transcription engine was not found`, majd az útmutatóra mutató hivatkozás |
| nincs megadva modell, vagy nincs ott | `no model was given, or the model file is not there`, majd az útmutatóra mutató hivatkozás |
| az `ffmpeg` nem található egy videóhoz | `ffmpeg was not found, so the audio of a video cannot be extracted`, majd a hivatkozás |
| a videóban nem létező hangsáv | `the video has no audio track N`, majd a meglévő hangsávok |
| a modell által le nem iratozható nyelv | `the model cannot transcribe the language xx` |
| nincs nyelv, és a modell nem tud felismerni egyet | `the model cannot detect the language: give one` |
| egy motor, amelyet senki sem regisztrált | `no transcription engine is called NAME` |
| minden más hiba | `the transcription failed`, azzal, amit a motor mondott |

Az útmutatóra mutató hivatkozás: `See the manual page "Installing the transcription tools"
(install-transcription).`
