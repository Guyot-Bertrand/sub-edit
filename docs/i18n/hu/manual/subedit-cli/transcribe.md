# `transcribe`

<!-- machine-translated -->
> Ez az oldal gépi fordítás, amelyet a nyelv egyik olvasója még nem nézett át.

```
subedit-cli transcribe --output FILE
                       [--to FORMAT]
                       [--audio-track N]
                       [--language CODE]
                       [--start TIME] [--end TIME]
                       [--suppress-notes | --keep-notes]
                       [--model FILE]
                       [--vad-model FILE]
                       [--engine NAME]
                       [--keep-audio FILE]
                       [--raw]
                       <input>

subedit-cli transcribe --list-languages [--model FILE] [--engine NAME]
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
- **A nyelvet két betűvel kell megadni — `fr`, `en`, `de` —, vagy a fájl által megadott hárombetűs kóddal, `fra`,
  `eng`, amelyet a kétbetűs kódként olvas.** Minden más elutasításra kerül a munka megkezdése előtt. **A felismerés
  csak az első harminc másodpercet hallgatja**: egy zenével vagy csenddel kezdődő filmet másik nyelvűnek
  nézhet, és a motor ekkor a beszédet arra a nyelvre *lefordítva* írja le — egy francia film, amelyet angolnak néz,
  angolul jön ki, vagy `[SPEAKING FRENCH]` alakban. Ha ismeri a nyelvet, adja meg.
- **A film egy része külön is leiratozható** a `--start` és az `--end` kapcsolóval — lásd:
  [Egy rész leiratozása](#egy-rész-leiratozása). Így lehet egy modellt kétperces szakaszon kipróbálni, mielőtt
  kétórás filmre indítják.
- **A motor által kínált nyelvek listázhatók**, a `--list-languages` kapcsolóval — lásd:
  [A nyelvek listázása](#a-nyelvek-listázása).
- **A hangok alapértelmezés szerint nem íródnak ki megjegyzésként**: a motor megkapja az utasítást, hogy ne tegye (`--suppress-notes`; a saját kapcsolója
  a `--suppress-nst`), így a `(Sonnerie de téléphone)` vagy a `[MUSIC PLAYING]` nem jelenik meg a feliratokban, sem kerek, sem szögletes
  zárójelben. A `--keep-notes` kiírja őket, halláskárosultaknak szóló feliratokhoz. A megjegyzések tiltása mellett a motor `...`-ot ír a beszéd nélküli
  szakaszra, és **az utófeldolgozás eldobja azokat a darabokat, amelyekben nincs betű és nincs számjegy** — `...`, `…`, `♪` —
  az összevonás és a vágás előtt; a `--raw` megtartja őket, ahogy a motor darabjai megérkeztek.
- **A szakaszokat egyesíti és vágja** a beállítások korlátai szerint (`transcription.merge-gap-ms`,
  `transcription.max-duration-ms`, `transcription.max-characters`): egy mondat két közeli darabja
  egy felirat, a túl hosszú vagy túl széles darabot pedig vágja el — először mondatvégnél, aztán vesszőnél, pontosvesszőnél vagy kettőspontnál, és mondat közepén csak akkor, ha nincs írásjel, ahol vágni lehetne —, elkerülve azt a vágást, amely olvashatatlanul kicsi feliratot hagy. A három lépés sorrendben megy, mindegyik az egész átiraton: eldobás, egyesítés, vágás. Az a felirat, amely egy darabot kezd, megtartja a kezdetét, az, amelyik lezárja, megtartja a végét. **A `--raw`
  úgy írja ki a motor szakaszait, ahogy megjöttek**, szakaszonként egy feliratot, és így látható az is, mit
  csinál az egyesítés.

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
  --vad-model FILE            Voice-activity model that skips silences; the transcription.vad-model setting by default
  --engine NAME               Engine; the transcription.engine setting by default
  --keep-audio FILE           Keep the extracted audio in this file
  --list-languages            List the languages the engine offers for the model, and transcribe nothing
  --raw                       Write the segments as the engine cut them
```

## Argumentumok és kapcsolók

