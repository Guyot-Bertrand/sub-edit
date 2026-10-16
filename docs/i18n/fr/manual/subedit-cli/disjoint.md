# `disjoint`

```
subedit-cli disjoint (--output FILE | --output-dir DIR | --in-place)
                     [--minimum TIME] [--max-lines N] [--dashes] [--sort]
                     [--dry-run] [--recursive]
                     <file>...
```

Découpe les sous-titres qui se chevauchent pour que **plus aucune plage `[start, end]` n'en chevauche
une autre**. C'est `Make Ranges Disjoint…` de la fenêtre — [le même moteur](../subedit-gui/lines.md#rendre-les-plages-disjointes).

**Rien n'est raccourci ni prolongé** : à chaque instant, le fichier montre les mêmes textes qu'avant. Ce qui
change, c'est la façon dont la ligne de temps est découpée :

- elle est coupée à **chaque début et chaque fin** des sous-titres d'un groupe qui se chevauchent ;
- un intervalle couvert par **un seul** sous-titre reste ce sous-titre ;
- un intervalle couvert par **plusieurs** devient **un** sous-titre dont les lignes sont les leurs, **dans
  l'ordre de leurs débuts** — le plus tôt en ligne 1, le suivant en ligne 2 ;
- un sous-titre qui ne chevauche rien n'est pas touché, et l'ordre du reste est conservé.

<!-- example: subedit-cli disjoint --help -->
```console
$ subedit-cli disjoint --help
Cut overlapping subtitles so that no range overlaps another, merging what overlaps
Usage: subedit-cli disjoint [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to make disjoint

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --minimum TIME              Shortest interval kept; a shorter one is taken over by a neighbour (default 0.08)
  --max-lines N               Most lines of a merged subtitle; more are shown in windows, 0 for no limit (default 2)
  --dashes                    Put a dash before the text of each merged subtitle
  --sort                      Put the subtitles in order of their start first; without it, a file out of order is refused
  --output TEXT               File to write, for a single input
  --output-dir TEXT           Directory to write into
  --in-place                  Write back over the inputs
  --dry-run                   Work out and say what would be written, and write nothing
```

## Arguments et options

