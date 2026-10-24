# subedit — manual

<!-- machine-translated -->
> Ca págine es un traduction machinal e it ancor ne ha esset revidet de un lector del lingue.

subedit lee, inspecte e retemporisa files de subtitules. It es composit de du
programmas queles partisse li sam nucleo, e dunc li sam regules e li sam paroles.

| Programma | Por quo | Su manual |
| :-------- | :-------------- | :--------- |
| `subedit-gui` | aperter un file, vider it in un tabelle, redacter it, observar it con li film | [li manual del fenestre](subedit-gui/index.md) |
| `subedit-cli` | tractar files in lotes, ex un script, sin interfacie | [li manual del linea de comande](subedit-cli/index.md) |

Ambi lee e scri **li nov formates** — SubRip, WebVTT, SubViewer 2,
Sub Station Alpha, Advanced SSA, MicroDVD, MPL2, TMPlayer, LRC — e
recognosce li formate per su contenete plu tosc quam per su extension. **Anc li codification
es recognoscet per li contenete**: un file in Latin-1, in CP1252 o in UTF-16
aperte se, it es dit quo it esset leet quam, e it es scrit denov in su propri codification — un
file aperit e pos salvat sin change ne move se de un sol byte.

## U comensar

| Quo on vole | U ear |
| :----------------- | :------- |
| installar li instrument | [Installation](subedit-cli/installation.md) — it vale por ambi programmas |
| aperter un file e vider quo it contene | [Li tabelle](subedit-gui/table.md) |
| retemporisar subtitules displaciat de un quantité constant | [Li operationes](subedit-gui/operations.md#shift-positions), o [`shift`](subedit-cli/shift.md) |
| retemporisar un file scrit por un altri frequentie de fotogrammas | [`framerate`](subedit-cli/framerate.md), e [`snap`](subedit-cli/snap.md) si li temporisation ja es just |
| saver por qual frequentie de fotogrammas un file esset scrit | [Li grille de fotogrammas](subedit-gui/frame-grid.md), o [`inspect`](subedit-cli/inspect.md) |
| remover li mentiones por surdes | [Li operationes](subedit-gui/operations.md#remove-hearing-impaired-mentions), o [`hearing-impaired`](subedit-cli/hearing-impaired.md) |
| corecter mentiones, errores comun, casu de littere e ruptura de linea in un sol passage | [`Correct Texts…`](subedit-gui/correct-texts.md), o [`correct`](subedit-cli/correct.md) |
| controlar li ortografie parol per parol, con li dictionariums del sistema | [`Check Spelling…`](subedit-gui/check-spelling.md) |
| allongar subtitules tro curt por esser leet | [`Adjust Durations…`](subedit-gui/operations.md#adjust-durations), o [`adjust`](subedit-cli/adjust.md) |
| remetter in órdine un file queles subtitules ne es in órdine | [`sort`](subedit-cli/sort.md) |
| crear un sol subtitul ex li peces de un textu queles seque se | [`Join Repeated Subtitles`](subedit-gui/lines.md#unir-li-subtitules-repetit), o [`join-repeats`](subedit-cli/join-repeats.md) |
| redresser in un vice un file mal segmentat, foris de órdine e con superpositiones | [`repair`](subedit-cli/repair.md) |
| remover li superpositiones inter subtitules | [`Make Ranges Disjoint…`](subedit-gui/lines.md#far-disjuncte-li-intervalles), o [`disjoint`](subedit-cli/disjoint.md) |
| mettre li du partes de un film un pos li altri | [`Append File…`](subedit-gui/operations.md#append-file), o [`append`](subedit-cli/append.md) |
| separar un film in du partes | [`Split Project…`](subedit-gui/operations.md#split-project), o [`split-file`](subedit-cli/split-file.md) |
| changear li casu de littere, li italic o li strecs de dialog de un tot file | [Li operationes](subedit-gui/operations.md#italic), o [`case`](subedit-cli/case.md), [`italics`](subedit-cli/italics.md), [`dialogue-dashes`](subedit-cli/dialogue-dashes.md) |
| serchar un parol e substituer it, sin rupter li etiquettes | [Trovar e substituer](subedit-gui/find-and-replace.md), o [`replace`](subedit-cli/replace.md) |
| scrir li parol de un film quam subtitules | [`Transcribe Audio…`](subedit-gui/video.md#transcrir-li-audio), o [`transcribe`](subedit-cli/transcribe.md) — pos [instalar li utensiles](subedit-cli/install-transcription.md) |
| traducter along li textu original | [Aperter un traduction](subedit-gui/files.md#aperter-un-traduction), o [`pair`](subedit-cli/pair.md) por retemporisar it secun li file principal |
| tractar cent files in un vice | [Tractar un arbore](subedit-cli/lots.md), poy [Invocation](subedit-cli/invocation.md) |

**Inter `framerate` e `snap`, un error passa inperceptet.** Si on hesita,
[`snap`](subedit-cli/snap.md#snap-ne-es-framerate-e-confunder-les-ne-es-visibil)
comensa con li comparation del du.

## Quo ca manual promete

**It descrive quo existe, nequande quo es planat.** Quo va venir, e in
qual órdine, es in li [plan de via](../../../feuille-de-route.md).

**Ca manual es scrit in anglesi, e traductet.** Su traductiones seque it
págine por págine, in li directoria `docs/i18n/` del repositoria, un arbore per lingue. Li
programmas parla li lingue de lor ambiente; li nómines queles ili monstra — subcomandes,
optiones, etiquettes de menu — es citat ci quam li programma les scri in anglesi.

**Li exemples e li images es generat, li textu ne.** Li
blocs `console` es produit per executer realmen li comande; li
capturas de ecran, per construer li veri fenestre e fotografar it. Nequi del
du dunc posse descrir un instrument quel ne existe plu. Quo resta sin rete es
li textu around les, queles exactità depende del relectura.

**Li capturas de ecran monstra ambi palettas**, `Light` e `Dark` — les quel
[li preferenties](subedit-gui/preferences.md#li-tema) ofre.

## Li du surfacies di li sam paroles

Un rason por leer li un o li altri manual quande on usa li altri: li
missages de error, li nómines de formates, li verdictes pri li grille e li
etiquettes de anullation es scrit **un vice**, in li nucleo, e ambi programmas
les cita. Un file refusat dona li sam frase de ambi lateres.

Quo diferentia es quo chascun surfacie posse far:

| | `subedit-gui` | `subedit-cli` |
| :--- | :------------ | :------------ |
| plu files in un vice | yes — un lingueta per projecte | yes |
| un traduction along li file principal | yes | yes — `-t` con li subcomandes de textu, e [`pair`](subedit-cli/pair.md) por retemporisar it |
| apender un file, separar un projecte | yes | yes — [`append`](subedit-cli/append.md), [`split-file`](subedit-cli/split-file.md) |
| anullar | yes, mil entratas | ne aplicabil — nullcos es changeat sur li input self sin demande |
| redacter un textu o un position per manu | yes | no |
| inserter, remover, coalescer e separar lineas | yes | no |
| trovar e substituer | yes | yes — [`replace`](subedit-cli/replace.md) |
| decuper, copiar e collar textus | yes | no |
| ajustar li durationes, li italic, li casu de littere, li strecs | yes | yes — [`adjust`](subedit-cli/adjust.md), [`italics`](subedit-cli/italics.md), [`case`](subedit-cli/case.md), [`dialogue-dashes`](subedit-cli/dialogue-dashes.md) |
| corecter li textus per modelles | yes — `Correct Texts…` | yes — [`correct`](subedit-cli/correct.md) |
| controlar li ortografie parol per parol | yes — `Check Spelling…` | no |
| observar li film durant li temporisation | yes | no |
| scrir sur li input | `Save` | `--in-place` |
