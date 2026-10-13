# `pair`

```
subedit-cli pair (--output FICHIER | --output-dir DOSSIER | --in-place)
                 -t FICHIER [--align-method position|number]
                 [--dry-run]
                 <principal>
```

Écrit **la traduction recalée sur les positions du principal**, et dit comment ses lignes ont trouvé leur
sous-titre. C'est `Open Translation…` de la fenêtre — [le même appariement](../subedit-gui/fichiers.md#ouvrir-une-traduction),
la même phrase — suivi de l'écriture du résultat, que la fenêtre fait par `Save Translation`.

**C'est la seule chose que l'appariement fait et qu'un fichier traité seul ne fait pas.** Les autres sous-commandes
qui reçoivent `-t` changent la traduction qu'elles lisent — [`case`](case.md), [`replace`](replace.md)… — ; `pair` ne
change rien au texte : il **pose** la traduction sur le principal et écrit ce que la pose a produit. Le principal est
lu pour recevoir les lignes, **jamais écrit**.

- **Ce qui s'écrit est la traduction**, à **son propre chemin et dans son propre format** : le format, l'encodage et
  les fins de ligne du fichier de traduction. Ses sous-titres sont **aux positions du principal** : une ligne qui
  tombait à côté retrouve les bornes du sous-titre qu'elle traduit.
- **`-t` est requis.** `pair` sans traduction n'a rien à poser : c'est une erreur d'usage, code `1`.
- **Un seul fichier principal**, et pas de `--recursive` : une paire, ce sont deux fichiers, et un arbre n'a aucun
  moyen de dire quelle traduction va avec quel principal. Pour un lot, une invocation par paire.
- **`--output` nomme le fichier de la traduction écrite, `--output-dir` y met son nom de base, `--in-place` la
  réécrit**, comme pour toute sous-commande qui reçoit `-t` — voir [Une traduction](invocation.md#une-traduction).
- **`--document` n'existe pas** : il n'y a qu'un document visé, la traduction.

<!-- exemple: subedit-cli pair --help -->
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

## Arguments et options

| Option | Requis | Valeurs | Défaut |
| :----- | :----- | :------ | :----- |
| `<principal>` | oui | **un seul** chemin | — |
| `-t`, `--translation-file` | **oui** | le fichier de traduction | — |
| `--align-method` | non | `position` ou `number` — voir [Une traduction](invocation.md#une-traduction) | `position` |
| `--output` / `--output-dir` / `--in-place` | **l'une des trois**, sauf avec `--dry-run` | voir [Invocation](invocation.md#la-destination) | — |
| `--dry-run` | non | un drapeau : calcule et dit, n'écrit rien — voir [Voir avant d'écrire](invocation.md#voir-avant-décrire) | désactivé |

<!-- exemple: printf '1\n00:00:01,000 --> 00:00:02,000\nHello.\n\n2\n00:00:03,000 --> 00:00:04,000\nBye.\n\n' > film.srt; printf '1\n00:00:01,300 --> 00:00:02,300\nBonjour.\n\n2\n00:00:03,300 --> 00:00:04,300\nAu revoir.\n\n' > film.fr.srt; subedit-cli pair film.srt -t film.fr.srt --output recale.fr.srt; cat recale.fr.srt -->
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

La traduction était en retard de 0,3 s : chaque ligne a trouvé le sous-titre qu'elle recouvre par son milieu, et le
fichier écrit porte les positions du principal.

## Ce que dit l'alignement

La phrase est **celle que la fenêtre dit** quand elle ouvre une traduction, chaque partie seulement quand elle n'est
pas nulle — voir [`inspect -t`](inspect.md#une-traduction---t) pour le tableau de ses parties. **Elle est écrite
dès le niveau 1**, et non au niveau 2 comme pour les autres sous-commandes : ici l'alignement n'est pas un
détail du traitement, c'est le résultat.

**Ce que l'alignement fait du fichier écrit** :

- **un sous-titre que personne n'a traduit s'écrit comme un bloc sans texte**, qui se lit sans anomalie ;
- **un sous-titre né d'une ligne** — une ligne que rien n'a atteint — est écrit avec **ses propres positions**, quelle
  que soit la méthode : le fichier écrit peut donc compter plus de sous-titres que le principal ;
- **ce que le document portait comme traduction est effacé** avant la pose, ce qui est sans objet en ligne de commande :
  le principal est lu seul.

**Les deux méthodes** donnent le même fichier quand les lignes tombent chacune sur son sous-titre, et ne le donnent
pas quand une ligne manque ou que la traduction est décalée — c'est pour cela que la méthode est un choix, et que la
position est le défaut. Les huit cas de `src/test/data/paires/` en font l'inventaire, avec le fichier attendu pour
chaque méthode.

## Sortie

**Sortie standard** — rien : le résultat est le fichier écrit.

| Niveau | Ce qui s'ajoute sur la sortie d'erreur |
| :----- | :------------------------------------- |
| 1 | `<traduction>: translation: N lines attached; … -> <destination>` — **le chemin est celui de la traduction**, qui est le fichier écrit ; avec `--dry-run`, `(dry run, nothing written)` à la place de `-> <destination>` |
| 2 | `<traduction>: SubRip, UTF-8, no BOM, LF line endings kept` — le format, l'encodage, la marque et les fins de ligne **de la traduction**, remis tels quels |
| 3 | `<traduction>: N bytes read, M written`, puis **chaque diagnostic de lecture** du principal et de la traduction — voir [Invocation](invocation.md#les-diagnostics-de-lecture) |

### En JSON

Avec [`--format json`](invocation.md#sortie-lisible-par-un-script), **un seul objet**, dont `file` est la traduction et
`destination` le fichier écrit. `counts.subtitles` compte **les sous-titres du fichier écrit**, ceux qu'une ligne a
fait naître compris. La clé `alignment` — `{"file", "method", "attached", "born", "untranslated", "out_of_order"}` —
porte les mêmes comptes que la phrase, comme la clé `translation` d'[`inspect`](inspect.md#en-json).

## Codes de retour

Ceux de l'outil : `0` si la traduction est écrite — ou calculée, avec `--dry-run` —, `2` si un des deux fichiers n'a pas
pu être lu ou si la destination n'a pas pu être écrite, `1` sur une erreur d'usage. **Une traduction mal alignée n'est
pas un échec** : elle est écrite, et la phrase dit ce qui n'a pas trouvé sa place.

## Erreurs

| Ce qui les déclenche | Message |
| :------------------- | :------ |
| pas de `-t` | `pair needs the translation file to lay over the main one: use -t` |
| `--document` | `The following argument was not expected: --document` — il n'existe pas pour `pair` |
| plus d'un fichier principal | `The following argument was not expected: <chemin>` |
| la traduction est le principal | `<traduction>: the file is already open as the main document`, code `2` |
| un des fichiers absent, illisible ou d'un format inconnu | les mêmes que partout, avec le chemin du fichier en cause |
