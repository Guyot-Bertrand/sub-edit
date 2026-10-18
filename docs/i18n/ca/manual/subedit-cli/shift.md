# `shift`

<!-- machine-translated -->
> Aquesta pàgina és una traducció automàtica i encara no l'ha revisada cap lector de la llengua.

```
subedit-cli shift (--by <time> | --to-grid)
                  (--output FILE | --output-dir DIR | --in-place)
                  [--dry-run]
                  [--recursive]
                  <file>...
```

Desplaça **totes** les posicions del fitxer la mateixa quantitat, en un sentit o
en l'altre. Cap subtítol no es queda a la pantalla més temps ni menys:
això és el que fa que un desplaçament sigui un desplaçament.

<!-- example: subedit-cli shift --help -->
```console
$ subedit-cli shift --help
Move every position of a file by a fixed amount
Usage: subedit-cli shift [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to shift

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --by TEXT                   Amount to move by: 2.999, -7.001, or 00:00:07.001
  --to-grid                   Move by the amount that puts the positions back on their frame grid
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
| `--by` | **una de les dues** | una durada amb signe, vegeu més avall | — |
| `--to-grid` | **una de les dues** | indicador: la quantitat es mesura, vegeu més avall | — |
| `--output` / `--output-dir` / `--in-place` | **una de les tres**, excepte amb `--dry-run` | vegeu [Invocació](invocation.md#la-destinació) | — |
| `--dry-run` | no | un indicador: calcula i diu, no escriu res — vegeu [Veure abans d'escriure](invocation.md#veure-abans-descriure) | desactivat |

El format del fitxer llegit **es conserva**: canviar el format és la feina de
[`convert`](convert.md).

## `--to-grid`: la quantitat mesurada en lloc de donada

Un fitxer les posicions del qual són en una graella de fotogrames **llevat d'una
constant** ha estat desplaçat, i aquesta constant es pot mesurar sobre les mateixes posicions.
`--to-grid` la mesura i l'aplica, sense que ningú l'hagi de conèixer.

La quantitat és **pròpia de cada fitxer**: dos fitxers desplaçats de la mateixa
graella en dues quantitats diferents tornen cadascun per la seva. S'escriu
a la narració, amb la freqüència de fotogrames trobada.

**El camí és el més curt.** Passat mig fotograma, la línia de la graella més propera
és la següent: la correcció avança en lloc de retrocedir gairebé
un fotograma sencer.

<!-- example: k=0; while [ $k -lt 12 ]; do s=$((1000 + k*47000 + (k%3)*40 + 7)); e=$((s+2000)); k=$((k+1)); printf '%d\n%02d:%02d:%02d,%03d --> %02d:%02d:%02d,%03d\nRéplique %d.\n\n' $k $((s/3600000)) $((s/60000%60)) $((s/1000%60)) $((s%1000)) $((e/3600000)) $((e/60000%60)) $((e/1000%60)) $((e%1000)) $k; done > decale.srt; subedit-cli shift --to-grid --output recale.srt decale.srt -->
```console
$ k=0; while [ $k -lt 12 ]; do s=$((1000 + k*47000 + (k%3)*40 + 7)); e=$((s+2000)); k=$((k+1)); printf '%d\n%02d:%02d:%02d,%03d --> %02d:%02d:%02d,%03d\nRéplique %d.\n\n' $k $((s/3600000)) $((s/60000%60)) $((s/1000%60)) $((s%1000)) $((e/3600000)) $((e/60000%60)) $((e/1000%60)) $((e%1000)) $k; done > decale.srt; subedit-cli shift --to-grid --output recale.srt decale.srt
decale.srt: 12 subtitles shifted by -0.007 s onto their 25 fps grid -> recale.srt
```

**Un fitxer que no és en cap graella coneguda es refusa, i se'n diu el motiu.** No hi
ha res a què tornar, i una fase mesurada sobre soroll mouria el fitxer
a l'atzar. Vegeu [`inspect`](inspect.md#què-indica-frame-rate-grid) per
saber què dona un fitxer abans de provar l'operació.

`--by` i `--to-grid` diuen tots dos quant cal desplaçar: donar-los junts
és un error d'ús, i no donar-ne cap també.

**`--to-grid` no és [`snap`](snap.md).** Desplaça tot el fitxer
la mateixa quantitat i conserva la sincronització relativa **exactament**; `snap`
mou cada posició independentment i absorbeix pel camí les que s'havien
corregit a mà. En un fitxer net tots dos donen el mateix
resultat; en un fitxer parcial, no.

## Escriure una durada

Dues formes, i només dues:

| Forma | Exemples |
| :---- | :------- |
| segons | `30`, `2.999`, `-7.001`, `+1.5` |
| marca de temps | `00:00:07.001`, `01:39:37.040`, `-00:00:07.001` |

Totes dues diuen el mateix: `--by 7.001` i `--by 00:00:07.001` produeixen
el mateix fitxer.

**El separador decimal és el punt**, perquè la línia d'ordres parla anglès.
Una coma es refusa en lloc d'interpretar-se: mig món escriu `7,001`
per dir set mil u, i llegir-ho com set segons seria una suposició —
equivocada per a aquesta meitat.

**La resolució és el mil·lisegon.** Una precisió més fina es refusa en lloc d'arrodonir-se
en silenci: un arrodoniment tàcit donaria un resultat que no correspon a res del que
s'havia demanat.

<!-- example: printf '1\n00:00:10,000 --> 00:00:12,000\nBonjour.\n\n' > a.srt; subedit-cli shift --by -7.001 --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:10,000 --> 00:00:12,000\nBonjour.\n\n' > a.srt; subedit-cli shift --by -7.001 --output b.srt a.srt; cat b.srt
a.srt: 1 subtitle shifted by -7.001 s -> b.srt
1
00:00:02,999 --> 00:00:04,999
Bonjour.
```

