# subedit — manual

<!-- machine-translated -->
> Aquesta pàgina és una traducció automàtica i encara no l'ha revisada cap lector de la llengua.

subedit llegeix, inspecciona i resincronitza fitxers de subtítols. Es presenta com a dos
programes que comparteixen el mateix nucli, i per tant les mateixes regles i les mateixes paraules.

| Programa | Per a què | El seu manual |
| :-------- | :-------------- | :--------- |
| `subedit-gui` | obrir un fitxer, veure'l en una taula, editar-lo, mirar-lo al costat de la pel·lícula | [el manual de la finestra](subedit-gui/index.md) |
| `subedit-cli` | processar fitxers per lots, des d'un script, sense interfície | [el manual de la línia d'ordres](subedit-cli/index.md) |

Tots dos llegeixen i escriuen **els nou formats** — SubRip, WebVTT, SubViewer 2,
Sub Station Alpha, Advanced SSA, MicroDVD, MPL2, TMPlayer, LRC — i
reconeixen el format pel contingut i no per l'extensió. **La codificació també
es reconeix pel contingut**: un fitxer en Latin-1, en CP1252 o en UTF-16
s'obre, es diu com s'ha llegit, i es torna a escriure en la seva pròpia codificació — un
fitxer obert i desat sense cap canvi no es mou ni un byte.

## Per on començar

