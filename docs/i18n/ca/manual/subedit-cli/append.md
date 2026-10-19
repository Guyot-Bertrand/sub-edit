# `append`

<!-- machine-translated -->
> Aquesta pàgina és una traducció automàtica i encara no l'ha revisada cap lector de la llengua.

```
subedit-cli append --output FILE
                   [--dry-run] [--sort] [--disjoint]
                   <base> <file>...
```

Posa fitxers **després del primer**, en **un sol** fitxer: la pel·lícula en dues parts, convertida en un
sol joc de subtítols. És `Append File…` de la finestra — [el mateix motor](../subedit-gui/operations.md#append-file),
els mateixos recomptes — aplicat tantes vegades com fitxers hi ha per afegir.

- **El primer fitxer és la base.** Dona al resultat **el seu format, la seva codificació, la seva marca
  d'ordre de bytes i els seus finals de línia**.
- **No s'insereix res entre dos fitxers**: cadascun es desplaça fins a **el final de l'últim subtítol del que
  el precedeix**, i les seves posicions conserven el seu ordre i la seva separació. El tercer segueix el segon, no la
  base: els fitxers se succeeixen en l'ordre en què es donen.
- **Cada fitxer afegit passa per la conversió** al format de la base: les seves etiquetes es tradueixen,
  i el que aquest format no pot escriure es diu, fitxer per fitxer, amb les paraules de la finestra.
- **Els fitxers afegits no tenen traducció**: un fitxer obert sol no en té cap per donar.
- **Un fitxer es pot donar diverses vegades**, o afegir-se a si mateix: res no ho prohibeix.

**`append` no és un lot.** N entrades, **una** sortida: el bucle de les altres subordres — un resultat
per fitxer, i un error que deixa continuar els altres — aquí no té sentit. Se'n deriven tres coses:

- **la destinació és `--output`, i res més.** `--output-dir` i `--in-place` no existeixen per a
  `append` i es refusen com qualsevol opció desconeguda, codi `1`; `--recursive` igualment: l'ordre dels
  fitxers és el que es dona, i un arbre recorregut no en té cap;
- **tot o res**: el primer fitxer que no es pot llegir atura l'execució, i **no s'escriu res**. Una
  pel·lícula en tres parts de la qual en falta una no és una pel·lícula en dues;
- **una sola línia per execució** per al resultat, i, en JSON, **un sol objecte**.

<!-- example: subedit-cli append --help -->
```console
$ subedit-cli append --help
Put subtitle files one after another into a single file
Usage: subedit-cli append [OPTIONS] files...

Positionals:
  files BASE FILE...          The base file, then the files to put after it, in order

Options:
  -h,--help                   Print this help message and exit
  --output FILE               File to write
  --dry-run                   Work out and say what would be written, and write nothing
  --sort                      Put the subtitles in order of their start before writing
  --disjoint                  Cut overlapping subtitles so that no range overlaps another, before writing
```

## Arguments i opcions

| Opció | Obligatori | Valors | Per defecte |
| :----- | :----- | :------ | :----- |
| `<base> <file>...` | sí, **almenys dos** | camins: la base, i després el que es posa al darrere, en ordre | — |
| `--output` | **sí**, excepte amb `--dry-run` | un camí de fitxer; el seu directori es crea si falta | — |
| `--dry-run` | no | un indicador: calcula i diu, no escriu res — vegeu [Veure abans d'escriure](invocation.md#veure-abans-descriure) | desactivat |
| `--sort` | no | un indicador: posa el conjunt en l'ordre dels seus començaments **un cop tot afegit**, vegeu [Ordenar el resultat](#ordenar-el-resultat) | desactivat |
| `--disjoint` | no | un indicador: talla els subtítols que s'encavalquen **un cop tot afegit i ordenat**, vegeu [Fer disjuntes les franges](#fer-disjuntes-les-franges) | desactivat |

**La destinació no pot ser cap de les entrades**, inclosa la base: escriure sobre un fitxer que s'està
llegint es refusa abans de llegir res, codi `1`. Un `--output` existent diferent de les entrades se
sobreescriu, com arreu.

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nun\n\n2\n00:00:03,000 --> 00:00:04,000\ndeux\n\n' > partie1.srt; printf '1\n00:00:00,500 --> 00:00:01,500\ntrois\n\n' > partie2.srt; subedit-cli append --output film.srt partie1.srt partie2.srt; cat film.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nun\n\n2\n00:00:03,000 --> 00:00:04,000\ndeux\n\n' > partie1.srt; printf '1\n00:00:00,500 --> 00:00:01,500\ntrois\n\n' > partie2.srt; subedit-cli append --output film.srt partie1.srt partie2.srt; cat film.srt
partie2.srt: appended 1 subtitle
partie1.srt: 3 subtitles from 2 files -> film.srt
1
00:00:01,000 --> 00:00:02,000
un

2
00:00:03,000 --> 00:00:04,000
deux

3
00:00:04,500 --> 00:00:05,500
trois
```

L'últim subtítol de `partie1.srt` acaba a 4 s; el de `partie2.srt`, que començava a 0,5 s, comença a
4,5 s.

## Ordenar el resultat

Cada fitxer es desplaça **a partir del final del que el precedeix**: l'ordre que es pot trencar és, doncs, el dels mateixos fitxers. `--sort`
posa el resultat en l'ordre dels començaments, **d'una sola vegada, després de l'última addició**, com ho faria [`sort`](sort.md) — de manera estable, i sense canviar res
del contingut.

Mai per defecte. Amb l'opció, la base ho diu al primer nivell de narració, `<base>: N subtitles moved` o `<base>: already in order`, i
el registre de `--format json` porta un recompte `moved`, **absent sense l'opció**.

## Fer disjuntes les franges

Mai per defecte. L'opció fa el que fa [`disjoint`](disjoint.md), **amb els seus paràmetres per defecte**, un cop tot afegit, sobre el conjunt. Necessita un fitxer en ordre: altrament es refusa, i `--sort` — que va primer — l'ordena abans. El fitxer ho diu al primer nivell de narració (`<path>: N subtitles split, M merged`), i el registre de `--format json` porta els recomptes `split` i `merged`, **absents sense l'opció**.

## Quan els formats són diferents

El resultat és en **el format de la base**, sigui quin sigui el dels altres. El que costa el pas d'un format a l'altre
es diu fitxer per fitxer, amb les paraules de la finestra — la mateixa frase que `Append File…` i que
[enganxar des d'un altre format](../subedit-gui/clipboard.md#les-etiquetes-dun-altre-format-es-tradueixen):

```text
d.ass: appended 1 subtitle; Advanced SSA into SubRip: 1 tag dropped, the header was dropped
```

El que es perd només depèn del format de **cada** fitxer afegit: una base en Advanced SSA rep un
fitxer SubRip sense perdre res, però no a la inversa. Per triar el format del resultat, poseu primer un fitxer
que ja sigui en aquest format, o convertiu-lo després amb [`convert`](convert.md).

## Sortida

**Sortida estàndard** — res: el resultat és el fitxer escrit.

| Nivell | Què s'afegeix a la sortida d'error |
| :----- | :------------------------------------- |
| 1 | una línia per fitxer afegit — `<path>: appended N subtitles`, seguida, quan el format ha costat alguna cosa, de `; <source format> into <base format>: <what is lost>` — i després `<base>: N subtitles from M files -> <destination>` (amb `--dry-run`, `(dry run, nothing written)` en lloc de `-> <destination>`) |
| 2 | `<base>: SubRip, UTF-8, no BOM, LF line endings kept` — el format, la codificació, la marca i els finals de línia de la base, restituïts tal com eren |
| 3 | `<base>: N bytes read, M written` (tots els fitxers junts), i després **cada diagnòstic de lectura** — vegeu [Invocació](invocation.md#diagnòstics-de-lectura) |

### En JSON

Amb [`--format json`](invocation.md#sortida-llegible-per-un-script), **un sol objecte** per a tota l'execució,
el `file` del qual és **la base**. Els seus `counts`:

| Recompte | Què diu |
| :----- | :----------- |
| `files` | els fitxers de l'execució, inclosa la base |
| `subtitles` | els subtítols del resultat |
| `appended` | els que vénen dels fitxers afegits |
| `lost_ends`, `lost_header` | `0` o `1`: un dels fitxers afegits l'ha perdut |
| `joined_lines`, `lost_tags`, `lost_fields` | la suma del que han perdut els fitxers afegits |
| `furthest_ms` | el desplaçament més gran d'una posició, tots els fitxers junts |

**Una matriu `inputs`** acompanya els `counts`, fora d'ells: un element per fitxer afegit, en ordre,
`{"file", "counts"}` — amb `counts` que porta `appended` i després les mateixes pèrdues, **només per a aquest fitxer**. És el que
diu, en una execució de deu fitxers, quin ha costat què. Els avisos de lectura de tots els fitxers
s'apleguen a `warnings`.

Un error també és un objecte, **un de sol**: el del primer fitxer que no s'ha pogut llegir.

## Codis de sortida

| Codi | Quan |
| :--- | :---- |
| `0` | el resultat s'ha escrit — o calculat, amb `--dry-run` |
| `1` | error d'ús: menys de dos fitxers, cap `--output`, una opció que `append` no té, una destinació que és una de les entrades |
| `2` | **un fitxer no s'ha pogut llegir**, o la destinació no s'ha pogut escriure: no s'escriu res. No hi ha codi `3`: una execució que només escriu un fitxer té èxit o no en té |

## Errors

| Què els provoca | Missatge |
| :------------------- | :------ |
| un sol fitxer donat | `append needs a base file and at least one file to put after it` |
| ni `--output` ni `--dry-run` | `no destination given: append writes one file, use --output` |
| `--output` anomena una entrada | `<destination>: would be written over the input <input>` |
| un fitxer que falta, il·legible o d'un format desconegut | `<path>: does not exist`, `<path>: is in no format this tool knows`… — el mateix que arreu |

**Un fitxer buit no s'afegeix**: cap format no llegeix sense almenys un subtítol, i el fitxer es
refusa com qualsevol fitxer que no es pot llegir — `<path>: holds nothing recognisable as a subtitle` —, cosa que
atura l'execució i no escriu res.
