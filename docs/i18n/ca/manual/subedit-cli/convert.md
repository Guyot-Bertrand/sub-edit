# `convert`

<!-- machine-translated -->
> Aquesta pàgina és una traducció automàtica i encara no l'ha revisada cap lector de la llengua.

```
subedit-cli convert --to <format>
                    [--line-endings unix|windows|mac] [--to-encoding NAME]
                    [--bom | --no-bom] [--sort] [--disjoint]
                    [--join-repeats] [--repair]
                    (--output FILE | --output-dir DIR | --in-place)
                    [--dry-run]
                    [--recursive]
                    <file>...
```

Escriu cada fitxer en el format i la forma demanats. **No canvia mai
l'entrada**, tret que `--in-place` ho demani explícitament.

<!-- example: subedit-cli convert --help -->
```console
$ subedit-cli convert --help
Write a subtitle file out in another format or shape
Usage: subedit-cli convert [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to convert

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --to TEXT:{srt,vtt,subviewer2,ssa,ass,microdvd,mpl2,tmplayer,lrc} REQUIRED
                              Format to write
  --line-endings TEXT:{unix,windows,mac}
                              Line endings to write; the source's by default
  --to-encoding NAME          Encoding to write; the source's by default
  --frame-rate RATE           Frame rate of a file counted in frames: 25, 23.976
  --bom                       Write a byte order mark
  --no-bom                    Write no byte order mark
  --sort                      Put the subtitles in order of their start before writing
  --disjoint                  Cut overlapping subtitles so that no range overlaps another, before writing
  --join-repeats              Join the subtitles that repeat each other into one, before writing
  --repair                    Join the repeats, sort, then make the ranges disjoint, before writing
  --output TEXT               File to write, for a single input
  --output-dir TEXT           Directory to write into
  --in-place                  Write back over the inputs
  --dry-run                   Work out and say what would be written, and write nothing
```

## Arguments i opcions