| Què voleu | On anar |
| :----------------- | :------- |
| instal·lar l'eina | [Instal·lació](subedit-cli/installation.md) — val per a tots dos programes |
| obrir un fitxer i mirar què conté | [La taula](subedit-gui/table.md) |
| resincronitzar subtítols desplaçats una quantitat constant | [Les operacions](subedit-gui/operations.md#shift-positions), o [`shift`](subedit-cli/shift.md) |
| resincronitzar un fitxer escrit per a una altra freqüència de fotogrames | [`framerate`](subedit-cli/framerate.md), i [`snap`](subedit-cli/snap.md) si la sincronització ja és correcta |
| saber amb quina freqüència de fotogrames es va escriure un fitxer | [La graella de fotogrames](subedit-gui/frame-grid.md), o [`inspect`](subedit-cli/inspect.md) |
| suprimir les mencions per a persones sordes | [Les operacions](subedit-gui/operations.md#remove-hearing-impaired-mentions), o [`hearing-impaired`](subedit-cli/hearing-impaired.md) |
| corregir mencions, errors comuns, majúscules i minúscules i la divisió de línies en una sola passada | [`Correct Texts…`](subedit-gui/correct-texts.md), o [`correct`](subedit-cli/correct.md) |
| comprovar l'ortografia paraula per paraula, amb els diccionaris del sistema | [`Check Spelling…`](subedit-gui/check-spelling.md) |
| allargar subtítols massa curts per llegir-los | [`Adjust Durations…`](subedit-gui/operations.md#adjust-durations), o [`adjust`](subedit-cli/adjust.md) |
| tornar a posar en ordre un fitxer els subtítols del qual no ho estan | [`sort`](subedit-cli/sort.md) |
| fer un sol subtítol dels trossos d'un mateix text que se segueixen | [`Join Repeated Subtitles`](subedit-gui/lines.md#unir-els-subtítols-repetits), o [`join-repeats`](subedit-cli/join-repeats.md) |
| arreglar, d'una sola vegada, un fitxer mal segmentat, desordenat i amb encavalcaments | [`repair`](subedit-cli/repair.md) |
| suprimir els encavalcaments entre subtítols | [`Make Ranges Disjoint…`](subedit-gui/lines.md#fer-disjuntes-les-franges), o [`disjoint`](subedit-cli/disjoint.md) |
| posar una darrere l'altra les dues parts d'una pel·lícula | [`Append File…`](subedit-gui/operations.md#append-file), o [`append`](subedit-cli/append.md) |
| tallar una pel·lícula en dues parts | [`Split Project…`](subedit-gui/operations.md#split-project), o [`split-file`](subedit-cli/split-file.md) |
| canviar les majúscules i minúscules, la cursiva o els guions de diàleg de tot un fitxer | [Les operacions](subedit-gui/operations.md#italic), o [`case`](subedit-cli/case.md), [`italics`](subedit-cli/italics.md), [`dialogue-dashes`](subedit-cli/dialogue-dashes.md) |
| cercar una paraula i reemplaçar-la, sense trencar les etiquetes | [Cerca i reemplaça](subedit-gui/find-and-replace.md), o [`replace`](subedit-cli/replace.md) |
| traduir al costat del text original | [Obrir una traducció](subedit-gui/files.md#obrir-una-traducció), o [`pair`](subedit-cli/pair.md) per resincronitzar-la sobre el fitxer principal |
| processar cent fitxers alhora | [Processar un arbre](subedit-cli/lots.md), i després [Invocació](subedit-cli/invocation.md) |

**Entre `framerate` i `snap`, un error passa desapercebut.** Si
dubteu, [`snap`](subedit-cli/snap.md#snap-no-és-framerate-i-confondrels-no-es-nota)
comença amb la comparació de tots dos.

## Què promet aquest manual

**Descriu el que existeix, mai el que està previst.** El que vindrà, i en
quin ordre, és al [full de ruta](../../../feuille-de-route.md).

**Aquest manual s'escriu en anglès, i es tradueix.** Les seves traduccions el segueixen
pàgina per pàgina, sota el directori `docs/i18n/` del repositori, un arbre per llengua. Els
programes parlen la llengua del seu entorn; els noms que mostren — subordres,
opcions, etiquetes de menú — es citen aquí tal com el programa els escriu en anglès.

**Els exemples i les imatges es generen; la prosa, no.** Els blocs
`console` es produeixen executant realment l'ordre; les
captures de pantalla, construint la finestra real i fotografiant-la. Cap
dels dos no pot, doncs, descriure una eina que ja no existeix. El que queda sense xarxa
és el text que els envolta, la correcció del qual depèn de la revisió.

**Les captures de pantalla mostren totes dues paletes**, `Light` i `Dark` — les que
ofereixen [les preferències](subedit-gui/preferences.md#el-tema).

## Les dues superfícies diuen les mateixes paraules

Un motiu per llegir qualsevol dels dos manuals quan s'usa l'altre: els
missatges d'error, els noms dels formats, els veredictes de la graella i les etiquetes
de desfer s'escriuen **una sola vegada**, al nucli, i tots dos programes
els citen. Un fitxer refusat dona la mateixa frase a totes dues bandes.

El que difereix és el que pot fer cada superfície:

| | `subedit-gui` | `subedit-cli` |
| :--- | :------------ | :------------ |
| diversos fitxers alhora | sí — una pestanya per projecte | sí |
| una traducció al costat del fitxer principal | sí | sí — `-t` a les subordres de text, i [`pair`](subedit-cli/pair.md) per resincronitzar-la |
| afegir un fitxer, dividir un projecte | sí | sí — [`append`](subedit-cli/append.md), [`split-file`](subedit-cli/split-file.md) |
| desfer | sí, mil entrades | no s'hi aplica — no es modifica res al seu lloc sense demanar-ho |
| editar un text o una posició a mà | sí | no |
| inserir, suprimir, fusionar i dividir línies | sí | no |
| cercar i reemplaçar | sí | sí — [`replace`](subedit-cli/replace.md) |
| retallar, copiar i enganxar textos | sí | no |
| ajustar les durades, la cursiva, les majúscules i minúscules, els guions | sí | sí — [`adjust`](subedit-cli/adjust.md), [`italics`](subedit-cli/italics.md), [`case`](subedit-cli/case.md), [`dialogue-dashes`](subedit-cli/dialogue-dashes.md) |
| corregir els textos amb patrons | sí — `Correct Texts…` | sí — [`correct`](subedit-cli/correct.md) |
| comprovar l'ortografia paraula per paraula | sí — `Check Spelling…` | no |
| mirar la pel·lícula mentre se sincronitza | sí | no |
| escriure sobre l'entrada | `Save` | `--in-place` |
