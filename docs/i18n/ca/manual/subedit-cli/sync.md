# `sync`

<!-- machine-translated -->
> Aquesta pàgina és una traducció automàtica i encara no l'ha revisada cap lector de la llengua.

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

**Resincronitza un fitxer de subtítols segons l'àudio d'una pel·lícula.** El cas per al qual existeix és el `.srt`
descarregat d'internet, desplaçat al principi i al final, que un sol desplaçament no arregla. Dos mètodes ho fan, i
`--method` (o el paràmetre `alignment.method`) tria entre ells:

- **`speech`, el mètode per defecte, posa el fitxer sobre el moment en què algú parla.** Només pregunta al so on hi ha
  parla, i troba el desplaçament que fa coincidir els subtítols amb ella, tros de pel·lícula per tros de pel·lícula: un
  desplaçament constant, una deriva, una segona meitat endarrerida i un fitxer sincronitzat per a una altra freqüència de
  fotogrames (25 per a 23.976, i similars) es corregeixen tots. **No depèn de la llengua** — els subtítols poden ser en
  francès i la pel·lícula en anglès —, no necessita cap transcripció, i triga segons. Necessita `ffmpeg`, el programa
  `whisper-vad-speech-segments` que ve amb whisper.cpp, i un model d'activitat de veu (`--vad-model`, el paràmetre
  `transcription.vad-model`).
- **`words` fa coincidir les paraules de cada subtítol amb les que ha sentit un motor.** Escolta amb un motor de
  transcripció — el mateix que [`transcribe`](transcribe.md) — i cada subtítol pren el lloc on es diuen les seves
  paraules. No es tradueix res: els subtítols han d'estar en la llengua que es parla. Pot col·locar un sol subtítol que
  està fora de lloc en un fitxer que per la resta és correcte, i triga el temps d'una transcripció.

**El que `speech` no fa es diu cada vegada que s'executa**, a la sortida d'error i al registre: corregeix el fitxer
**sencer**, mai un subtítol per separat. Un subtítol que està fora de lloc tot sol en un fitxer que per la resta és
correcte, o un títol mostrat sobre un silenci (una targeta de capítol, un rètol que tradueix alguna cosa de la imatge),
no es cerca i pot continuar sent erroni, i els primers minuts poden continuar desfasats quan la pel·lícula comença amb
música o títols. Reviseu el resultat, o feu servir `--method words` quan el text és en la llengua parlada.

**Res del que necessita no es lliura amb el programa**: els programes, el model i `ffmpeg` són els d'[Instal·lació de les eines de transcripció](install-transcription.md), i la seva absència no trenca res —
l'ordre diu què falta, amb el punter cap a aquesta pàgina, no escriu res i surt amb un codi diferent de zero.

- **Només canvien els inicis i els finals.** Mai el text, l'estil, el nombre de subtítols ni el seu ordre.
- **Un subtítol que l'àudio no dona conserva els seus temps** — uns crèdits que ningú no diu, una cançó, una línia en
  una altra llengua. **Cap posició no s'inventa mai, i cap no s'endevina a partir dels veïns**: la línia es deixa on
  la tenia el fitxer, i l'ordre ho diu. *No* és un error, i el codi de sortida és `0`.
- **Amb `words`, un subtítol es considera trobat quan s'han sentit almenys la meitat de les seves paraules**
  (`--min-match`, `alignment.min-match`), i almenys una. Les majúscules, els accents, la puntuació i el marcatge no
  importen; una paraula que difereix en una lletra d'una paraula llarga compta com a sentida. `speech` col·loca tots els
  subtítols, i no accepta cap de `--language`, `--model`, `--engine` ni `--min-match`.
- **El final és on acaba l'última paraula.** El motor data les paraules de manera aproximada, unes centèsimes de segon
  abans del final propi del subtítol, de manera que un subtítol pot sortir una mica més curt del que era.
- **Un subtítol que s'ficaria dins el següent es retalla fins on comença el següent.** El que encara se superposa al
  següent, cosa que retallar no pot curar, es deixa i es diu.
- **Un fitxer que no encaixa amb el so es refusa i no s'escriu**: amb `words`, no s'ha trobat cap dels subtítols, cosa
  que gairebé sempre vol dir l'àudio equivocat o la llengua equivocada — `speech` no depèn de la llengua; amb `speech`,
  els subtítols i la parla tenen massa poc en comú, cosa que gairebé sempre vol dir la pel·lícula equivocada.
