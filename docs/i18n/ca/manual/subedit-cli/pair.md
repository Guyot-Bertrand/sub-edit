# `pair`

<!-- machine-translated -->
> Aquesta pàgina és una traducció automàtica i encara no l'ha revisada cap lector de la llengua.

```
subedit-cli pair (--output FILE | --output-dir DIR | --in-place)
                 -t FILE [--align-method position|number]
                 [--dry-run]
                 <main>
```

Escriu **la traducció resincronitzada sobre les posicions del fitxer principal**, i diu com les seves línies han trobat el seu
subtítol. És `Open Translation…` de la finestra — [el mateix aparellament](../subedit-gui/files.md#obrir-una-traducció),
la mateixa frase — seguit de l'escriptura del resultat, que la finestra fa amb `Save Translation`.

**És l'única cosa que fa l'aparellament que no fa un fitxer tractat sol.** Les altres subordres
que prenen `-t` canvien la traducció que llegeixen — [`case`](case.md), [`replace`](replace.md)… —; `pair` no canvia
res del text: **posa** la traducció sobre el fitxer principal i escriu el que n'ha resultat. El fitxer principal es
llegeix per rebre les línies, **no s'escriu mai**.

- **El que s'escriu és la traducció**, al **seu propi camí i en el seu propi format**: el format, la codificació i
  els finals de línia del fitxer de traducció. Els seus subtítols són **a les posicions del fitxer principal**: una línia que
  havia caigut fora de lloc recupera els límits del subtítol que tradueix.
- **`-t` és obligatori.** `pair` sense traducció no té res a posar: és un error d'ús, codi `1`.
- **Un sol fitxer principal**, i cap `--recursive`: un parell són dos fitxers, i un arbre no té cap
  manera de dir quina traducció va amb quin fitxer principal. Per a un lot, una invocació per parell.
- **`--output` anomena el fitxer de la traducció escrita, `--output-dir` hi posa el seu nom de base, `--in-place`
  la reescriu**, com per a qualsevol subordre que pren `-t` — vegeu [Una traducció](invocation.md#una-traducció).
- **`--document` no existeix**: només hi ha un document apuntat, la traducció.

<!-- example: subedit-cli pair --help -->
```console
$ subedit-cli pair --help
Write a translation at the positions of its main file, and say how it lined up
Usage: subedit-cli pair [OPTIONS] main

Positionals:
  main TEXT REQUIRED          The main subtitle file

Options:
  -h,--help                   Print this help message and exit
  -t,--translation-file FILE  Translation file to lay over the subtitle file, for a single input
  --align-method position|number
                              How the lines of the translation find their subtitles
  --output TEXT               File to write, for a single input
  --output-dir TEXT           Directory to write into
  --in-place                  Write back over the inputs
  --dry-run                   Work out and say what would be written, and write nothing
```

## Arguments i opcions

| Opció | Obligatori | Valors | Per defecte |
| :----- | :----- | :------ | :----- |
| `<main>` | sí | **un sol** camí | — |
| `-t`, `--translation-file` | **sí** | el fitxer de traducció | — |
| `--align-method` | no | `position` o `number` — vegeu [Una traducció](invocation.md#una-traducció) | `position` |
| `--output` / `--output-dir` / `--in-place` | **una de les tres**, excepte amb `--dry-run` | vegeu [Invocació](invocation.md#la-destinació) | — |
| `--dry-run` | no | un indicador: calcula i diu, no escriu res — vegeu [Veure abans d'escriure](invocation.md#veure-abans-descriure) | desactivat |

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nHello.\n\n2\n00:00:03,000 --> 00:00:04,000\nBye.\n\n' > film.srt; printf '1\n00:00:01,300 --> 00:00:02,300\nBonjour.\n\n2\n00:00:03,300 --> 00:00:04,300\nAu revoir.\n\n' > film.fr.srt; subedit-cli pair film.srt -t film.fr.srt --output recale.fr.srt; cat recale.fr.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nHello.\n\n2\n00:00:03,000 --> 00:00:04,000\nBye.\n\n' > film.srt; printf '1\n00:00:01,300 --> 00:00:02,300\nBonjour.\n\n2\n00:00:03,300 --> 00:00:04,300\nAu revoir.\n\n' > film.fr.srt; subedit-cli pair film.srt -t film.fr.srt --output recale.fr.srt; cat recale.fr.srt
film.fr.srt: translation: 2 lines attached -> recale.fr.srt
1
00:00:01,000 --> 00:00:02,000
Bonjour.

2
00:00:03,000 --> 00:00:04,000
Au revoir.
```

La traducció anava 0,3 s endarrerida: cada línia ha trobat el subtítol amb què s'encavalca el seu centre, i el
fitxer escrit porta les posicions del fitxer principal.

## Què diu l'alineació

La frase és **la que diu la finestra** quan obre una traducció, cada part només quan no és
zero — vegeu [`inspect -t`](inspect.md#una-traducció--t) per a la taula de les seves parts. **S'escriu
a partir del nivell 1**, i no al nivell 2 com per a les altres subordres: aquí l'alineació no és un
detall del tractament, és el resultat.

**Què fa l'alineació al fitxer escrit**:

- **un subtítol que ningú no ha traduït s'escriu com un bloc sense text**, que es llegeix sense cap anomalia;
- **un subtítol nascut d'una línia** — una línia que res no ha acollit — s'escriu amb **les seves pròpies posicions**, sigui quin sigui
  el mètode: el fitxer escrit pot comptar, doncs, més subtítols que el fitxer principal;
- **el que el document portava com a traducció s'esborra** abans de posar-hi la nova, cosa que no s'aplica a la línia d'ordres:
  el fitxer principal es llegeix sol.

**Els dos mètodes** donen el mateix fitxer quan cada línia cau sobre el seu subtítol, i no el donen
quan falta una línia o la traducció està desplaçada — per això el mètode és una tria, i per això
la posició és el valor per defecte. Els vuit casos de `src/test/data/paires/` els enumeren, amb el fitxer esperat per a
cada mètode.

## Sortida

**Sortida estàndard** — res: el resultat és el fitxer escrit.

| Nivell | Què s'afegeix a la sortida d'error |
| :----- | :------------------------------------- |
| 1 | `<translation>: translation: N lines attached; … -> <destination>` — **el camí és el de la traducció**, que és el fitxer escrit; amb `--dry-run`, `(dry run, nothing written)` en lloc de `-> <destination>` |
| 2 | `<translation>: SubRip, UTF-8, no BOM, LF line endings kept` — el format, la codificació, la marca i els finals de línia **de la traducció**, restituïts tal com eren |
| 3 | `<translation>: N bytes read, M written`, i després **cada diagnòstic de lectura** del fitxer principal i de la traducció — vegeu [Invocació](invocation.md#diagnòstics-de-lectura) |

### En JSON

Amb [`--format json`](invocation.md#sortida-llegible-per-un-script), **un sol objecte**, el `file` del qual és la traducció i
`destination` el fitxer escrit. `counts.subtitles` compta **els subtítols del fitxer escrit**, inclosos els que una línia
ha fet néixer. La clau `alignment` — `{"file", "method", "attached", "born", "untranslated", "out_of_order"}` —
porta els mateixos recomptes que la frase, com la clau `translation` d'[`inspect`](inspect.md#en-json).

## Codis de sortida

Els de l'eina: `0` si la traducció s'ha escrit — o calculat, amb `--dry-run` —, `2` si un dels dos fitxers no s'ha pogut
llegir o si la destinació no s'ha pogut escriure, `1` en un error d'ús. **Una traducció mal alineada no és
un error**: s'escriu, i la frase diu què no ha trobat el seu lloc.

## Errors

| Què els provoca | Missatge |
| :------------------- | :------ |
| cap `-t` | `pair needs the translation file to lay over the main one: use -t` |
| `--document` | `The following argument was not expected: --document` — no existeix per a `pair` |
| més d'un fitxer principal | `The following argument was not expected: <path>` |
| la traducció és el fitxer principal | `<translation>: the file is already open as the main document`, codi `2` |
| un dels fitxers que falta, il·legible o d'un format desconegut | el mateix que arreu, amb el camí del fitxer que falla |
