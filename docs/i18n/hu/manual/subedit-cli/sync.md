# `sync`

<!-- machine-translated -->
> Ez az oldal gépi fordítás, amelyet a nyelv egyik olvasója még nem nézett át.

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

**Egy feliratfájlt időzít újra egy film hangja szerint.** Arra az esetre való, amikor az online letöltött `.srt` az elején és a
végén is el van csúszva, és egyetlen eltolás sem hozza helyre. Két módszer van rá, és a `--method` (vagy az `alignment.method` beállítás)
választ közülük:

- **A `speech`, az alapértelmezett, arra fekteti rá a fájlt, amikor valaki beszél.** A hangtól csak azt kérdezi, hol van beszéd,
  és megkeresi azt az eltolást, amelytől a feliratok egyeznek vele, filmszakaszonként: az állandó eltolást, az elcsúszást, a késő második felet
  és a más képkockasebességre időzített fájlt (25 a 23,976 helyett és a hasonlók) mind helyre hozza.
  **Nem függ a nyelvtől** — a feliratok lehetnek franciául, a film angolul —, nem kell hozzá leiratozás, és másodpercek alatt végez. Az `ffmpeg`,
  a whisper.cpp-vel érkező `whisper-vad-speech-segments` program és egy beszédaktivitás-modell kell hozzá (`--vad-model`, a
  `transcription.vad-model` beállítás).
- **A `words` minden felirat szavait összeveti azokkal a szavakkal, amelyeket egy motor hallott.** Egy leiratozó motorral hallgat —
  ugyanazzal, mint a [`transcribe`](transcribe.md) —, és minden felirat oda kerül, ahol a szavai elhangzanak. Semmit sem fordít:
  a feliratoknak azon a nyelven kell lenniük, amelyen a beszéd folyik. Egyetlen rossz helyen álló feliratot is el tud helyezni egy egyébként jó fájlban, és
  egy leiratozás idejébe kerül.

**Azt, hogy mit nem tesz a `speech`, minden futásnál kimondja**, a hibakimeneten és a jegyzékben: a fájlt
**egészében** javítja, soha nem egyenként a feliratokat. Az a felirat, amely magában áll rossz helyen egy egyébként jó fájlban,
vagy a csend fölött látható cím (fejezetkártya, a képernyőn látható valamit fordító felirat) nincs keresve,
és még mindig téves lehet, az első percek pedig eltolva maradhatnak, ha a film zenével vagy főcímmel nyílik. Ellenőrizze az eredményt,
vagy használja a `--method words` kapcsolót, ha a szöveg a beszélt nyelven van.

**Semmit, amire szüksége van, nem szállít a program**: a programok, a modell és az `ffmpeg` azok, amelyeket
[A leiratozó eszközök telepítése](install-transcription.md) leír, és hiányuk nem tör el semmit — a parancs megmondja,
mi hiányzik, az oldalra mutató hivatkozással, nem ír semmit, és nullától különböző kóddal lép ki.

- **Csak a kezdetek és a végek változnak.** Soha a szöveg, a stílus, a feliratok száma vagy sorrendjük.
- **Az a felirat, amelyet a hang nem ad meg, megtartja az idejét** — egy stáblista, amelyet senki sem mond ki, egy dal, egy másik nyelvű sor.
  **Pozíciót soha nem talál ki, és a szomszédokból sem sejt**: a sor ott marad, ahol a fájlban volt,
  és a parancs ezt közli. Ez *nem* hiba, és a kilépési kód `0`.
- **A `words` esetén egy feliratot akkor talál meg, ha a szavainak legalább fele hallható** (`--min-match`,
  `alignment.min-match`), és legalább egy. A kis- és nagybetű, az ékezetek, az írásjelek és a jelölések nem számítanak; egy hosszú szótól egy betűvel eltérő szó
  hallottnak számít. A `speech` minden feliratot elhelyez, és a `--language`, `--model`, `--engine` és `--min-match` kapcsolók közül egyet sem vesz figyelembe.
- **A vég ott van, ahol az utolsó szó véget ér.** A motor a szavak idejét hozzávetőlegesen adja meg, a felirat saját végénél néhány századmásodperccel
  korábban, ezért egy felirat kissé rövidebb lehet, mint volt.
- **Az a felirat, amely beleütközne a következőbe, vissza lesz vágva a következő kezdetéig.** Az, amely még így is átfedi a
  következőt, mert a vágás nem segít rajta, úgy marad, és a parancs jelzi.
- **Az a fájl, amely nem illik a hanghoz, elutasított, és nem íródik ki**: a `words` esetén egyetlen felirat sem található,
  ami szinte mindig rossz hangot vagy rossz nyelvet jelent — a `speech` nem függ a nyelvtől —; a `speech` esetén a feliratoknak és a beszédnek
  túl kevés a közös vonása, ami szinte mindig rossz filmet jelent.