| Kapcsoló | Kötelező | Értékek | Alapértelmezés |
| :----- | :----- | :------ | :----- |
| `<input>` | igen, kivéve a `--list-languages` esetén | egy videó vagy egy hangfájl | — |
| `--output` | igen, kivéve a `--list-languages` esetén | a kiírandó feliratfájl; a könyvtára létrejön, ha hiányzik | — |
| `--to` | nem | egy formátum rövid neve: `srt`, `vtt`, `subviewer2`, `ssa`, `ass`, `microdvd`, `mpl2`, `tmplayer`, `lrc` | az `--output` kiterjesztése által megadott formátum, különben `srt` |
| `--audio-track` | nem | 0-tól induló egész szám: a hely a videó hangsávjai között | `0`, az első |
| `--language` | nem | két betű, egy ISO 639-1 kód: `en`, `fr`; egy hárombetűs kódot, például `fra`-t, `fr`-ként olvas | a `transcription.language` beállítás; üres: felismeri |
| `--suppress-notes` | nem | kapcsoló: ne írja ki a hangokat megjegyzésként; elutasítva olyan motornál, amelynek ez nem mondható meg | a `transcription.suppress-notes` beállítás |
| `--keep-notes` | nem | kapcsoló: írja ki őket, a beállítással szemben; a `--suppress-notes` ellentéte, és a kettő együtt elutasított | a `transcription.suppress-notes` beállítás |
| `--list-languages` | nem | kapcsoló: a motor által a modellhez kínált nyelvek listázása, leiratozás nélkül | ki |
| `--start` | nem | egy időpont: `2:00`, `1:02:03`, `2:00.5`, vagy másodpercek száma, `90` | a hang eleje |
| `--end` | nem | egy időpont, a `--start`-hoz hasonlóan írva; a `--start` után kell jönnie | a hang vége |
| `--model` | nem | a modellfájl | a `transcription.model` beállítás |
| `--vad-model` | nem | egy beszédaktivitás-modell (egy Silero-modell a whisper.cpp-hez), amely a dekódolás előtt kihagyatja a motorral a csendet: a szegmensei ilyenkor a beszédet követik, és az első ott kezdődik, ahol a beszéd, nem 0-nál. A fájlnak léteznie kell, és az a motor, amely nem tudja használni, név szerint elutasítja a kapcsolót | a `transcription.vad-model` beállítás |
| `--engine` | nem | egy motor neve: `whisper.cpp` | a `transcription.engine` beállítás |
| `--keep-audio` | nem | egy fájl a kinyert hang megtartásához, mono 16 bites PCM-ként WAV fájlban, a motor által kért mintavételi frekvencián (whisper.cpp esetén 16 kHz); a könyvtára létrejön, ha hiányzik, és a név tartalmazhat `{rate}`-et, amelyet ez a frekvencia helyettesít hertzben (a `film.{rate}.wav` neve `film.16000.wav` lesz) | a hang sehol sem marad meg |
| `--raw` | nem | kapcsoló: a motor szakaszainak kiírása egyesítés és vágás nélkül | ki |