| Opció | Obligatori | Valors | Per defecte |
| :----- | :----- | :------ | :----- |
| `<file>...` | sí | un o més camins | — |
| `--recursive`, `-r` | no | un indicador | desactivat — vegeu [Tractar un arbre](lots.md) |
| `--to` | **sí** | un nom de la taula següent, i res més | — |
| `--line-endings` | no | `unix`, `windows` o `mac` | els del fitxer llegit |
| `--to-encoding` | no | qualsevol codificació que ICU pugui escriure, excepte les que escriuen la seva pròpia marca | la del fitxer llegit |
| `--bom` / `--no-bom` | no | indicadors, excloents l'un de l'altre | el que portava el fitxer llegit |
| `--sort` | no | un indicador: posa els subtítols en l'ordre del seu començament **abans d'escriure**, vegeu [Ordenar en escriure](#ordenar-en-escriure) | desactivat |
| `--join-repeats` | no | un indicador: fa un sol subtítol amb els fragments d'un mateix text que se segueixen, **abans de qualsevol altra cosa**, vegeu [Fer disjuntes les franges](#fer-disjuntes-les-franges) | desactivat |
| `--repair` | no | un indicador: el mateix que `--join-repeats --sort --disjoint`, en aquest ordre | desactivat |
| `--disjoint` | no | un indicador: talla els subtítols que s'encavalquen **abans d'escriure**, vegeu [Fer disjuntes les franges](#fer-disjuntes-les-franges) | desactivat |
| `--output` / `--output-dir` / `--in-place` | **una de les tres**, excepte amb `--dry-run` | vegeu [Invocació](invocation.md#la-destinació) | — |
| `--dry-run` | no | un indicador: calcula i diu, no escriu res — vegeu [Veure abans d'escriure](invocation.md#veure-abans-descriure) | desactivat |

`mac` anomena el retorn de carro sol (`\r`), el final de línia del Mac OS clàssic.

## Els formats que accepta `--to`

| Nom | Format | Extensió escrita |
| :-- | :----- | :--------------- |
| `srt` | SubRip | `.srt` |
| `vtt` | WebVTT | `.vtt` |
| `subviewer2` | SubViewer 2.0 | `.sub` |
| `ssa` | Sub Station Alpha | `.ssa` |
| `ass` | Advanced SSA | `.ass` |
| `mpl2` | MPL2 | `.txt` |
| `microdvd` | MicroDVD | `.sub` |
| `tmplayer` | TMPlayer | `.txt` |
| `lrc` | LRC | `.lrc` |

**Un nom, i no una extensió.** Dues extensions anomenen dos formats
cadascuna — `.sub` també és la de MicroDVD, `.txt` la de MPL2 i la de
TMPlayer —, de manera que una opció que prengués extensions no sabria quin
es demana. Els noms anteriors no són ambigus, i coincideixen amb
l'extensió allà on l'extensió tampoc no és ambigua.

**La llista és tancada.** És la de Gaupol, i els nou hi són.

## Els dos formats que no porten final

**TMPlayer i LRC només escriuen una posició per línia**, el seu començament. Això
es nota per tots dos costats.

**En llegir**, els finals es dedueixen: el d'una línia és el començament de
la següent, i l'última rep cinc segons. És el que fa Gaupol.
També ho diem, amb un diagnòstic que `--verbose` mostra a les tres
subordres:

```
file.lrc: carries no end times; each one was taken from the next start, settled by the reader
```

**En escriure**, els finals no s'escriuen, per manca d'un lloc on posar-los.
Convertir a un d'aquests dos formats, per tant, els perd, i un viatge d'anada i tornada no
els torna a trobar — els torna a deduir.

**LRC perd una cosa més**: no té cap manera de portar un salt de línia.
Una rèplica en dues línies s'escriu en una, amb les dues línies unides per
un espai. Aquesta pèrdua tampoc no es pot reparar.

Cap dels dos no té vocabulari per a la cursiva.

## `--frame-rate`, i l'únic format que compta en fotogrames

**MicroDVD no compta en segons**: les seves posicions *són* números de
fotograma, i cap fitxer MicroDVD no declara la freqüència a la qual es van
comptar. `--frame-rate` la dona, i serveix per tots dos costats: és la
freqüència a la qual es llegeix un `.sub` en fotogrames, i aquella amb què una
conversió a MicroDVD compta els fotogrames.

| Opció | Obligatori | Valors | Per defecte |
| :----- | :----- | :------ | :----- |
| `--frame-rate` | no | una freqüència en fotogrames per segon: `25`, `23.976`, `29.97` | vegeu més avall |

**En llegir, sense `--frame-rate`, el fitxer es llegeix a 23.976** — el valor
de Gaupol — i el nivell `-vvv` ho diu:
`counts in frames and states no rate; it was read at`. És l'únic lloc de l'eina on totes les posicions mostrades
reposen sobre una suposició que el fitxer no pot confirmar.

**En escriure, sense `--frame-rate`, tres casos i tres respostes:**

| El fitxer de partida | Què passa |
| :------------------- | :-------------- |
| ja és MicroDVD | la freqüència que l'ha llegit el torna a escriure, i els fotogrames tornen idèntics |
| és basat en el temps, i les seves posicions cauen sobre una graella | es pren la graella, i `-vv` diu quina |
| és basat en el temps, sense graella | **refús**, codi 2: `writing frames needs a frame rate, and the positions fall on no grid to take one from — give --frame-rate` |

El refús és deliberat. L'únic gest que quedaria seria inventar-se una xifra, i
cada línia del fitxer es mouria tant com aquesta fos errònia.

## Què perd una conversió

**Convertir a un altre format no és mai sense pèrdua**, i el que no sobreviu
depèn de la parella:

| Què no passa | Per què |
| :--------------------- | :------- |
| la capçalera | només té sentit en el seu propi format: `[INFORMATION]` no és una capçalera WebVTT |
| les dades pròpies del format | les coordenades de SubRip, l'identificador i els paràmetres d'una indicació WebVTT no tenen equivalent en cap altre lloc |
| la precisió | SubViewer 2, els dos Sub Station Alpha i LRC escriuen a la centèsima, MPL2 a la dècima, TMPlayer al segon, MicroDVD al fotograma; `01:00:00,017` torna com a `01:00:00,020` |
| el final d'una línia | TMPlayer i LRC no hi tenen cap camp; la lectura següent el torna a deduir |
| els salts de línia | només LRC: les seves rèpliques caben en una línia, i les dues es tornen a enganxar amb un espai |
| les etiquetes que el format de destinació no pot escriure | WebVTT no té `<font color>`, TMPlayer i LRC no tenen res de res |
| la disposició | `{\pos(x,y)}`, `{\an8}`, `<v Marie>`, `<ruby>`: sobreviuen a un viatge d'anada i tornada en el seu propi format, i desapareixen en la conversió |

**Tornar a escriure un fitxer en el seu propi format no perd res** — és la garantia
que sosté tota la resta, i `--to srt` sobre un `.srt` torna els mateixos bytes. Ni tan sols
es descodifica res: el text continua sent la cadena en brut que es va llegir.

## Les etiquetes es tradueixen, no es copien

**Cada format escriu la cursiva a la seva manera**, i convertir sense traduir
deixaria un `{\i1}` en un `.srt`: un text que l'usuari veu, en un
fitxer que mai no sabrà interpretar-lo. El que passa són **sis
coses** — negreta, cursiva, subratllat, color, tipus de lletra i mida:

| Format | Negreta | Cursiva | Subratllat | Color | Tipus de lletra | Mida |
| :----- | :--: | :------: | :------: | :-----: | :----: | :----: |
| SubRip | `<b>` | `<i>` | `<u>` | `<font color="#RRGGBB">` | — | — |
| WebVTT | `<b>` | `<i>` | `<u>` | — | — | — |
| SubViewer 2 | `<b>` | `<i>` | `<u>` | `<font color="#RRGGBB">` | — | — |
| Sub Station Alpha | `{\b1}` | `{\i1}` | — | `{\c&HBBGGRR&}` | `{\fnNAME}` | `{\fsN}` |
| Advanced SSA | `{\b1}` | `{\i1}` | `{\u1}` | `{\c&HBBGGRR&}` | `{\fnNAME}` | `{\fsN}` |
| MicroDVD | `{Y:b}` | `{Y:i}` | `{Y:u}` | `{C:$BBGGRR}` | `{F:NAME}` | `{S:N}` |
| MPL2 | `\` | `/` | `_` | `{C:$BBGGRR}` | `{F:NAME}` | `{S:N}` |
| TMPlayer | — | — | — | — | — | — |
| LRC | — | — | — | — | — | — |

**Un guió és una pèrdua anunciada.** Convertir un `.srt` amb color a WebVTT suprimeix
el color i ho diu; el fitxer produït és WebVTT, no SubRip disfressat.

**MicroDVD i MPL2 no poden donar estil a una part d'una línia.** Les seves etiquetes
no tenen final: `{y:i}` inclina tot el que segueix fins al final de la línia, i
`{Y:i}` fins al final del subtítol. Una línia que només era en negreta a la meitat
s'escriu, per tant, sense cap negreta, en lloc d'amb una negreta que es desborda.

## Què diu la conversió que ha perdut

**Ho diu quan perd, i calla quan no perd res.** Un informe
que apareix a cada crida és un informe que ningú no llegeix.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,500\n<i>sur deux</i>\nlignes\n\n' > a.srt; subedit-cli convert --to lrc --output b.lrc a.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,500\n<i>sur deux</i>\nlignes\n\n' > a.srt; subedit-cli convert --to lrc --output b.lrc a.srt
a.srt: 1 subtitle written as LRC -> b.lrc
a.srt: ends are not carried by LRC, line breaks were joined in 1 subtitle, 1 tag dropped
```

La línia apareix al nivell de verbositat per defecte, i a tots els superiors.
Els elements, en aquest ordre:

| Element | Què diu |
| :---- | :----------- |
| `ends are not carried by <format>` | el format de destinació no escriu cap final |
| `line breaks were joined in N subtitles` | només LRC: N subtítols eren en diverses línies |
| `N tags dropped` | N etiquetes no s'han pogut escriure, disposició inclosa |
| `the header was dropped` | la capçalera no travessa una frontera de format |
| `the <format> fields of N subtitles were dropped` | N subtítols portaven dades pròpies del seu format |
| `positions moved by up to N ms` | la distància més gran que s'ha mogut una posició |

## Canviar la codificació

**`--to-encoding` tria la codificació escrita; `--encoding` diu la que es
llegeix.** Totes dues juntes recodifiquen un fitxer:

```console
$ subedit-cli convert --to srt --encoding cp1252 --to-encoding utf-8 \
      --output-dir sortie film.srt
```

Sense `--to-encoding`, el fitxer s'escriu en **la codificació en què s'ha llegit**,
com passa amb els seus finals de línia i la seva marca: un fitxer convertit sense cap
instrucció sobre la codificació torna els mateixos caràcters, amb els mateixos bytes.

**Un caràcter que la codificació de destinació no pot escriure atura el fitxer**, i
no s'escriu res. Una `ł` no té lloc en Latin-1; substituir-la per un `?`
perdria text entre la lectura i l'escriptura, sense que ningú ho veiés.

**Una marca d'ordre de bytes només existeix per a les codificacions Unicode.**
`--bom` amb un `--to-encoding` que no en porta es refusa en lloc d'ignorar-se
— vegeu la taula d'errors més avall.

**I la codificació ha d'anomenar el seu ordre de bytes.** `UTF-16` i `UTF-32` són
noms que ICU coneix, el convertidor dels quals escriu **una marca pròpia**, sigui el que sigui
el que se li demani: `--no-bom` aleshores s'acceptaria i es desobeiria. Per tant
tots dos es refusen, i el missatge anomena la sortida:

```console
$ subedit-cli convert --to srt --to-encoding UTF-16 --output copie.srt film.srt
"UTF-16" writes a byte order mark of its own; name the byte order, as UTF-16LE and UTF-16BE do
```

`UTF-16LE`, `UTF-16BE`, `UTF-32LE` i `UTF-32BE` escriuen exactament els mateixos
bytes, marca inclosa — però sota el control de `--bom` i `--no-bom`. Per tant el
refús no costa res més que un nom més precís, i val tant per a
la lectura com per a l'escriptura: `--encoding UTF-16` rep la mateixa resposta.

## La destinació

Les tres opcions i les seves regles són comunes a les subordres que
escriuen: vegeu [Invocació](invocation.md#la-destinació). No s'escriu res
sense una d'elles.

El que és propi de `convert`: **`--output-dir` canvia l'extensió amb el
format.** `convert --to vtt` sobre `a.srt` escriu `a.vtt`, mai un `a.srt` que contingui
WebVTT — un fitxer el nom del qual menteix fa ensopegar totes les altres eines.

## Convertir al mateix lloc es refusa

`--in-place` amb un `--to` que canvia el format dona el codi `1` i no escriu
res: al mateix lloc, no hi ha cap segon nom per portar el nou format, i
el fitxer quedaria sota una extensió que el seu contingut ja no justifica.

`--in-place` encara es pot fer servir per canviar **la forma sense el format** —
tornar a escriure un `.srt` com a `.srt` amb uns altres finals de línia, per exemple.

El refús es decideix només per **l'extensió**, abans de cap lectura: un error
d'ús no ha de deixar mai un lot escrit a mitges.

## Finals de línia i marca d'ordre de bytes

Per defecte, **el fitxer escrit reprèn el que portava el fitxer llegit**. El model
conserva totes dues coses en llegir; imposar-les per defecte faria perdre, a cada
conversió, una informació conservada expressament.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nBonjour.\n\n' > a.srt; subedit-cli convert --to vtt --output b.vtt a.srt; cat b.vtt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nBonjour.\n\n' > a.srt; subedit-cli convert --to vtt --output b.vtt a.srt; cat b.vtt
a.srt: 1 subtitle written as WebVTT -> b.vtt
WEBVTT

00:01.000 --> 00:03.000
Bonjour.
```

## Ordenar en escriure

`--sort` escriu el fitxer **en l'ordre dels començaments**, sigui quin sigui el format de destinació. L'ordenació es fa **sobre una còpia**: el fitxer llegit no
es toca, i el que s'escriu és el que n'hauria fet [`sort`](sort.md) — estable: dos subtítols que comencen alhora conserven el seu ordre.

**Mai per defecte**: l'ordre d'un fitxer és el del seu autor mentre no es demani res més. Amb l'opció, cada fitxer ho diu, al
primer nivell de narració, `<path>: N subtitles moved` o `<path>: already in order`, i el registre `--format json` porta un recompte
`moved` — **absent sense l'opció**, per no canviar la forma del que llegeixen els scripts existents.

## Fer disjuntes les franges

`--disjoint` talla els subtítols que s'encavalquen perquè cap franja no s'encavalqui ja amb una altra, **sobre la mateixa còpia que l'ordenació**: el fitxer llegit no es toca.

Mai per defecte. L'opció fa el que fa [`disjoint`](disjoint.md), **amb els seus paràmetres per defecte**, abans d'escriure, després de l'ordenació. Necessita un fitxer en ordre: altrament es refusa, i `--sort` — que va primer — el posa en ordre abans. El fitxer ho diu al primer nivell de narració (`<path>: N subtitles split, M merged`), i el registre `--format json` porta els recomptes `split` i `merged`, **absents sense l'opció**.

**`--join-repeats` va abans de l'ordenació i del tall**: un text tallat en fragments que se segueixen es reconstrueix primer (vegeu
[`join-repeats`](join-repeats.md)), després s'ordenen els subtítols, i després es fan disjuntes les franges. **`--repair` és les tres coses alhora**,
com [`repair`](repair.md), amb els paràmetres per defecte del tall. Amb `--join-repeats`, cada fitxer diu
`<path>: N repeated subtitles joined` al primer nivell de narració, i el registre `--format json` porta `counts.joined` — **absent sense l'opció**.

## Sortida

**Sortida estàndard** — res: el resultat és el fitxer escrit. Amb `--dry-run`, tampoc res: no hi ha cap fitxer, i la línia de narració ho diu — vegeu [Veure abans d'escriure](invocation.md#veure-abans-descriure).

A la sortida d'error, segons el nivell:

| Nivell | Què s'hi afegeix |
| :----- | :-------------- |
| 1 | `<path>: N subtitles written as WebVTT -> <destination>` |
| 2 | `<path>: SubRip -> WebVTT, UTF-8, no BOM, LF line endings` — la codificació anomenada és l'escrita |
| 3 | `<path>: N bytes read, M written`, i després **cada diagnòstic de lectura** — vegeu [Invocació](invocation.md#diagnòstics-de-lectura) |

### En JSON

Amb [`--format json`](invocation.md#sortida-llegible-per-un-script), l'objecte de cada fitxer porta `counts.subtitles`, i després el que la conversió ha perdut: `lost_ends` i `lost_header` (`0` o `1`), `joined_lines`, `lost_tags`, `lost_fields` i `furthest_ms` — els elements de [la frase de pèrdua](#què-diu-la-conversió-que-ha-perdut). La taula completa és la d'[Invocació](invocation.md#què-posa-cada-subordre-a-counts).

## Codis de sortida

Els de l'eina: `0` si s'han escrit tots els fitxers, `2` si no se n'ha escrit cap, `3` si
només alguns, `1` en un error d'ús.

## Errors

| Què el provoca | Missatge |
| :------------------ | :------ |
| `--bom` amb `--no-bom` | `--bom and --no-bom ask for opposite things; give one or the other` |
| `--in-place` que canvia el format | `--in-place cannot change the format: the file would keep a name its content no longer matches` |
| `--bom` sobre una codificació que no en té | `<path>: <encoding> has no byte order mark to write` |
| `--to-encoding` que anomena una codificació desconeguda | `no encoding is named "<name>"` |
| `--to-encoding` que anomena una codificació que escriu la seva pròpia marca | `"<name>" writes a byte order mark of its own; name the byte order, as UTF-16LE and UTF-16BE do` |
| caràcter que falta a la codificació escrita | `<path>: holds a character the chosen encoding cannot write` |

**Refusar `--bom` sobre una codificació que no en porta no és
pedanteria.** Una marca d'ordre de bytes existeix per a les codificacions Unicode i
per a cap altra: demanar-ne una per a un fitxer en Windows-1252 és demanar
una cosa que no existeix. El fitxer s'escriuria sense, i ningú no
sabria que la petició no s'ha atès.

Els de la destinació són comuns a les subordres que escriuen:
vegeu [Invocació](invocation.md#la-destinació).

Els errors de lectura són els d'[`inspect`](inspect.md): afecten
el fitxer d'entrada i tenen els mateixos missatges.
