# `join-repeats`

<!-- machine-translated -->
> Aquesta pàgina és una traducció automàtica i encara no l'ha revisada cap lector de la llengua.

```
subedit-cli join-repeats (--output FILE | --output-dir DIR | --in-place)
                         [--dry-run]
                         [--recursive]
                         <file>...
```

Fa **un sol subtítol** dels subtítols que mostren el mateix l'un darrere l'altre: un text tallat en trossos
que se succeeixen, tal com el deixen algunes eines de segmentació. És `Join Repeated Subtitles` de la finestra —
[el mateix motor](../subedit-gui/lines.md#unir-els-subtítols-repetits).

**Una repetició** és un subtítol que té **el mateix text, la mateixa traducció i els mateixos paràmetres de format** (un
estil, una posició, paràmetres de cue) que un altre, **i que comença com a molt tard quan aquest acaba**: es
toquen o s'encavalquen. Una sèrie de repeticions esdevé **la que comença primer**, el final de la qual
es porta fins al final més tardà de la sèrie; les altres desapareixen.

- **L'ordre del fitxer no hi fa res, i no es canvia**: el subtítol que es conserva és el que comença primer,
  siguin on siguin els altres. Un fitxer en ordre continua en ordre.
- **No s'uneix res per error**: un altre text, una altra traducció, uns altres paràmetres, un interval entre dos
  trossos, un subtítol sense text o el final del qual arriba abans del començament — en tots aquests casos es mantenen separats.
- **Cal fer-ho abans de la resta**: ordenar i [tallar els encavalcaments](disjoint.md) llegeixen millor una sèrie neta.
  [`repair`](repair.md) encadena les tres, i [`convert`](convert.md#fer-disjuntes-les-franges) ho accepta com a opció (`--join-repeats`, `--repair`).

<!-- example: subedit-cli join-repeats --help -->
```console
$ subedit-cli join-repeats --help
Join the subtitles that repeat each other, one after the other, into one
Usage: subedit-cli join-repeats [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to join the repeats of

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
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
| `--output` / `--output-dir` / `--in-place` | **una de les tres**, excepte amb `--dry-run` | vegeu [Invocació](invocation.md#la-destinació) | — |
| `--dry-run` | no | un indicador: calcula i diu, no escriu res — vegeu [Veure abans d'escriure](invocation.md#veure-abans-descriure) | desactivat |

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nAlpha\n\n2\n00:00:02,000 --> 00:00:04,000\nAlpha\n\n3\n00:00:04,000 --> 00:00:06,000\nAlpha\n\n4\n00:00:08,000 --> 00:00:09,000\nBeta\n\n' > morceaux.srt; subedit-cli join-repeats --output joint.srt morceaux.srt; cat joint.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nAlpha\n\n2\n00:00:02,000 --> 00:00:04,000\nAlpha\n\n3\n00:00:04,000 --> 00:00:06,000\nAlpha\n\n4\n00:00:08,000 --> 00:00:09,000\nBeta\n\n' > morceaux.srt; subedit-cli join-repeats --output joint.srt morceaux.srt; cat joint.srt
morceaux.srt: 2 repeated subtitles joined -> joint.srt
1
00:00:01,000 --> 00:00:06,000
Alpha

2
00:00:08,000 --> 00:00:09,000
Beta
```

Els tres trossos d'`Alpha`, que se succeeixen d'1 s a 6 s, ara només en fan un. `Beta`, separat per un interval,
s'ha mantingut. Una segona passada no canvia res i ho diu: `no repeated subtitles`.

## Sortida

**Sortida estàndard** — res: el resultat és el fitxer escrit.

| Nivell | Què s'afegeix a la sortida d'error |
| :----- | :------------------------------------- |
| 1 | `<path>: N repeated subtitles joined -> <destination>`, o `<path>: no repeated subtitles -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` |
| 3 | `<path>: N bytes read, M written`, i després **cada diagnòstic de lectura** |

### En JSON

Amb [`--format json`](invocation.md#sortida-llegible-per-un-script), els `counts` de cada fitxer: `subtitles` (els del fitxer
escrit), `joined` (els subtítols eliminats perquè un altre els cobreix) i `chains` (les sèries unides).

## Codis de sortida

Els de l'eina: `0` si s'han tractat tots els fitxers — un fitxer sense repeticions ho és —, `2` si no se n'ha tractat cap, `3` si
només alguns, `1` en un error d'ús.

## Errors

Cap de propi de `join-repeats`: un fitxer il·legible falla com per a qualsevol subordre, i els errors de
la destinació són comuns a les subordres que escriuen: vegeu [Invocació](invocation.md#la-destinació).