A **beállítások** a beállításfájlból olvasódnak, mint az [`adjust`](adjust.md) beállításai — lásd
[A beállításfájl](invocation.md#a-beállításfájl) —, és mindegyiknek itt megvan a saját kapcsolója:

| Kulcs | Jelentés | Alapértelmezés |
| :-- | :------ | :------ |
| `transcription.engine` | a motor | `whisper.cpp` |
| `transcription.executable` | a motor programja, abszolút elérési út; üres: a `PATH`-ban keresi | üres |
| `transcription.model` | a modellfájl, abszolút elérési út | üres |
| `transcription.vad-model` | a beszédaktivitás-modell, abszolút elérési út; üres: nincs, és a csendet nem hagyja ki. A beállítást, amelyet a motor nem tud használni, szó nélkül elhagyja | üres |
| `transcription.language` | a hang nyelve, két betű; üres: felismeri | üres |
| `transcription.suppress-notes` | ne írja ki a hangokat megjegyzésként | `true` |
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

## Egy rész leiratozása

A `--start` és az `--end` a munkát a kettejük közti filmrészre korlátozza. Elég az egyiket megadni: a puszta
`--start` a végéig tart, a puszta `--end` az elejétől kezdődik.

```bash
subedit-cli transcribe --start 2:00 --end 3:30 --output test.srt --model large-v3.bin film.mkv
```

- **Az időpontot `M:SS` vagy `H:MM:SS` alakban kell írni, kívánság szerint ponttal elválasztott törtrésszel (`2:00.5`), vagy
  másodpercek számaként (`90`, `90.5`)**. A vesszőt elutasítja, mint az eszköz mindenhol.
- **A feliratok megtartják a film idejét**: a film 2:10-nél elhangzó mondata `00:02:10`-nél áll, nem
  `00:00:10`-nél. A fájl úgy, ahogy van, ráhelyezhető a teljes filmre.
- **A szélek hozzávetőlegesek**: a motor harminc másodperces ablakokban dolgozik, ezért amit visszaad, azt utólag
  a tartományra vágja; a szélen átnyúló mondatot ott vágja el.
- **Semmi sem indul el, ha a tartomány használhatatlan**: egy határ, amely nem időpont, vagy egy `--end`, amely nem a
  `--start` után van, használati hiba, amely megnevezi a problémát, nem ír ki semmit, és sem az `ffmpeg`-et, sem a motort nem indítja el.

A teljes film hangja ettől még kinyerődik: a tartomány a leiratozáson spórol, amely sokkal hosszabb.

## A nyelvek listázása

A `--list-languages` kiírja a motor által **a modellhez** kínált nyelveket, soronként egyet: a `--language` által elfogadott kódot, egy tabulátort, és a
nyelv nevét a felület nyelvén. Nem kér sem bemenetet, sem kimenetet, és nem iratoz le semmit.

```bash
subedit-cli transcribe --list-languages --model ggml-small.bin
```

```text
en	English
zh	Chinese
de	German
…
```

A lista a motortól és a modelltől függ: egy többnyelvű whisper.cpp modellnél körülbelül száz nyelv, egy csak angolra való
modellnél csak az `en`. Az a motor, amely nem tudja felsorolni a nyelveit, egyet sem listáz. Egy meg nem adott vagy nem létező modell hiba, az útmutatóra mutató hivatkozással; csak
a szöveges alak létezik, a [`--format json`](invocation.md#szkript-által-olvasható-kimenet) mellett semmi sem íródik ki.

## Kimenet

**Szabványos kimenet** — semmi: az eredmény a kiírt fájl.

| Szint | Mi kerül még a hibakimenetre |
| :---- | :-------------------------------- |
| 1 | `<input>: N subtitles transcribed, language: xx -> <output>` |
| 2 | a lépés és az előrehaladás, százalékonként egy sor: `extracting the audio…`, majd `transcribing…`, majd `transcribing: 25%, 0:12:30 of 0:50:00, about 38 min left, elapsed: 0:05:12` — a százalék, a hangban elért pozíció, a hátralévő idő és az eltelt idő; **a sort tíz másodpercenként újra kiírja, ha semmi új nem érkezett**, hogy a hosszú várakozást ne tekintsék lefagyásnak |
| 3 | és minden új szöveg, amelyet a motor megkap, `last text: …` |

**A százalék a motor pozíciója a hangban, nem a saját ablakainak száma**: a motor menet közben kiírja, amit leiratozott,
a parancs pedig kiolvassa, hol tart, így a szám már az első percben mozog. (A motor saját előrehaladása
a hang 5 %-os lépéseiben érkezik, egy harmincmásodperces ablak elkészülte után: egyedül akár negyedóráig is
0 %-on maradhat egy hosszú filmnél, nagy modellel, processzoron.) **A hátralévő idő az elején hallgat**: akkor jelenik meg,
amikor elég munka készült az extrapoláláshoz, és az utolsó perc sebességét követi, nem a teljes futás átlagát.

### JSON-ban

A [`--format json`](invocation.md#szkript-által-olvasható-kimenet) kapcsolóval (a globális kapcsoló, az
alparancs előtt írva) a fájl objektuma a használt nyelvet a `language` mezőben, a `counts.subtitles`
(a kiírt feliratok) és a `counts.segments` (a motor darabjai, amelyekből készültek) mezőt, valamint
az `audio` mezőt hordozza, ha a `--keep-audio` megőrizte a hangot.

## Figyelmeztetések

Egy figyelmeztetés hangzik el az első szinten, a munka előtt, mert megváltoztatja, mire kell számítani:

- **Csak angolra való modellnél**: `This model is for English only: whatever language the film
  speaks, the result will be English, or notes in brackets. Choose a multilingual model in the preferences.` Az a modell
  ilyen, amelynek a fájlneve `.en`-nel végződik.

## Kilépési kódok

Az eszköz kódjai: `0`, ha a fájl kiíródott, `2`, ha nem sikerült — hiányzó eszköz vagy modell, a modell
által nem fogadható nyelv, sikertelen kinyerés vagy leiratozás —, `1` használati hiba esetén, például
egy motornál, amelyet senki sem regisztrált, vagy használhatatlan tartománynál.

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
| hiányzó `<input>` vagy `--output` | `input is required` vagy `--output is required` |
| `--suppress-notes` a `--keep-notes` mellett | `--suppress-notes and --keep-notes say opposite things; give one of them` |
| `--suppress-notes` olyan motornál, amelynek ez nem mondható meg | `the engine NAME cannot be told not to write sounds as notes` |
| egy nyelv, amely nem kód | `"TEXT" is not a language code: write two letters, such as fr, en, de or es` |
| a tartomány egy határa, amely nem időpont | `"TEXT" is not a time: write it as 2:00, 1:02:03 or 2:00.5, or in seconds` |
| az `--end` nem a `--start` után | `the end of the range must come after its start` |
| minden más hiba | `the transcription failed`, azzal, amit a motor mondott |

Az útmutatóra mutató hivatkozás: `See the manual page "Installing the transcription tools"
(install-transcription).`
