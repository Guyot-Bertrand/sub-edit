# Li video associat

<!-- machine-translated -->
> Ca págine es un traduction machinal e it ancor ne ha esset revidet de un lector del lingue.

Li menu **Video** porta prim du comandes, e li gestes quel condui li film
— vide [Li reproductor](player.md#mover-se-tra-li-film).

| Comande | Via rapid | Quo it fa |
| :------ | :------- | :----------- |
| Selecter video… | null | elige li file de video con quel li document vade |
| Reproducter / Pausa | `Ctrl+P` | reproducte o halta li video — vide [Li reproductor](player.md#reproducter-e-halter) |

Li video associat es **un proprietá del document aperit**, ne un parametre de
l'aplication: aperter un altri file de subtitules comensa ex zero. It
ne es scrit in null file e desapare con li fenestre.

## Li pista audio

Un film posse porta plu pistas de son — li version original, un doblage, un commentario. Li
submenu **`Video ▸ Audio ▸ Lingue`** les lista, **un entrata per pista**, e **marca ti quel es in reproduction**:

| Quo li file dona | Quo li entrata dice |
| :------------------ | :------------------ |
| un lingue e un titul | `1: fra — Original` |
| un lingue sol | `2: eng` |
| un titul sol | `3: Commentary` |
| null de ambi | `1` |

Li númere es **li loc del pista in li film**, un per un, in li órdine del file: du pistas
posse dir `eng`, e un file silent pri su etiquettes have tamen un prim e un secund.

**Eliger un entrata reproducte ta pista.** Li marca es li response del reproductor, ne li clic: it dice li
pista quel es in reproduction.

- **Li submenu es desactivat** sin video o sin pista audio. Con un sol pista, it resta activ e
  monstra it, marcat: it dice li lingue del film. Li du gestes de volúmine, queles es apu, resta
  activ in cada cas: vide [Li volúmine](player.md#li-volúmine).
- **Li pista ne es memorat** de un video al altri, ni de un session al altri: un númere de pista es
  ti de su file, e portar it al film sequent vell eliger ti quel have ta númere ta.
  Retornar a un lingueta reaperte su film sur li pista quel li reproductor elige per se.
- Null via rapid: on changea lingue rarmen, e li menu basta.

## Selecter un video

Li dialog aperte sur li directoria del file de subtitules e filtra sur
li extensiones de video recognosset:

    .avi .flv .m2ts .mkv .mov .mp4 .ogg .ogv .vob .webm

It propone anc monstrar omni files. Anullar li dialog changea
nullcos.

Li file elet es **aperit strax**: li image apare in li fenestre,
in pausa sur su prim instante. Vide [Li reproductor](player.md).

**Lassar cader un file de video sur li fenestre fa li sam** — vide
[Lassar cader files sur li fenestre](files.md#lassar-cader-files-sur-li-fenestre).

**Li video es ti del projecte del lingueta actual.** Chascun lingueta conserva
li su; changear lingueta changea li film. **Retornar a un lingueta reprende su
film u on on lassat it**, in pausa, quam cada apertion. Un altri film
elet pro ti projecte intertande comensa, ci, ex li comense.

## Li proposition automatic

Quande un file de subtitules es aperit, **si null video ha esset elet**,
li fenestre sercha un in li sam directoria: un file de video queles nómine
sin extension es un prefix del nostre, til un punctu.

| File de subtitules | Video trovat | Pro quo |
| :------------ | :---------- | :-- |
| `film.fr.srt` | `film.mkv` | `film` es un prefix de `film.fr` |
| `film.en.forced.srt` | `film.mkv` | egalmen, quant que sia li númere de segmentes |
| `film.fr.srt` | `film.fr.mkv` plu tosc quam `film.mkv` | li nómine li max long gania |
| `film.fr.srt` | null, si `film.mkv` **e** `film.mp4` es ta | du candidatos ne posse esser distinget |
| `film.fr.srt` | null, si solmen `fil.mkv` es ta | `fil` ne halta sur un punctu |

**Un proposition ne es un election.** Un proposition lassa se remplazzar per un altri
proposition — aperter un altri file, salvar sub un altri nómine. Un election
explicit, in contrari, nequande es superscrit: pos haver usat `Select Video…`, li
proposition automatic ne retorna plu detra it.

To es quo fa li conjectura acceptabil: it posse esser corecte, e su
correction tene.

## Quo li barre de statu monstra

A dextre del barre de statu, un linea dice con quo li document vade. It
es li ultim de tri: li [codification](files.md#li-codification-in-li-barre-de-statu)
del file leet vene prim, li [grille de fotogrammas](frame-grid.md) deducet del
positiones seque.

| Situation | Quo es scrit |
| :-------- | :-------------- |
| un video es associat | `Video: film.mkv` |
| … e `ffprobe` dona su frequentie de fotogrammas | `Video: film.mkv, 24000/1001 fps` |
| null | `No video` |

**Li frequentie de fotogrammas vade con li film plu tosc quam apu it.** Les es un sol fact — con quo
ca document vade — e un tertie mention vell metter it sur li sam rang quam
li [grille deducet del positiones](frame-grid.md), quel es un altri.

## Quande il ne ha film

Li fenestre monstra un buton **`Select Video…`** u li image esset, vice
nullcos del tot. It es li sam comande quam ti del menu `Video`, a portata
del mus: un absentie sur quel on ne posse actuar es mal distinget de un
defecte.

Li buton desapare tam rapid quam un film es aperit, e li image prende **tot** su
loc: li altore quel li tabelle lassa supra it — ti quel
[`window.table-share`](preferences.md) memorat de un session al altri —, e
nequande minus quam li altore minimal del image, quel li tabelle cede si necessi.
Quande li film vada, li buton reprende li sam loc.

**Li nómine del file, nequande su path.** Un path de du cent characteres
vell pussar omni altri cosess foris del barre; li path complet es ti quel li
dialog monstrat.

## `ffmpeg` ne es obligatori

`subedit-gui` interroga `ffprobe` — livrat con `ffmpeg` — por un sol cose:
**li frequentie de fotogrammas quel li contenitor declara**. It es leet un vez
per film, quande it es associat.

| Con `ffprobe` | Sin |
| :------------- | :------ |
| li barre de statu scri li frequentie de fotogrammas apu li nómine del film | it scri solmen li nómine |
| li dialog de conversion propone li frequentie de fotogrammas del film in li campe del fundo | it propone nullcos, e aperte quam ante |
| li dialog de alliniation aperte sur it | it aperte sur li frequentie de fotogrammas del projecte |

**Nullcos altri depende de it, e null operation refusa.** Li reproductor
ne besona `ffprobe` del tot: it se basa sur `libmpv`, quel es un
dependentie de compilation. Li duration del film, ti usat por reportar quo
passa ultra li fin, vene del reproductor e ne de `ffprobe`.

It es dunc ne un modo degradat ma **li functionament normal** de un machina
quel nequande ha installat `ffmpeg`: li data vene del usator, e `ffprobe`
propone it quande it es ta.

Un frequentie de fotogrammas leet es un **rational exact** — `24000/1001`, ne `23.976`.
To es li rason pro quo `ffprobe` es conservat por ta response: li
reproductor conosce li sam cose quam un númere in virgul flottant, e un frequentie de fotogrammas
aproximativ vell retemporisar li tot file un poc mal.
