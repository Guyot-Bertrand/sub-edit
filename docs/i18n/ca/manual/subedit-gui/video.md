# El vídeo associat

<!-- machine-translated -->
> Aquesta pàgina és una traducció automàtica i encara no l'ha revisada cap lector de la llengua.

El menú **Vídeo** porta primer tres ordres, i els gestos que condueixen la pel·lícula
— vegeu [El reproductor](player.md#desplaçar-se-per-la-pellícula).

| Ordre | Drecera | Què fa |
| :------ | :------- | :----------- |
| Selecciona un vídeo… | cap | tria el fitxer de vídeo amb què va el document |
| Transcriu l'àudio… | cap | transcriu l'àudio de la pel·lícula en subtítols que substitueixen els del projecte — vegeu [Transcriure l'àudio](#transcriure-làudio) |
| Reprodueix / Pausa | `Ctrl+P` | reprodueix o atura el vídeo — vegeu [El reproductor](player.md#reproduir-i-aturar) |

El vídeo associat és **una propietat del document obert**, no un paràmetre de
l'aplicació: obrir un altre fitxer de subtítols torna a començar de zero. No
s'escriu a cap fitxer i desapareix amb la finestra.

## La pista d'àudio

Una pel·lícula pot portar diverses pistes de so — la versió original, un doblatge, un comentari. El
submenú **`Vídeo ▸ Àudio ▸ Llengua`** les llista, **una entrada per pista**, i **marca la que sona**:

| Què dona el fitxer | Què diu l'entrada |
| :------------------ | :------------------ |
| una llengua i un títol | `1: fra — Original` |
| només una llengua | `2: eng` |
| només un títol | `3: Commentary` |
| cap dels dos | `1` |

El número és **el lloc de la pista a la pel·lícula**, una per una, en l'ordre del fitxer: dues pistes
poden dir `eng`, i un fitxer que no diu res de les seves etiquetes continua tenint una primera i una segona.

**Triar una entrada reprodueix aquesta pista.** La marca és la resposta del reproductor, no el clic: indica la
pista que sona.

- **El submenú està desactivat** sense vídeo o sense pista d'àudio. Amb una sola pista, continua activat i
  la mostra, marcada: indica la llengua de la pel·lícula. Els dos gestos de volum, que hi són al costat, continuen
  activats en tot cas: vegeu [El volum](player.md#el-volum).
- **La pista no es recorda** d'un vídeo al següent, ni d'una sessió a la següent: un número de pista és
  el del seu fitxer, i traslladar-lo a la pel·lícula següent triaria el que hi porti aquest número.
  Tornar a una pestanya torna a obrir la seva pel·lícula a la pista que el reproductor tria per si mateix.
- Cap drecera: es canvia de llengua poques vegades, i el menú n'hi ha prou.

## Triar un vídeo

El diàleg s'obre al directori del fitxer de subtítols i filtra per
les extensions de vídeo reconegudes:

    .avi .flv .m2ts .mkv .mov .mp4 .ogg .ogv .vob .webm

També ofereix mostrar tots els fitxers. Cancel·lar el diàleg no canvia
res.

El fitxer triat **s'obre immediatament**: la imatge apareix a la finestra,
en pausa en el seu primer instant. Vegeu [El reproductor](player.md).

**Deixar anar un fitxer de vídeo a la finestra fa el mateix** — vegeu
[Deixar anar fitxers a la finestra](files.md#deixar-anar-fitxers-a-la-finestra).

**El vídeo és el del projecte de la pestanya actual.** Cada pestanya conserva
el seu; canviar de pestanya canvia de pel·lícula. **Tornar a una pestanya reprèn la seva
pel·lícula allà on l'havíeu deixat**, en pausa, com qualsevol obertura. Una altra pel·lícula
triada per a aquest projecte entretant comença, en canvi, des del principi.

## La proposta automàtica

Quan s'obre un fitxer de subtítols, **si no s'ha triat cap vídeo**,
la finestra en cerca un al mateix directori: un fitxer de vídeo el nom del qual
sense extensió és un prefix del nostre, fins a un punt.

| Fitxer de subtítols | Vídeo trobat | Per què |
| :------------ | :---------- | :-- |
| `film.fr.srt` | `film.mkv` | `film` és un prefix de `film.fr` |
| `film.en.forced.srt` | `film.mkv` | igualment, sigui quin sigui el nombre de segments |
| `film.fr.srt` | `film.fr.mkv` en lloc de `film.mkv` | guanya el nom més llarg |
| `film.fr.srt` | cap, si hi ha `film.mkv` **i** `film.mp4` | dos candidats no es poden distingir |
| `film.fr.srt` | cap, si només hi ha `fil.mkv` | `fil` no s'atura en un punt |

**Una proposta no és una elecció.** Es deixa substituir per una altra
proposta — obrir un altre fitxer, desar amb un altre nom. Una elecció
explícita, en canvi, no se sobreescriu mai: un cop s'ha fet servir `Select Video…`, la
proposta automàtica ja no hi torna al darrere.

És el que fa acceptable l'endevinalla: es pot corregir, i la seva
correcció es manté.

## Què mostra la barra d'estat

A la dreta de la barra d'estat, una línia diu amb què va el document.
És l'última de tres: la [codificació](files.md#la-codificació-a-la-barra-destat)
del fitxer llegit va primer, la [graella de fotogrames](frame-grid.md) deduïda de les
posicions després.

| Situació | Què s'escriu |
| :-------- | :-------------- |
| hi ha un vídeo associat | `Video: film.mkv` |
| … i `ffprobe` en dona la freqüència de fotogrames | `Video: film.mkv, 24000/1001 fps` |
| cap | `No video` |

**La freqüència de fotogrames va amb la pel·lícula en lloc d'anar al seu costat.** Són un sol fet — amb què
va aquest document — i una tercera menció la posaria al mateix rang que
la [graella deduïda de les posicions](frame-grid.md), que és un altre fet.

## Transcriure l'àudio

**`Transcribe Audio…`** escriu la parla de la pel·lícula en forma de subtítols. Està desactivada fins que s'obre una pel·lícula: és
l'àudio de **la pel·lícula del projecte**, i mai un fitxer que triaríeu — per transcriure un
fitxer d'àudio, useu [`transcribe`](../subedit-cli/transcribe.md) a la línia d'ordres.

El diàleg demana tres coses, i diu si pot començar:

- **la pista d'àudio**, llistada com la llisten el menú [`Audio`](#la-pista-dàudio), amb la llengua
  i el títol que declara el fitxer;
- **la llengua**, dues lletres com `en` o `fr`. Comença amb la llengua de la pista triada quan el
  fitxer en declara una, segueix la pista mentre no n'hàgiu escrit cap, i
  **una línia sota el camp diu d'on ve**. Quan el fitxer no declara cap llengua per a la pista,
  la línia ho diu: el model la detectarà aleshores a partir del so, cosa que només fa un model
  multilingüe, i l'hauríeu d'escriure si la coneixeu. Amb un model només per a l'anglès, que no
  detecta res, la línia diu que cal escriure la llengua;
- **la part de la pel·lícula**, d'un temps a un altre — vegeu [Provar primer una part](#provar-primer-una-part). Amb els dos
  camps buits, és tota la pel·lícula.

**També diu què hi falta.** Tres línies diuen si el motor, `ffmpeg` i el model hi són, i
on; quan un no hi és, `OK` queda apagat, i el diàleg ho diu amb la referència a
[Instal·lació de les eines de transcripció](../subedit-cli/install-transcription.md). Un botó `Preferences…` obre
les [preferències](preferences.md#transcripció), on s'indiquen el programa i el model — són fets
sobre la màquina, no eleccions que calgui refer a cada transcripció — i el diàleg torna a mirar quan es tanquen.

La llengua es conserva per a la vegada següent. El motor mateix i els límits dels subtítols són
[paràmetres](preferences.md) (`transcription.*`).

**Quan el projecte ja conté subtítols, la finestra pregunta abans de substituir-los**, i diu
quants se'n van. Pregunta abans de la feina, de manera que declinar no costa res. Quan el projecte és buit no
hi ha res a preguntar.

La feina s'executa **en segon pla**, amb un quadre i un botó `Cancel`; la finestra continua utilitzable al voltant. **El quadre
diu en quin punt és la feina**, perquè sapigueu que és viva i quant trigarà:

- **el pas**: primer “Extracció de l'àudio…” — el quadre és aleshores ocupat, sense xifra, ja que extreure no diu res de fins on ha
  arribat —, després “Transcripció de l'àudio…”, amb una barra;
- **la posició**: “0:12:30 de 0:50:00”, fins on de la pel·lícula ha arribat el motor, tan bon punt ha produït el seu primer text;
- **el temps que queda**, “queden uns 38 min”: absent al principi, mostrat quan s'ha fet prou feina per extrapolar, i calculat a partir de la
  velocitat del darrer minut; és una precisió de pocs minuts, no una promesa;
- **el darrer text** obtingut, perquè veieu que la feina té sentit.

En una pel·lícula llarga amb un model gran en un processador, els primers minuts poden ser llargs: el motor carrega el model i treballa en les seves
primeres finestres de trenta segons. La posició i el darrer text es mouen abans que el tant per cent propi del motor, que només ve a intervals
del 5 % de la pel·lícula. Quan acaba, els subtítols del projecte se substitueixen
per la transcripció **com una sola operació**: un sol `Undo` retorna el que hi havia, textos de
traducció inclosos. Una transcripció que falla, o que cancel·leu, no canvia res — el motiu es
diu en un quadre, tret d'una cancel·lació, que només diu que s'ha cancel·lat a la barra d'estat.

### Provar primer una part

Un interval converteix la transcripció en una **prova**, per jutjar un model o una llengua en dos minuts abans de comprometre's amb
dues hores. Els camps “Part de la pel·lícula” accepten un inici i un final, cadascun escrit `2:00`, `1:02:03`, `2:00.5` o un nombre
de segons, `90`; se'n permet només un (d'allà fins al final, o del principi fins allà). `OK` queda apagat mentre un camp no
és un temps o el final no ve després de l'inici, i una línia sota els camps diu quin.

- **Els subtítols conserven el temps de la pel·lícula**: el que es diu a 2:10 és a `0:02:10` al resultat.
- **No se substitueix res i no es pregunta res.** El resultat s'obre com **un projecte a part**, en una pestanya nova, sense
  fitxer; el projecte del qual ve, i els seus subtítols, queden com abans. Deseu-lo, o tanqueu-lo.

**El que necessita no es lliura**: un motor (whisper.cpp), un model, i `ffmpeg` per extreure l'àudio.
Vegeu [Instal·lació de les eines de transcripció](../subedit-cli/install-transcription.md).

## Quan no hi ha pel·lícula

La finestra mostra un botó **`Select Video…`** allà on aniria la imatge, en lloc de
no res. És la mateixa ordre que la del menú `Video`, a l'abast
del ratolí: una absència sobre la qual no es pot actuar és difícil de distingir d'una
avaria.

El botó desapareix tan bon punt s'obre una pel·lícula, i la imatge n'ocupa **tot** el
lloc: l'altura que la taula li deixa a sobre — la que
[`window.table-share`](preferences.md) ha recordat d'una sessió a la següent —, i
mai menys que l'altura mínima de la imatge, que la taula cedeix si cal.
Quan la pel·lícula se'n va, el botó reprèn el mateix lloc.

**El nom del fitxer, mai el seu camí.** Un camí de dos-cents caràcters
empenyeria tota la resta fora de la barra; el camí complet és el que mostrava
el diàleg.

## `ffmpeg` no és obligatori

`subedit-gui` consulta `ffprobe` — distribuït amb `ffmpeg` — per a una cosa i
només una: **la freqüència de fotogrames que declara el contenidor**. Es llegeix una
vegada per pel·lícula, quan s'associa.

| Amb `ffprobe` | Sense |
| :------------- | :------ |
| la barra d'estat escriu la freqüència de fotogrames al costat del nom de la pel·lícula | només n'escriu el nom |
| el diàleg de conversió proposa la freqüència de fotogrames de la pel·lícula al camp inferior | no proposa res, i s'obre com abans |
| el diàleg d'alineació s'hi obre | s'obre a la freqüència de fotogrames del projecte |

**No en depèn res més, i cap operació no es nega.** El reproductor, per la seva banda,
no necessita gens `ffprobe`: es basa en `libmpv`, que és una
dependència de compilació. La durada de la pel·lícula, la que serveix per indicar el que
va més enllà del final, ve del reproductor i no de `ffprobe`.

**La transcripció és l'única cosa que sí que necessita `ffmpeg`**: n'extreu l'àudio de la pel·lícula. Vegeu
[Instal·lació de les eines de transcripció](../subedit-cli/install-transcription.md).
No és, per tant, un mode degradat sinó **el funcionament normal** d'una màquina
que no ha instal·lat mai `ffmpeg`: la dada ve de l'usuari, i `ffprobe`
la proposa quan hi és.

Una freqüència de fotogrames llegida és un **racional exacte** — `24000/1001`, no `23.976`.
És el motiu pel qual es conserva `ffprobe` per a aquesta resposta: el
reproductor sap el mateix com a nombre de coma flotant, i una freqüència de fotogrames aproximada
resincronitzaria tot el fitxer lleugerament desviat.