- **Es pot escoltar una part de la pel·lícula tota sola**, amb `--start` i `--end`, com per a
  [`transcribe`](transcribe.md#transcriure-una-part): només es cerquen i es toquen els subtítols que **comencen** dins
  l'interval, els altres es deixen tal com són i no es compten.
- **L'àudio s'escolta una vegada per cada fitxer donat.** Per resincronitzar diversos fitxers sobre la mateixa
  pel·lícula, compteu d'esperar per cadascun.

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

## Arguments i opcions

| Opció | Obligatori | Valors | Per defecte |
| :----- | :------- | :----- | :------ |
| `<files>` | sí | els fitxers de subtítols que cal resincronitzar; amb `-r`, directoris | — |
| `--recursive`, `-r` | no | un indicador | desactivat — vegeu [Tractar un arbre](lots.md) |
| `--video` | una de `--video` i `--audio` | la pel·lícula que cal escoltar | — |
| `--audio` | una de `--video` i `--audio` | un fitxer d'àudio que cal escoltar, en lloc d'una pel·lícula | — |
| `--audio-track` | no | un nombre enter des de 0: el lloc entre les pistes d'àudio de la pel·lícula | `0`, la primera |
| `--language` | no | dues lletres, un codi ISO 639-1: `en`, `fr` | el paràmetre `transcription.language`; buit: es detecta |
| `--start`, `--end` | no | un temps: `2:00`, `1:02:03`, `2:00.5`, o un nombre de segons; `--end` ha de venir després de `--start` | tot l'àudio |
| `--model` | no | el fitxer de model | el paràmetre `transcription.model` |
| `--vad-model` | no | un model d'activitat de veu, que fa que el motor ometi els silencis; amb ell les paraules segueixen més de prop la parla | el paràmetre `transcription.vad-model` |
| `--engine` | no | el nom del motor que escolta: `whisper.cpp` | el paràmetre `transcription.engine` |
| `--keep-audio` | no | un fitxer on conservar l'àudio extret; el seu directori es crea si falta | l'àudio no es conserva enlloc |
| `--method` | no | `speech`: posa el fitxer sobre el moment en què hi ha parla, en qualsevol llengua; `words`: fa coincidir les paraules amb les que s'han sentit, en la llengua parlada | el paràmetre `alignment.method`, `speech` |
| `--min-match` | no | un nombre enter de l'1 al 100: la part de les paraules d'un subtítol que s'han d'haver sentit, en percentatge | el paràmetre `alignment.min-match`, `50` |
| `--doubt-gap` | no | un nombre enter de mil·lisegons, 0 o més: quant pot diferir el desplaçament d'un subtítol del dels veïns abans de ser assenyalat | el paràmetre `alignment.doubt-gap`, `2000` |
| `--report` | no | un fitxer on escriure els subtítols que cal revisar, un per línia: el número del subtítol, un tabulador, i el motiu; un fitxer de subtítols alhora | no s'escriu res |
| `--output` / `--output-dir` / `--in-place` | **una de les tres**, excepte amb `--dry-run` | vegeu [Invocació](invocation.md#la-destinació) | — |
| `--dry-run` | no | un indicador: calcula i diu, no escriu res — vegeu [Veure abans d'escriure](invocation.md#veure-abans-descriure) | desactivat |

La destinació — `--output`, `--output-dir`, `--in-place`, `--dry-run` — i `-r` són els de les altres ordres que
reescriuen un fitxer: vegeu [la destinació](invocation.md#la-destinació). El fitxer conserva el seu format, la seva
codificació i els seus salts de línia.

Els **paràmetres** es llegeixen del fitxer de configuració — vegeu [el fitxer de configuració](invocation.md#el-fitxer-de-configuració) — i
cadascun té aquí la seva opció. El motor, el model i el model d'activitat de veu són els de `transcribe`.

| Clau | Significat | Per defecte |
| :-- | :------ | :------ |
| `alignment.engine` | el motor d'alineació del mètode `words`; n'hi ha un, `transcript-matching`, que escolta i fa coincidir | `transcript-matching` |
| `alignment.method` | com es posen els subtítols en línia: `speech` o `words` | `speech` |
| `alignment.min-match` | la part de les paraules d'un subtítol que s'han d'haver sentit perquè es consideri trobat, en percentatge | `50` |
| `alignment.doubt-gap` | quant, en mil·lisegons, pot allunyar-se el desplaçament d'un subtítol del dels veïns abans de ser assenyalat | `2000` |

## Què s'assenyala

L'ordre diu quins subtítols cal revisar, i per què. Un subtítol s'assenyala pel **primer** d'aquests motius que es compleix:

| Per què | Significat |
| :-- | :------ |
| not found in the audio, left as it was | s'han sentit menys de `--min-match` de les seves paraules: conserva els seus temps |
| only partly heard | trobat, però s'han sentit menys del 80 % de les seves paraules |
| moved differently from its neighbours | trobat, però el seu desplaçament s'allunya del desplaçament mitjà dels dos subtítols de cada costat en més de `--doubt-gap` |

Amb `speech` es col·loca tots els subtítols, i només l'últim d'aquests motius pot aplicar-se: allà on el fitxer canvia el
seu desplaçament en més de `--doubt-gap` d'un subtítol al següent (una segona meitat endarrerida), els subtítols a cada
costat del salt s'assenyalen.

Un subtítol també pot ser **retallat fins a l'inici del subtítol següent**, o **encara se superposa al subtítol següent**.

Dues xifres expliquen la història del fitxer sencer. **El desplaçament mitjà** és el valor central de quant s'han
mogut els subtítols: un fitxer que simplement anava tard es llegeix aquí, i enlloc més. **La deriva** és el
desplaçament mitjà de l'última desena part dels subtítols menys el de la primera desena: és gairebé zero per a un
decalatge constant, i gran per a un fitxer que anava tard cada vegada més al llarg de la pel·lícula.

## Sortida

**Sortida estàndard** — res: el resultat és el fitxer escrit.

| Nivell | Què es diu a la sortida d'error |
| :---- | :------------------------------- |
| 1 | `source: <file>`, `media: <audio>`, `result: N of M subtitles placed on the audio, K to check; median shift X s, drift Y s`, `dest: <destination>` — i, quan s'ha corregit una freqüència de fotogrames, la raó; amb `speech`, **l'advertència del que no fa**, en una línia pròpia |
| 2 | el pas i el progrés, com per a [`transcribe`](transcribe.md#sortida) |

### En JSON

Amb [`--format json`](invocation.md#sortida-llegible-per-un-script), l'objecte del fitxer porta `counts`: `subtitles`
(cercats), `placed` (col·locats), `moved` (els temps dels quals han canviat), `partial`, `out_of_line`, `not_found`,
`median_shift_ms` i `drift_ms`; i `flagged`, una llista de `{"subtitle": N, "kind": "…"}` on el número és el lloc del
subtítol al fitxer i `kind` és un de `not-found`, `partial`, `out-of-line`, `cut-back` i `overlaps`.
Amb `speech`, `warnings` porta una entrada de `kind` `speech-only`. Els identificadors són una promesa; les frases no.

## Exemple

```bash
subedit-cli sync --video film.mkv --vad-model silero.bin \
    --report to-check.txt --output film.synced.srt film.srt
```

Per col·locar cada subtítol segons les seves paraules, per a un text en la llengua parlada:

```bash
subedit-cli sync --method words --video film.mkv --model small.bin --vad-model silero.bin \
    --output film.synced.srt film.srt
```

## Codis de sortida

Els de l'eina: `0` si s'ha escrit el fitxer, amb subtítols assenyalats o sense, `2` si no s'ha pogut — una eina o un
model que falta, una extracció o una escolta fallida, un fitxer que no encaixa amb el so (cap dels seus subtítols trobat, o massa poc en
comú amb la parla) —, `1` en un error d'ús, com ara ni `--video` ni `--audio` (o tots dos), una opció fora del seu
interval, o un interval que no es pot fer servir.

## Errors

| Causa | Missatge |
| :---- | :------ |
| ni `--video` ni `--audio`, o tots dos | `sync needs the audio to listen to: give --video or --audio, and not both` |
| no s'ha trobat cap programa de motor, o el configurat no hi és | `the transcription engine was not found`, i després el punter a la guia |
| no s'ha donat cap model, o no hi és | `no model was given, or the model file is not there`, i després el punter a la guia |
| `ffmpeg` no trobat per a una pel·lícula | `ffmpeg was not found, so the audio of a video cannot be extracted`, i després el punter |
| una pista que la pel·lícula no té | `the video has no audio track N`, i després les pistes que té |
| una llengua que el model no pot transcriure | `the model cannot transcribe the language xx` |
| un motor que ningú no ha registrat | `no transcription engine is called NAME`, o `there is no alignment engine called NAME` |
| cap subtítol trobat (`words`) | `none of the N subtitles was found in the audio: is it the right audio, and the right language?`, i després `The method based on speech does not depend on the language.` |
| no s'ha indicat el model d'activitat de veu, o no es troba el programa que troba la parla (`speech`) | `Finding where there is speech needs a voice-activity model: name one in the transcription.vad-model setting.` o `Finding where there is speech needs the program whisper-vad-speech-segments, which comes with whisper.cpp, and it was not found.`, i després el punter a la guia |
| els subtítols i la parla tenen massa poc en comú (`speech`) | `The sound and the subtitles have too little in common to be put in line: is it the right film?` |
| `--min-match` fora de 1 a 100, `--doubt-gap` negatiu | l'error d'ús de l'opció |
| `--report` amb més d'un fitxer de subtítols | `--report names one file: give one subtitle file when you use it` |
