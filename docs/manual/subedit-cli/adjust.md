# `adjust`

```
subedit-cli adjust [--speed CPS|off] [--shorten] [--no-lengthen]
                   [--minimum TIME|off] [--maximum TIME] [--gap TIME|off]
                   [--range N-M|N-]
                   (--output FICHIER | --output-dir DOSSIER | --in-place)
                   [--dry-run]
                   [--recursive]
                   <fichier>...
```

Ajuste la **durée** de chaque sous-titre à une vitesse de lecture et à des bornes : un
sous-titre trop court pour être lu est allongé, un sous-titre qui laisse l'écran
trop longtemps peut être raccourci, et deux sous-titres ne se recouvrent pas. **Seule la
fin bouge** : déplacer le début déplacerait le sous-titre, ce qui est le travail de
[`shift`](shift.md). C'est l'opération `Adjust Durations…` de la
[fenêtre](../subedit-gui/operations.md#adjust-durations), avec les mêmes défauts.

## Les quatre contraintes, et leur ordre

Elles s'appliquent **dans cet ordre**, chacune posant la fin, la dernière gagnant :

1. **la vitesse de lecture** — la durée dont le texte a besoin, à tant de caractères par
   seconde (les balises ne comptent pas, les retours à la ligne oui) ;
2. **le minimum** — une durée en deçà est portée au minimum ;
3. **le maximum** — une durée au-delà est ramenée au maximum ;
4. **l'écart** — le temps laissé avant le sous-titre suivant. Il vient en dernier et
   **il gagne** : deux sous-titres affichés ensemble sont une faute que le spectateur voit,
   un sous-titre un peu court non.

Le dernier sous-titre du fichier n'a pas de suivant, donc pas d'écart à tenir. Le suivant
d'un sous-titre est celui du fichier, **choisi ou non** par `--range`.

| Contrainte | Option | Défaut | Éteindre |
| :--------- | :----- | :----- | :------- |
| vitesse | `--speed CPS` | **15** caractères par seconde | `--speed off` |
| sens de la vitesse | `--shorten`, `--no-lengthen` | elle **allonge** seulement | — |
| minimum | `--minimum TIME` | **1,5 s** | `--minimum off` |
| maximum | `--maximum TIME` | **aucun** | ne pas le donner |
| écart | `--gap TIME` | **0** | `--gap off` |

Ce sont les réglages de Gaupol et de la première ouverture de la fenêtre. **`off` éteint une
contrainte : elle n'est plus vérifiée.** Ce n'est pas la mettre à zéro — un minimum de `0`
est un minimum de zéro, un écart de `0` interdit tout recouvrement.

<!-- exemple: subedit-cli adjust --help -->
```console
$ subedit-cli adjust --help
Bring the duration of every subtitle within a reading speed and limits
Usage: subedit-cli adjust [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to adjust

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --speed CPS|off             Reading speed in characters per second, or off (default 15)
  --shorten                   Let the reading speed bring an end earlier
  --no-lengthen               Do not let the reading speed move an end later
  --minimum TIME|off          Shortest duration, or off (default 1.5)
  --maximum TIME              Longest duration (default: none)
  --gap TIME|off              Least time left before the next subtitle, or off (default 0)
  --range N-M|N-              Adjust only subtitles N to M, or N to the end
  --output TEXT               File to write, for a single input
  --output-dir TEXT           Directory to write into
  --in-place                  Write back over the inputs
  --dry-run                   Work out and say what would be written, and write nothing
```

## Arguments et options

| Option | Requis | Valeurs | Défaut |
| :----- | :----- | :------ | :----- |
| `<fichier>...` | oui | un ou plusieurs chemins | — |
| `--recursive`, `-r` | non | un drapeau | désactivé — voir [Traiter un arbre](lots.md) |
| `--speed` | non | un nombre de caractères par seconde, **supérieur à zéro** (`15`, `12.5`), ou `off` | `15` |
| `--shorten` | non | un drapeau : la vitesse peut aussi **avancer** une fin | désactivé |
| `--no-lengthen` | non | un drapeau : la vitesse ne **recule** plus une fin | désactivé |
| `--minimum` | non | une durée de zéro ou plus, ou `off` | `1.5` |
| `--maximum` | non | une durée **supérieure à zéro** | aucun |
| `--gap` | non | une durée de zéro ou plus, ou `off` | `0` |
| `--range` | non | `N-M`, ou `N-` jusqu'à la fin — voir ci-dessous | tout le fichier |
| `--output` / `--output-dir` / `--in-place` | **l'une des trois**, sauf avec `--dry-run` | voir [Invocation](invocation.md#la-destination) | — |
| `--dry-run` | non | un drapeau : calcule et dit, n'écrit rien — voir [Voir avant d'écrire](invocation.md#voir-avant-décrire) | désactivé |

Une durée s'écrit comme pour [`shift`](shift.md#écrire-une-durée) : `2.5`, `00:00:02.500`.

**Sans aucune contrainte active, `adjust` est refusé** (code `1`, rien d'écrit) : il n'y aurait
rien à quoi ajuster, et une commande qui ne fait rien en disant qu'elle a réussi est la pire
réponse. C'est le cas de `--speed off --minimum off --gap off` sans maximum, et de
`--no-lengthen` seul avec le minimum et l'écart éteints : une vitesse qui ne déplace aucune
fin ne contraint rien.

`--shorten` et `--no-lengthen` disent dans quel sens la vitesse déplace une fin ; ils n'ont de
sens qu'avec une vitesse, et **`--speed off` les refuse**.

### `--range`

`--range 3-7` limite l'ajustement aux sous-titres 3 à 7, **comptés depuis 1 et bornes
comprises** ; `--range 3-` va jusqu'au dernier. Un seul sous-titre s'écrit `4-4`.

- Une plage **qui finit avant de commencer**, un `0`, un signe ou un nombre seul sont
  refusés comme erreur d'usage (code `1`).
- **Elle vaut pour chaque fichier du lot**, ce qui n'a de sens que si les fichiers se
  ressemblent. Elle est jugée **contre chaque fichier** : une plage qui dépasse celui-ci est
  refusée **avant qu'on y touche**, en nommant la borne — `range 3-9 ends after the last
  subtitle: the file has 4` —, ce fichier échoue (code `2` ou `3`, identifiant
  `range-out-of-bounds`) et les autres continuent.
- Les comptes du rapport ne portent que sur les sous-titres choisis.

## Ce qui est sacrifié

Deux contraintes peuvent se contredire sur un même sous-titre : un minimum de 1,5 s ne tient
pas dans les 0,8 s que laisse le sous-titre suivant. **L'ordre décide, et le compte rendu dit
ce qui a cédé** — c'est ce que Gaupol ne fait pas : il viole en silence.

Chaque contrainte n'est nommée que si **un sous-titre au moins ne l'a pas obtenue**, une fois
les autres appliquées. Le maximum n'a pas de compte : il est appliqué avant l'écart, qui ne peut
que raccourcir, donc il tient toujours. Un minimum au-dessus du maximum cède au maximum et est
compté comme un minimum.

<!-- exemple: printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli adjust --output ajuste.srt courts.srt; grep -e '-->' ajuste.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli adjust --output ajuste.srt courts.srt; grep -e '-->' ajuste.srt
courts.srt: adjusted the durations of 3 subtitles; could not satisfy the minimum duration in 1 subtitle -> ajuste.srt
00:00:00,000 --> 00:00:02,000
00:00:10,000 --> 00:00:14,000
00:00:14,200 --> 00:00:15,000
00:00:15,000 --> 00:00:16,500
```

Le troisième sous-titre voudrait finir à 15,7 s pour durer 1,5 s, mais le quatrième commence à
15,0 s : **l'écart gagne**, il finit à 15,0 s, et le compte rendu nomme le minimum.

Éteindre l'écart laisse le minimum l'emporter — et les deux sous-titres se recouvrent :

<!-- exemple: printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli adjust --gap off --output libre.srt courts.srt; grep -e '-->' libre.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli adjust --gap off --output libre.srt courts.srt; grep -e '-->' libre.srt
courts.srt: adjusted the durations of 3 subtitles -> libre.srt
00:00:00,000 --> 00:00:02,000
00:00:10,000 --> 00:00:14,000
00:00:14,200 --> 00:00:15,700
00:00:15,000 --> 00:00:16,500
```

Et rien n'est sacrifié ni ajusté quand le fichier tient déjà dans ses limites :

<!-- exemple: printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli adjust --speed off --minimum off --output intact.srt courts.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli adjust --speed off --minimum off --output intact.srt courts.srt
courts.srt: no duration to adjust -> intact.srt
```

Le fichier est écrit tel quel : **une destination donnée est une destination écrite**.

## Sortie

**Sortie standard** — rien : le résultat est le fichier écrit.

| Niveau | Ce qui s'ajoute sur la sortie d'erreur |
| :----- | :------------------------------------- |
| 1 | `<chemin>: adjusted the durations of N subtitles; could not satisfy …  -> <destination>`, ou `<chemin>: no duration to adjust -> <destination>` |
| 2 | `<chemin>: SubRip, UTF-8, no BOM, LF line endings kept` — le format, l'encodage, la marque et les fins de ligne du fichier lu, remis tels quels |
| 3 | `<chemin>: N bytes read, M written`, puis **chaque diagnostic de lecture** — voir [Invocation](invocation.md#les-diagnostics-de-lecture) |

La phrase est celle de la fenêtre : `N` est le nombre de sous-titres dont la fin a bougé, et la
suite, absente quand rien n'a cédé, énumère ce qui n'a pas été tenu — `the reading speed`,
`the minimum duration`, `the gap` — avec le nombre de sous-titres concernés.

### En JSON

Avec [`--format json`](invocation.md#sortie-lisible-par-un-script), l'objet de chaque fichier
porte, en plus de l'enveloppe :

| Clé | Contenu |
| :-- | :------ |
| `counts.subtitles` | les sous-titres **visés** : tout le fichier, ou la plage |
| `counts.adjusted` | ceux dont la fin a bougé |
| `counts.sacrificed.speed`, `.minimum`, `.gap` | les sous-titres qui ne tiennent pas encore cette contrainte, une fois les autres appliquées, **y compris ceux dont la fin n'a pas bougé** |
| `constraints.speed` | `{"cps", "lengthen", "shorten"}` — les caractères par seconde **en chaîne** (`"15"`, `"12.5"`) — ou `null` si éteinte |
| `constraints.minimum_ms`, `.maximum_ms`, `.gap_ms` | les durées employées, en **millisecondes entières**, ou `null` si éteintes |

`constraints` dit **les réglages réellement employés**, défauts compris : deux calculs
qui portent sur les mêmes sous-titres se comparent alors sans lire la ligne de commande.

<!-- exemple: printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --format json adjust --dry-run --maximum 1.8 courts.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --format json adjust --dry-run --maximum 1.8 courts.srt
courts.srt: adjusted the durations of 4 subtitles; could not satisfy the reading speed in 1 subtitle, the minimum duration in 1 subtitle (dry run, nothing written)
{"schema":1,"command":"adjust","file":"courts.srt","ok":true,"dry_run":true,"destination":null,"counts":{"subtitles":4,"adjusted":4,"sacrificed":{"speed":1,"minimum":1,"gap":0}},"constraints":{"speed":{"cps":"15","lengthen":true,"shorten":false},"minimum_ms":1500,"maximum_ms":1800,"gap_ms":0},"warnings":[]}
```

## Codes de retour

Ceux de l'outil : `0` si tous les fichiers ont été traités, `2` si aucun, `3` si certains
seulement, `1` sur une erreur d'usage. Un fichier qui tient déjà dans ses limites est traité.

## Erreurs

| Ce qui la déclenche | Message |
| :------------------ | :------ |
| aucune contrainte active | `adjust has nothing to adjust to: every constraint is off, or the reading speed is told to move no end` |
| vitesse qui n'en est pas une | `--speed: "…" is not a reading speed: expected a number of characters per second above zero, like 15 or 12.5, or off` |
| `--shorten`/`--no-lengthen` avec `--speed off` | `--shorten and --no-lengthen say which way the reading speed may move an end, and --speed off switches the reading speed off` |
| durée qui n'en est pas une | `--minimum: "…" is not a time: …` (idem `--maximum`, `--gap`) |
| durée négative | `--minimum: "-1" is not a duration of zero or more` (idem `--gap`) |
| maximum nul ou négatif | `--maximum: "0" is not a duration above zero` |
| plage mal écrite | `--range: "…" is not a range: write it N-M, or N- to go to the end, counted from 1 and inclusive` |
| plage qui finit avant de commencer | `--range: "5-3" is not a range: it ends before it starts` |
| plage hors du fichier | `<chemin>: range N-M starts after the last subtitle: the file has K`, ou `… ends after …` |

Les erreurs de la destination sont communes aux sous-commandes qui écrivent :
voir [Invocation](invocation.md#la-destination).
