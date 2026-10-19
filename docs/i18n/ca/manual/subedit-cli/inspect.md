# `inspect`

<!-- machine-translated -->
> Aquesta pàgina és una traducció automàtica i encara no l'ha revisada cap lector de la llengua.

```
subedit-cli inspect [--frame-rate RATE] [--recursive] [--detail]
                   [-t FILE] [--align-method position|number] <file>...
```

Informa de què està fet cada fitxer. **No canvia res i no escriu cap
fitxer.**

<!-- example: subedit-cli inspect --help -->
```console
$ subedit-cli inspect --help
Report what a subtitle file is made of
Usage: subedit-cli inspect [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to report on

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --detail                    List every anomaly, subtitle by subtitle, instead of counting them
  --frame-rate RATE           Frame rate of a file counted in frames: 25, 23.976
  -t,--translation-file FILE  Translation file to lay over the subtitle file, for a single input
  --align-method position|number
                              How the lines of the translation find their subtitles
```

## Arguments

| Argument | Obligatori | Valor | Per defecte |
| :------- | :----- | :----- | :----- |
| `<file>...` | sí | un o més camins de fitxers de subtítols | — |
| `--recursive`, `-r` | no | un indicador | desactivat — vegeu [Tractar un arbre](lots.md) |
| `--detail` | no | un indicador: anomena cada anomalia, en lloc de comptar-les — vegeu [Què indica `anomalies`](#què-indica-anomalies) | desactivat |

No donar cap camí és un error d'ús, i per tant codi `1`.

## Opcions

| Opció | Obligatori | Valor | Per defecte |
| :----- | :------ | :----- | :----- |
| `--frame-rate` | no | una freqüència en fotogrames per segon: `25`, `23.976`, `29.97` | 23.976 |

**Només serveix per als fitxers comptats en fotogrames**, és a dir MicroDVD, l'únic
dels nou. Un fitxer MicroDVD només porta números de fotograma; la
freqüència a la qual es llegeixen decideix **totes les posicions de l'informe**,
començant per `span`. Donada en un altre cas, no canvia res i no és un
error.

Un valor que no anomena res és un error d'ús, i per tant codi `1`:

<!-- example: subedit-cli inspect --frame-rate douze film.sub; echo $? -->
```console
$ subedit-cli inspect --frame-rate douze film.sub; echo $?
--frame-rate: "douze" is not a frame rate: expected frames per second, like 25 or 23.976
1
```

És la mateixa opció que a [`convert`](convert.md), paraula per paraula, perquè
és la mateixa freqüència. `convert` també la fa servir per **escriure**.

## Una traducció: `-t`

| Opció | Obligatori | Valor | Per defecte |
| :----- | :------ | :----- | :----- |
| `-t`, `--translation-file` | no | un fitxer de traducció, per a **una sola** entrada | — |
| `--align-method` | no, i només amb `-t` | `position` o `number` | `position` |

Amb `-t`, la traducció es col·loca sobre el fitxer **tal com ho fa la finestra en obrir-la** — vegeu
[Una traducció](invocation.md#una-traducció) — i l'informe diu **com les seves línies han trobat el seu
subtítol**. Així un script sap si una traducció està alineada abans de corregir-la.

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nHello.\n\n2\n00:00:03,000 --> 00:00:04,000\nBye.\n\n' > film.srt; printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour.\n\n' > film.fr.srt; subedit-cli --quiet inspect -t film.fr.srt film.srt | tail -3 -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nHello.\n\n2\n00:00:03,000 --> 00:00:04,000\nBye.\n\n' > film.srt; printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour.\n\n' > film.fr.srt; subedit-cli --quiet inspect -t film.fr.srt film.srt | tail -3
  anomalies: none
  translation file: film.fr.srt, matched by position
  translation: 1 line attached; 1 subtitle left without a translation
```

La primera línia afegida anomena el fitxer i el mètode; la segona és **la frase que diu la
finestra** quan obre una traducció, cada part només quan no és zero:

| Part | Què compta |
| :----- | :---------------- |
| `N lines attached` | les línies de la traducció que han trobat un subtítol del fitxer principal |
| `N subtitles born of a line` | les línies que no n'han trobat cap i n'han fet néixer un de propi, a les seves pròpies posicions |
| `N subtitles left without a translation` | els subtítols del fitxer principal als quals no ha arribat cap línia |
| `N lines out of order` | les línies que venien després d'una de posterior al fitxer |

Una traducció que no porta cap línia diu `translation: the file holds no line`. **L'informe del
fitxer mateix continua sent el del fitxer sol**: un subtítol nascut d'una línia no es compta a
`subtitles`.

Els errors de `-t` són els d'una lectura, dits amb el camí de la traducció, i aquest:
`<translation>: the file is already open as the main document`, quan el fitxer donat és el principal.
`--align-method` sense `-t` és un error d'ús, i també ho és `-t` amb diverses entrades — vegeu
[Una traducció](invocation.md#una-traducció).

## Sortida

L'informe va a la **sortida estàndard** — és el resultat de l'ordre, i
s'escriu a tots els nivells, `--quiet` inclòs. Un bloc per fitxer, precedit
pel camí:

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,500\nLe canot dérive.\n\n2\n00:00:04,000 --> 00:00:06,200\nPersonne n a vu la côte.\n' > exemple.srt; subedit-cli --quiet inspect exemple.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,500\nLe canot dérive.\n\n2\n00:00:04,000 --> 00:00:06,200\nPersonne n a vu la côte.\n' > exemple.srt; subedit-cli --quiet inspect exemple.srt
exemple.srt
  format: SubRip
  encoding: UTF-8, detected
  byte order mark: absent
  line endings: LF
  subtitles: 2
  span: 00:00:01.000 -> 00:00:06.200
  frame rate grid: none (too few subtitles to tell)
  anomalies: none
```

| Camp | Què diu |
| :---- | :----------- |
| `format` | un dels formats que accepta `--to`, escrit sencer: `SubRip`, `WebVTT`, `SubViewer 2`, `Sub Station Alpha`, `Advanced SSA`, `MicroDVD`, `MPL2`, `TMPlayer`, `LRC` |
| `encoding` | la codificació llegida, seguida de **d'on ve la resposta**: `, from its byte order mark` si el fitxer la declara, `, as asked for` si `--encoding` l'ha imposada, `, detected` si s'ha sospesat |
| `byte order mark` | `present` o `absent` |
| `line endings` | `LF`, `CRLF` o `CR`, seguit de `, mixed from line N` si el fitxer els barreja |
| `subtitles` | el nombre de subtítols llegits |
| `span` | del començament més primerenc al final més tardà, `HH:MM:SS.mmm` |
| `frame rate grid` | la freqüència de fotogrames sobre la qual s'han calculat les posicions, **deduïda** — vegeu més avall |
| `frame rate` | per a un fitxer comptat en fotogrames, la freqüència a la qual s'ha llegit i **d'on ve** — substitueix la línia anterior, vegeu més avall |
| `anomalies` | `none` a la mateixa línia, o el que no va bé: **un tipus per línia amb el seu recompte**, sagnat sota `anomalies:`; amb `--detail`, **una anomalia per línia**, subtítol per subtítol |

**`span` no és «del primer a l'últim»** sinó del més primerenc al més tardà:
en un fitxer amb l'ordre trencat, tots dos difereixen, i només el segon diu
la veritat sobre el que cobreix el fitxer.

**`anomalies`, amb `--detail`, compta els subtítols a partir d'1**, tal com els numera
el fitxer — i no les línies del fitxer. Un número de subtítol sobreviu a un
canvi; un número de línia, no. Vegeu més avall.

## Què indica `frame rate grid`

**Un fitxer de subtítols no declara la seva freqüència de fotogrames.** SubRip no té
capçalera, la de WebVTT és text lliure. Per tant aquesta línia no llegeix res:
**dedueix**, mesurant si les posicions cauen sobre una graella de fotogrames.

El que es mesura són els **començaments**. Allà on hi ha una graella hi són
sense excepció, mentre que els finals sovint se n'aparten — un final
sovint es calcula amb una regla de velocitat de lectura en lloc de posar-lo sobre
un fotograma.

La primera línia sempre diu el mateix: la freqüència triada, el veredicte,
i la concentració que el sosté.

| Veredicte | Què significa |
| :------ | :----------------- |
| `clean` | les posicions són sobre una graella |
| `partial` | una part hi és, la resta no |
| `none` | cap candidat no explica res |

`none` té **dues causes, i no són la mateixa resposta**: o bé cap
graella no encaixa — aleshores la línia dona el millor candidat i la seva puntuació,
perquè es vegi per quant falla — o bé el fitxer té massa pocs
subtítols perquè se'n pugui dir res. Dos començaments sempre semblen
perfectament alineats; això no prova res.

**Un fitxer regular sobre una freqüència no estàndard dona `none`, i és
intencionat.** El conjunt de candidats és tancat — les vuit freqüències que ofereix la
conversió — i indicar la menys errònia seria donar
una resposta errònia allà on «no ho sé» és la correcta.

### Les línies que segueixen, quan tenen alguna cosa a dir

| Línia | Quan apareix |
| :---- | :------------------ |
| `grid offset` | les posicions són sobre la graella **amb una constant de diferència**: el fitxer s'ha desplaçat, i aquí es diu quant |
| `also fits` | un altre candidat encaixa igual de bé, perquè és un múltiple enter del triat |
| `too short a span to separate` | l'abast del fitxer no permet distingir la freqüència triada de les anomenades |
| `off the grid` | quants començaments s'aparten de la graella, i en quantes **sèries** |

**`also fits` no és una vacil·lació.** Una graella a 25 fotogrames per segon és
*inclosa* en una graella a 50: per tant un fitxer sincronitzat a 25 encaixa en totes dues.
El contrari és fals — un 50 real s'esfondra sobre 25 —, així que es tria la més
baixa, i l'altra s'anomena en lloc de callar-la. Les úniques
parelles afectades són 25 i 50, 29.97 i 59.94, 30 i 60.

**`too short a span to separate` protegeix d'una resposta massa segura.** 23.976
i 24 se separen un mil·lisegon per segon de pel·lícula: en un llargmetratge es
distingeixen sense dubte, en deu segons no es poden distingir. La línia diu
quina continua sent possible.

**`off the grid` distingeix dues històries que el percentatge sol confon.**
Moltes sèries d'un sol començament són posicions corregides a mà,
una per una. Unes poques sèries llargues són una secció resincronitzada o un fitxer
muntat a partir de dos altres. El nombre de sèries és el que les separa.

### Un exemple

Dotze línies posades sobre una graella a 25 fotogrames per segon — és a dir a
posicions múltiples de 40 mil·lisegons — repartides en gairebé nou minuts:

<!-- example: k=0; while [ $k -lt 12 ]; do s=$((1000 + k*47000 + (k%3)*40)); e=$((s+2000)); k=$((k+1)); printf '%d\n%02d:%02d:%02d,%03d --> %02d:%02d:%02d,%03d\nRéplique %d.\n\n' $k $((s/3600000)) $((s/60000%60)) $((s/1000%60)) $((s%1000)) $((e/3600000)) $((e/60000%60)) $((e/1000%60)) $((e%1000)) $k; done > grille.srt; subedit-cli --quiet inspect grille.srt | tail -3 -->
```console
$ k=0; while [ $k -lt 12 ]; do s=$((1000 + k*47000 + (k%3)*40)); e=$((s+2000)); k=$((k+1)); printf '%d\n%02d:%02d:%02d,%03d --> %02d:%02d:%02d,%03d\nRéplique %d.\n\n' $k $((s/3600000)) $((s/60000%60)) $((s/1000%60)) $((s%1000)) $((e/3600000)) $((e/60000%60)) $((e/1000%60)) $((e%1000)) $k; done > grille.srt; subedit-cli --quiet inspect grille.srt | tail -3
  frame rate grid: 25 fps, clean (100.0%)
  also fits: 50 fps, of which this rate is a whole divisor
  anomalies: none
```

L'exemple fabrica el seu propi fitxer en lloc de llegir-ne un del dipòsit, i és
deliberat: es pot reproduir tal com és, i el bucle mostra què és una graella
millor que una frase — posicions múltiples d'una durada de fotograma.

## Què indica `frame rate`, i per què no és una graella

**Només un dels nou formats compta en fotogrames**: MicroDVD, cada línia del qual
porta dos números de fotograma i res més. En un fitxer així, `inspect`
no escriu `frame rate grid` sinó `frame rate`, i les dues línies no es mostren
mai juntes.

**La raó cap en una frase.** Les posicions d'un fitxer MicroDVD s'han
calculat *a partir* dels seus fotogrames, a una freqüència donada; deduir-les d'una
graella tornaria aquella mateixa freqüència, tal com s'acaba de proporcionar. Seria
una xifra donada, disfressada de mesura.

El que s'escriu en lloc d'això és **la freqüència de lectura i d'on ve** —
la mateixa forma que la línia `encoding`:

| Què diu la línia | Quan |
| :------------------ | :---- |
| `frame rate: 25 fps, as asked for` | `--frame-rate` l'ha imposada |
| `frame rate: 24000/1001 fps, assumed` | ningú no l'ha dita; és el valor per defecte |

**Dues respostes i no tres**, allà on `encoding` en té tres: un fitxer MicroDVD
no declara res. Aquest és tot el problema — la freqüència ve necessàriament
de fora, i l'informe diu de quin dels dos «fora».

**La freqüència decideix tota la resta de l'informe.** `span` hi passa, i dues
lectures del mateix fitxer a dues freqüències donen dos abasts:

<!-- example: printf '{25}{75}Première.\n{100}{150}Deuxième.\n{200}{260}Troisième.\n' > images.sub; subedit-cli --quiet inspect images.sub | tail -3; subedit-cli --quiet inspect --frame-rate 25 images.sub | tail -3 -->
```console
$ printf '{25}{75}Première.\n{100}{150}Deuxième.\n{200}{260}Troisième.\n' > images.sub; subedit-cli --quiet inspect images.sub | tail -3; subedit-cli --quiet inspect --frame-rate 25 images.sub | tail -3
  span: 00:00:01.043 -> 00:00:10.844
  frame rate: 24000/1001 fps, assumed
  anomalies: none
  span: 00:00:01.000 -> 00:00:10.400
  frame rate: 25 fps, as asked for
  anomalies: none
```

## Què indica `anomalies`

En un document hi pot haver tres coses malament, i cadascuna es repara de manera diferent.
**Per defecte `inspect` les compta**, una línia per tipus, en l'ordre de la taula:
un fitxer mal fet en porta centenars, i saber quantes n'hi ha de cada tipus
és el que diu què cal executar. **`--detail` les anomena una per una.**

| Què s'escriu | Amb `--detail` | Què significa |
| :--------------- | :-------------- | :------------------ |
| `subtitle ends before it starts: N` | `subtitle N ends before it starts` | el final arriba abans del començament |
| `subtitle starts before the previous one ends: N` | `subtitle N starts before the previous one ends` | s'encavalca amb l'anterior |
| `subtitle starts before the previous one starts: N` | `subtitle N starts before the previous one starts` | trenca l'ordre del fitxer — [`sort`](sort.md) el torna a posar al seu lloc |

Un tipus absent no s'escriu. A la columna de l'esquerra, `N` és el nombre
de subtítols; a la següent, el número d'un subtítol.

**El mateix subtítol pot aparèixer dues vegades**, i és intencionat: el que
comença abans que l'anterior hagi començat també comença abans que hagi
acabat. El primer punt es resol movent el subtítol, el segon
canviant una durada.

**El subtítol anomenat és el que està mal col·locat**, no aquell respecte del qual està
mal col·locat. Amb començaments a `0`, `4 s`, `2 s`, `3 s`: el tercer trenca l'ordre, el
quart sí que segueix el tercer i per tant no s'anomena — no hi ha res a
fer-hi.

<!-- example: printf '1\n00:00:00,000 --> 00:00:00,500\nA\n\n2\n00:00:04,000 --> 00:00:04,500\nB\n\n3\n00:00:02,000 --> 00:00:02,500\nC\n\n4\n00:00:03,000 --> 00:00:03,500\nD\n' > desordre.srt; subedit-cli --quiet inspect desordre.srt | tail -3 -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,500\nA\n\n2\n00:00:04,000 --> 00:00:04,500\nB\n\n3\n00:00:02,000 --> 00:00:02,500\nC\n\n4\n00:00:03,000 --> 00:00:03,500\nD\n' > desordre.srt; subedit-cli --quiet inspect desordre.srt | tail -3
  anomalies:
    subtitle starts before the previous one ends: 1
    subtitle starts before the previous one starts: 1
```

I amb `--detail`, el subtítol anomenat:

<!-- example: printf '1\n00:00:00,000 --> 00:00:00,500\nA\n\n2\n00:00:04,000 --> 00:00:04,500\nB\n\n3\n00:00:02,000 --> 00:00:02,500\nC\n\n4\n00:00:03,000 --> 00:00:03,500\nD\n' > desordre.srt; subedit-cli --quiet inspect --detail desordre.srt | tail -3 -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,500\nA\n\n2\n00:00:04,000 --> 00:00:04,500\nB\n\n3\n00:00:02,000 --> 00:00:02,500\nC\n\n4\n00:00:03,000 --> 00:00:03,500\nD\n' > desordre.srt; subedit-cli --quiet inspect --detail desordre.srt | tail -3
  anomalies:
    subtitle 3 starts before the previous one ends
    subtitle 3 starts before the previous one starts
```

**Una línia per anomalia amb `--detail`**, perquè cent constatacions
enganxades en una línia no es poden ni llegir ni cercar. Un document sense cap defecte conserva
la seva única línia, `anomalies: none`, que és la que cerca un script. **El
registre `--format json` sempre llista cada anomalia**: un script llegeix el detall, i
`--detail` només canvia el text.

**No és un diagnòstic de lectura.** Els diagnòstics diuen amb què s'ha trobat la
lectura i assenyalen una **línia del fitxer**; les anomalies diuen
què *és* el document i assenyalen un **subtítol**. Un fitxer amb l'ordre
trencat no és pas mal format per això: la lectura no hi té res a dir.

## En JSON

Amb [`--format json`](invocation.md#sortida-llegible-per-un-script), `inspect` escriu **un
objecte per fitxer** en lloc dels blocs anteriors; la narració es queda a la sortida d'error.
Les claus descriuen el que diu el text, i **cap no s'ha de tornar a extreure d'una frase**:

| Clau | Contingut |
| :-- | :------ |
| `format` | l'identificador del format, el que accepta `convert --to`: `srt`, `vtt`… |
| `encoding` | `{"charset", "origin", "byte_order_mark"}` — `origin` és `byte-order-mark`, `asked` o `detected` |
| `line_endings` | `{"kind", "mixed_from_line"}` — `lf`, `crlf` o `cr`; la línia on apareix un segon tipus, o `null` |
| `subtitles` | el nombre de subtítols |
| `span_ms` | `{"start", "end"}` — el primer començament i l'últim final, en mil·lisegons |
| `frame_rate` | per a un fitxer comptat en fotogrames: `{"rate", "origin"}` (`asked` o `assumed`); altrament `null` |
| `grid` | per als altres: la graella deduïda, o `null` per a un fitxer comptat en fotogrames |
| `anomalies` | una taula de `{"subtitle", "kind"}` — `end-before-start`, `overlapping-subtitles`, `out-of-order` |
| `translation` | **només amb `-t`**: `{"file", "method", "attached", "born", "untranslated", "out_of_order"}` — `method` és `position` o `number`, i els quatre recomptes són els de la frase anterior |
| `warnings` | amb què s'ha trobat la lectura, a tots els nivells |

`grid` porta `verdict` (`clean`, `partial` o `silent`), `enough_starts`, `rate` (una cadena,
o `null` quan no s'anomena res), `concentration_permille` (mil·lèsimes, `null` sense prou
començaments), `offset_ms`, `also_fits` (la freqüència de la qual aquesta és un divisor enter, o
`null`), `not_separated` (les freqüències que un abast massa curt no separa), `strays`
(els començaments fora de la graella) i `starts`.

<!-- example: printf '{25}{50}Un.\n{75}{100}Deux.\n' > f.sub; subedit-cli --format json inspect --frame-rate 25 f.sub 2>/dev/null -->
```console
$ printf '{25}{50}Un.\n{75}{100}Deux.\n' > f.sub; subedit-cli --format json inspect --frame-rate 25 f.sub 2>/dev/null
{"schema":1,"command":"inspect","file":"f.sub","ok":true,"format":"microdvd","encoding":{"charset":"UTF-8","origin":"detected","byte_order_mark":false},"line_endings":{"kind":"lf","mixed_from_line":null},"subtitles":2,"span_ms":{"start":1000,"end":4000},"frame_rate":{"rate":"25","origin":"asked"},"grid":null,"anomalies":[],"warnings":[]}
```

## Narració

A la sortida d'error, segons el nivell demanat:

| Nivell | Què s'hi afegeix |
| :----- | :-------------- |
| 1 | `<path>: N subtitles` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings` |
| 3 | `<path>: N bytes read`, `<path>: N diagnostic(s) while reading`, i després **cadascun d'ells** — vegeu [Invocació](invocation.md#diagnòstics-de-lectura) |

## Codis de sortida

Els de l'eina: `0` si s'han llegit tots els fitxers, `2` si no se n'ha llegit cap, `3` si
només alguns, `1` en un error d'ús.

## Errors

Els d'una lectura — del fitxer, i de la traducció quan es dona `-t` —, i
els de `-t`: vegeu més amunt. `inspect` no escriu res que pugui ser refusat.

| Què el provoca | Missatge |
| :------------------ | :------ |
| cap camí donat | `files is required`, seguit d'una indicació cap a `--help` |
| fitxer inexistent | `<path>: does not exist` |
| fitxer il·legible | `<path>: cannot be opened: permission denied` |
| bytes que no es descodifiquen | `<path>: cannot be decoded in the chosen encoding` |
| format no reconegut | `<path>: is in no format this tool knows` |
| res que sembli un subtítol | `<path>: holds nothing recognisable as a subtitle` |

Són els mateixos missatges per a les disset subordres: la recepta per obrir
s'escriu una sola vegada, al nucli.

## Un exemple que falla

Un fitxer buit no correspon a cap format. No s'indica com a buit:
es refusa, i s'anomena.

<!-- example: : > vide.srt; subedit-cli inspect vide.srt; echo "code=$?" -->
```console
$ : > vide.srt; subedit-cli inspect vide.srt; echo "code=$?"
vide.srt: is in no format this tool knows
code=2
```