| Option | Requis | Valeurs | Défaut |
| :----- | :----- | :------ | :----- |
| `<file>...` | oui | un ou plusieurs chemins | — |
| `--minimum` | non | une durée : `0.08`, `1.5`, `00:00:00,080`. Zéro garde tout intervalle | `0.08` (environ deux images) |
| `--max-lines` | non | un entier : le plus de lignes d'un sous-titre fusionné ; `0` lève la limite | `2` |
| `--dashes` | non | un drapeau : un tiret devant le texte que chaque sous-titre apporte à un sous-titre fusionné | désactivé |
| `--sort` | non | un drapeau : met les sous-titres dans l'ordre de leur début **avant** — voir [Il faut un fichier dans l'ordre](#il-faut-un-fichier-dans-lordre) | désactivé |
| `--recursive`, `-r` | non | un drapeau | désactivé — voir [Traiter un arbre](lots.md) |
| `--output` / `--output-dir` / `--in-place` | **l'une des trois**, sauf avec `--dry-run` | voir [Invocation](invocation.md#la-destination) | — |
| `--dry-run` | non | un drapeau : calcule et dit, n'écrit rien — voir [Voir avant d'écrire](invocation.md#voir-avant-décrire) | désactivé |

<!-- example: printf '1\n00:00:00,000 --> 00:00:05,000\nA\n\n2\n00:00:03,000 --> 00:00:08,000\nB\n\n' > chevauche.srt; subedit-cli disjoint --output disjoint.srt chevauche.srt; cat disjoint.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:05,000\nA\n\n2\n00:00:03,000 --> 00:00:08,000\nB\n\n' > chevauche.srt; subedit-cli disjoint --output disjoint.srt chevauche.srt; cat disjoint.srt
chevauche.srt: 2 subtitles split, 1 merged -> disjoint.srt
1
00:00:00,000 --> 00:00:03,000
A

2
00:00:03,000 --> 00:00:05,000
A
B

3
00:00:05,000 --> 00:00:08,000
B
```

`A [0, 5]` et `B [3, 8]` donnent trois sous-titres disjoints : `A` seul, `A` et `B` ensemble, `B` seul. Aucun
instant n'est perdu ni ajouté.

## Ce que l'opération décide

**Une ligne n'est jamais dupliquée.** Deux sous-titres de même texte donnent une seule ligne là où ils se
recouvrent, et deux morceaux voisins qui montrent la même chose ne font qu'un sous-titre. Le texte d'un
sous-titre coupé en plusieurs morceaux est, sinon, **répété dans chacun** : c'est ce qui est à l'écran à cet
instant.

**Un intervalle très court est repris par un voisin.** Sous `--minimum`, un intervalle disparaît : le voisin
**étend sa plage pour le couvrir**, de sorte qu'aucun instant ne reste vide et que rien ne bouge de plus que ce
seuil. Le voisin retenu est celui dont les lignes **contiennent déjà** celles de l'intervalle, sinon le plus
long. Le texte de l'intervalle n'est perdu que si aucun voisin ne le porte, et alors pour moins que le seuil.
**Un sous-titre entier plus court que le seuil n'est jamais repris** : il est laissé, et compté.

**Plus de lignes que la limite : des fenêtres.** Un sous-titre fusionné qui aurait plus de `--max-lines` lignes
est montré **en fenêtres successives** de cette taille, en parts égales de la durée. Trois lignes sur trois
secondes :

<!-- example: printf '1\n00:00:00,000 --> 00:00:03,000\nA\n\n2\n00:00:00,000 --> 00:00:03,000\nB\n\n3\n00:00:00,000 --> 00:00:03,000\nC\n\n' > trois.srt; subedit-cli disjoint --output fenetres.srt trois.srt; cat fenetres.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:03,000\nA\n\n2\n00:00:00,000 --> 00:00:03,000\nB\n\n3\n00:00:00,000 --> 00:00:03,000\nC\n\n' > trois.srt; subedit-cli disjoint --output fenetres.srt trois.srt; cat fenetres.srt
trois.srt: 0 subtitles split, 1 merged; 1 shown in windows for holding too many lines -> fenetres.srt
1
00:00:00,000 --> 00:00:01,500
A
B

2
00:00:01,500 --> 00:00:03,000
B
C
```

Quatre lignes donnent trois fenêtres, et ainsi de suite : la fenêtre glisse d'une ligne à la fois. Rien n'est
tronqué ni recoupé.

**La traduction suit les mêmes coupes**, puisqu'un sous-titre porte les deux textes. **Ce que le format porte
en plus** — un style, une position — est celui du premier sous-titre qui couvre ; la perte est comptée quand
les sous-titres fusionnés diffèrent.

**Un sous-titre dont la fin précède le début** n'est pas deviné : il est laissé où il est, et compté.

## Il faut un fichier dans l'ordre

Les groupes sont des suites de voisins : hors d'ordre, les voisins ne sont pas ceux qui se chevauchent. Un fichier
dont les débuts ne sont pas dans l'ordre est donc **refusé**, et le message le dit :

<!-- example: printf '1\n00:00:03,000 --> 00:00:08,000\nB\n\n2\n00:00:00,000 --> 00:00:05,000\nA\n\n' > desordre.srt; subedit-cli disjoint --output x.srt desordre.srt; echo "code $?" -->
```console
$ printf '1\n00:00:03,000 --> 00:00:08,000\nB\n\n2\n00:00:00,000 --> 00:00:05,000\nA\n\n' > desordre.srt; subedit-cli disjoint --output x.srt desordre.srt; echo "code $?"
desordre.srt: The subtitles are not in the order of their start: sort them first. Use --sort to do that here.
code 2
```

`--sort` fait le tri d'abord — le même que [`sort`](sort.md) —, et le dit. Le noyau ne trie jamais de lui-même.

Un second passage ne change rien : une fois les plages disjointes, il n'y a plus rien à couper, et la
phrase dit `already disjoint`.

## Sortie

**Sortie standard** — rien : le résultat est le fichier écrit.

| Niveau | Ce qui s'ajoute sur la sortie d'erreur |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles split, M merged -> <destination>`, suivi après un `;` de ce qui n'a pas pu être fait en entier : `K shown in windows for holding too many lines`, `K very short taken over by a neighbour`, `K subtitles shorter than the limit left whole`, `K merged with styles or positions that differ, the first kept`, `K subtitles ending before the start left alone` — ou `<path>: already disjoint -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` |
| 3 | `<path>: N bytes read, M written`, puis **chaque diagnostic de lecture** |

`N subtitles split` compte les sous-titres d'origine coupés en plusieurs morceaux, `M merged` les morceaux qui
montrent plusieurs sous-titres ensemble.

### En JSON

Avec [`--format json`](invocation.md#sortie-lisible-par-un-script), les `counts` de chaque fichier :

| Compte | Ce qu'il dit |
| :----- | :----------- |
| `subtitles` | les sous-titres du fichier écrit |
| `split`, `merged` | comme dans la phrase |
| `windowed` | les morceaux montrés en fenêtres |
| `absorbed` | les intervalles repris par un voisin |
| `too_short` | les sous-titres plus courts que le seuil, laissés entiers |
| `ending_before_start` | les sous-titres dont la fin précède le début, laissés |
| `extras_dropped` | les morceaux fusionnés dont le style ou la position diffèrent |
| `moved` | avec `--sort` seulement : les places qui ont changé |

## Codes de retour

Ceux de l'outil : `0` si tous les fichiers ont été traités — un fichier sans chevauchement l'est —, `2` si
aucun, `3` si certains seulement, `1` sur une erreur d'usage.

## Erreurs

| Ce qui les déclenche | Message |
| :------------------- | :------ |
| un fichier hors d'ordre sans `--sort` | `<path>: The subtitles are not in the order of their start: sort them first. Use --sort to do that here.` |
| `--minimum` n'est pas une durée, ou est négatif | `--minimum: …` |
| `--max-lines` n'est pas un nombre entier | `--max-lines: "…" is not a number of lines: expected a whole number, 0 for no limit` |

Un fichier illisible échoue comme pour toute sous-commande ; les erreurs de la destination sont communes aux
sous-commandes qui écrivent : voir [Invocation](invocation.md#la-destination).

## Dans les autres commandes

[`convert`](convert.md#rendre-les-plages-disjointes), [`append`](append.md#rendre-les-plages-disjointes) et
[`split-file`](split-file.md#rendre-les-plages-disjointes) acceptent `--disjoint`, qui fait la même chose avec
les réglages par défaut, avant d'écrire.

Sur un fichier mal segmenté — un texte coupé en morceaux qui se suivent, donné hors d'ordre —, le tri et le découpage
seuls donnent des lignes qui basculent d'un morceau à l'autre : [`join-repeats`](join-repeats.md) d'abord, ou
[`repair`](repair.md), qui enchaîne les trois.