## Un desplaçament abans de l'origen es refusa

Retrocedir més enllà de zero dona el codi `2` i **no escriu res**. El missatge anomena
el subtítol que no ha pogut prendre el desplaçament, cosa que diu alhora
fins on es pot arribar.

El motiu no és un principi: una posició anterior a l'origen s'escriu
`-00:00:01,000`, una forma que cap dels nou formats no defineix i que cap lector no
accepta. El nucli sí que admet aquestes posicions — refusar-les faria d'una
operació d'edició un cas particular —, però un fitxer és una altra cosa que un
model en memòria.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nBonjour.\n\n' > a.srt; subedit-cli shift --by -7.001 --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nBonjour.\n\n' > a.srt; subedit-cli shift --by -7.001 --output b.srt a.srt; echo "code=$?"
a.srt: subtitle 1 would start before the origin, which no subtitle file can hold
code=2
```

## Sortida

**Sortida estàndard** — res: el resultat és el fitxer escrit. Amb `--dry-run`, tampoc res: no hi ha cap fitxer, i la línia de narració ho diu — vegeu [Veure abans d'escriure](invocation.md#veure-abans-descriure).

| Nivell | Què s'afegeix a la sortida d'error |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles shifted by 2.999 s -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — el format, la codificació, la marca i els finals de línia del fitxer llegit, restituïts tal com eren |
| 3 | `<path>: N bytes read, M written`, i després **cada diagnòstic de lectura** — vegeu [Invocació](invocation.md#diagnòstics-de-lectura) |

### En JSON

Amb [`--format json`](invocation.md#sortida-llegible-per-un-script), l'objecte de cada fitxer porta `counts.subtitles` (tots els subtítols del fitxer) i `counts.shifted_by_ms` (el desplaçament, en mil·lisegons enters, **amb signe**). La taula completa és la d'[Invocació](invocation.md#què-posa-cada-subordre-a-counts).

## Codis de sortida

Els de l'eina: `0` si s'han escrit tots els fitxers, `2` si no se n'ha escrit cap, `3` si
només alguns, `1` en un error d'ús.

## Errors

| Què el provoca | Missatge |
| :------------------ | :------ |
| coma decimal | `"…" is not a time: use a decimal point and not a comma, in every language` |
| més fi que el mil·lisegon | `"…" is not a time: positions are held to the millisecond, no finer` |
| text que no és una durada | `"…" is not a time: expected seconds like 2.999, or a timestamp like 00:00:07.001` |
| desplaçament abans de l'origen | `<path>: subtitle N would start before the origin, which no subtitle file can hold` |

**No s'escriu res sense una de les tres opcions de destinació**, i els seus
errors són comuns a les subordres que escriuen:
vegeu [Invocació](invocation.md#la-destinació).