- **A film egy része külön is meghallgatható**, a `--start` és az `--end` kapcsolóval, mint a
  [`transcribe`](transcribe.md#egy-rész-leiratozása) esetén: csak azokat a feliratokat keresi és érinti, amelyek a tartományban **kezdődnek**,
  a többit úgy hagyja, ahogy van, és nem számolja őket.
- **A hangot minden megadott fájlhoz egyszer hallgatja meg.** Ha több fájlt kell ugyanarra a filmre újraidőzíteni,
  mindegyikre várni kell.

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

## Argumentumok és kapcsolók

| Kapcsoló | Kötelező | Értékek | Alapértelmezés |
| :----- | :------- | :----- | :------ |
| `<files>` | igen | az újraidőzítendő feliratfájlok; `-r` esetén könyvtárak | — |
| `--recursive`, `-r` | nem | jelzőkapcsoló | ki — lásd [Könyvtárfa feldolgozása](lots.md) |
| `--video` | a `--video` és az `--audio` egyike | a meghallgatandó film | — |
| `--audio` | a `--video` és az `--audio` egyike | a meghallgatandó hangfájl, film helyett | — |
| `--audio-track` | nem | 0-tól induló egész szám: a hely a film hangsávjai között | `0`, az első |
| `--language` | nem | két betű, egy ISO 639-1 kód: `en`, `fr` | a `transcription.language` beállítás; üres: felismeri |
| `--start`, `--end` | nem | egy időpont: `2:00`, `1:02:03`, `2:00.5`, vagy másodpercek száma; az `--end` a `--start` után kell jöjjön | az egész hang |
| `--model` | nem | a modellfájl | a `transcription.model` beállítás |
| `--vad-model` | nem | egy beszédaktivitás-modell, amely a csendet kihagyatja a motorral; vele a szavak közelebb követik a beszédet | a `transcription.vad-model` beállítás |
| `--engine` | nem | a hallgató motor neve: `whisper.cpp` | a `transcription.engine` beállítás |
| `--keep-audio` | nem | egy fájl a kinyert hang megtartásához; a könyvtára létrejön, ha hiányzik | a hang sehol sem marad meg |
| `--method` | nem | `speech`: a fájlt arra fekteti, amikor beszéd van, bármilyen nyelven; `words`: a szavakat összeveti a hallottakkal, a beszélt nyelven | az `alignment.method` beállítás, `speech` |
| `--min-match` | nem | 1 és 100 közötti egész szám: a felirat szavainak hány százalékát kell meghallani | az `alignment.min-match` beállítás, `50` |
| `--doubt-gap` | nem | ezredmásodpercek egész száma, 0 vagy több: mennyivel térhet el egy felirat eltolása a szomszédaiétól, mielőtt megjelölik | az `alignment.doubt-gap` beállítás, `2000` |
| `--report` | nem | egy fájl az ellenőrizendő feliratok kiírásához, soronként egyet: a felirat száma, egy tabulátor és az ok; egyszerre egy feliratfájl | semmi sem íródik ki |
| `--output` / `--output-dir` / `--in-place` | **a három közül egy**, kivéve a `--dry-run` esetén | lásd [Indítás](invocation.md#a-cél) | — |
| `--dry-run` | nem | jelzőkapcsoló: kiszámítja és közli, de nem ír semmit — lásd [Megnézés írás előtt](invocation.md#megnézés-írás-előtt) | ki |

A cél — `--output`, `--output-dir`, `--in-place`, `--dry-run` — és a `-r` ugyanaz, mint a többi fájlt átíró parancsnál:
lásd [a cél](invocation.md#a-cél). A fájl megtartja a formátumát, a kódolását és a sorvégeit.

A **beállítások** a beállításfájlból olvasódnak — lásd [A beállításfájl](invocation.md#a-beállításfájl) —, és mindegyiknek itt megvan a saját
kapcsolója. A motor, a modell és a beszédaktivitás-modell ugyanaz, mint a `transcribe` esetén.

| Kulcs | Jelentés | Alapértelmezés |
| :-- | :------ | :------ |
| `alignment.engine` | a `words` módszer igazítómotorja; egy van, a `transcript-matching`, amely meghallgat és összevet | `transcript-matching` |
| `alignment.method` | hogyan igazodnak a feliratok: `speech` vagy `words` | `speech` |
| `alignment.min-match` | a felirat szavainak hány százalékát kell meghallani ahhoz, hogy megtalálják | `50` |
| `alignment.doubt-gap` | mennyivel, ezredmásodpercben, térhet el egy felirat eltolása a szomszédaiétól, mielőtt megjelölik | `2000` |

## Mi kerül megjelölésre

A parancs megmondja, mely feliratokat kell ellenőrizni, és miért. Egy feliratot az alábbiak **első** teljesülő okáért jelöl meg:

| Miért | Jelentés |
| :-- | :------ |
| not found in the audio, left as it was | szavainak kevesebb mint `--min-match` része hallható: megtartja az idejét |
| only partly heard | megtalálva, de szavainak kevesebb mint 80 %-a hallható |
| moved differently from its neighbours | megtalálva, de eltolása a két-két szomszédos felirat középső eltolásától `--doubt-gap`-nél nagyobb mértékben tér el |

A `speech` esetén minden felirat el van helyezve, és csak az utolsó ok lehetséges: ahol a fájl eltolása egyik feliratról a másikra
`--doubt-gap`-nél nagyobb mértékben változik (késő második fele), a lépcső két oldalán lévő feliratok meg vannak jelölve.

Egy felirat **vissza is lehet vágva a következő felirat kezdetéig**, vagy **még mindig átfedheti a következő feliratot**.

Két szám meséli el a fájl egészének történetét. **A medián eltolás** azt mutatja, mennyivel mozdultak a feliratok, a középső értéket
véve: az a fájl, amely egyszerűen késett, itt látszik, máshol sehol. **Az elcsúszás** az utolsó tized feliratai
medián eltolásának és az első tizedének a különbsége: állandó eltolásnál nulla közeli, és nagy annál a fájlnál, amely a film során egyre
többet késett.

## Kimenet

**Szabványos kimenet** — semmi: az eredmény a kiírt fájl.

| Szint | Mi kerül még a hibakimenetre |
| :---- | :------------------------------- |
| 1 | `source: <file>`, `media: <audio>`, `result: N of M subtitles placed on the audio, K to check; median shift X s, drift Y s`, `dest: <destination>` — és, ha egy képkockasebesség helyre lett hozva, az arány; a `speech` esetén **annak figyelmeztetése, amit nem tesz**, külön sorban |
| 2 | a lépés és az előrehaladás, mint a [`transcribe`](transcribe.md#kimenet) esetén |

### JSON-ban

A [`--format json`](invocation.md#szkript-által-olvasható-kimenet) kapcsolóval a fájl objektuma a `counts` mezőt hordozza: `subtitles`
(a keresettek), `placed` (a helyet kaptak), `moved` (az idejük megváltozott), `partial`, `out_of_line`, `not_found`,
`median_shift_ms` és `drift_ms`; valamint a `flagged` mezőt, `{"subtitle": N, "kind": "…"}` elemek listáját, ahol a szám a
felirat helye a fájlban, a `kind` pedig a `not-found`, `partial`, `out-of-line`, `cut-back` és `overlaps` egyike.
A `speech` esetén a `warnings` egy `speech-only` `kind`-ú elemet hordoz. Az azonosítók szavatoltak; a mondatok nem.

## Példa

```bash
subedit-cli sync --video film.mkv --vad-model silero.bin \
    --report to-check.txt --output film.synced.srt film.srt
```

Ha minden feliratot a szavai szerint kell elhelyezni, a beszélt nyelven lévő szövegnél:

```bash
subedit-cli sync --method words --video film.mkv --model small.bin --vad-model silero.bin \
    --output film.synced.srt film.srt
```

## Kilépési kódok

Az eszköz kódjai: `0`, ha a fájl kiíródott, megjelölt feliratokkal vagy anélkül, `2`, ha nem sikerült — hiányzó eszköz vagy modell, sikertelen
kinyerés vagy meghallgatás, a hanghoz nem illő fájl (egyetlen felirata sem található, vagy túl kevés a közös vonás a beszéddel) —, `1` használati hiba esetén, például ha sem a
`--video`, sem az `--audio` nincs megadva (vagy mindkettő), egy kapcsoló kívül esik a tartományán, vagy a tartomány használhatatlan.

## Hibák

| Ok | Üzenet |
| :---- | :------ |
| sem `--video`, sem `--audio`, vagy mindkettő | `sync needs the audio to listen to: give --video or --audio, and not both` |
| nincs motorprogram, vagy a beállított nincs ott | `the transcription engine was not found`, majd az útmutatóra mutató hivatkozás |
| nincs megadva modell, vagy nincs ott | `no model was given, or the model file is not there`, majd az útmutatóra mutató hivatkozás |
| az `ffmpeg` nem található egy filmhez | `ffmpeg was not found, so the audio of a video cannot be extracted`, majd a hivatkozás |
| a filmben nem létező hangsáv | `the video has no audio track N`, majd a meglévő hangsávok |
| a modell által le nem iratozható nyelv | `the model cannot transcribe the language xx` |
| egy motor, amelyet senki sem regisztrált | `no transcription engine is called NAME`, vagy `there is no alignment engine called NAME` |
| egyetlen felirat sem található (`words`) | `none of the N subtitles was found in the audio: is it the right audio, and the right language?`, majd `The method based on speech does not depend on the language.` |
| a beszédaktivitás-modell nincs megnevezve, vagy a beszédet kereső program nem található (`speech`) | `Finding where there is speech needs a voice-activity model: name one in the transcription.vad-model setting.` vagy `Finding where there is speech needs the program whisper-vad-speech-segments, which comes with whisper.cpp, and it was not found.`, majd az útmutatóra mutató hivatkozás |
| a feliratoknak és a beszédnek túl kevés a közös vonása (`speech`) | `The sound and the subtitles have too little in common to be put in line: is it the right film?` |
| `--min-match` az 1–100 tartományon kívül, negatív `--doubt-gap` | a kapcsoló használati hibája |
| `--report` egynél több feliratfájllal | `--report names one file: give one subtitle file when you use it` |
