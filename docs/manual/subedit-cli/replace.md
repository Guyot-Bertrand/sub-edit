# `replace`

```
subedit-cli replace [--regex] [--case-sensitive] [--range N-M|N-]
                    (--output FICHIER | --output-dir DOSSIER | --in-place)
                    [--dry-run]
                    [--recursive]
                    <motif> <remplacement> <fichier>...
```

Remplace un texte dans les sous-titres **sans casser une balise**. C'est l'opération
`Replace All` de la [recherche](../subedit-gui/recherche.md) de la fenêtre, avec ses règles :

- **on cherche dans le texte visible** — ce que le spectateur lit. Les balises n'en font pas
  partie : chercher `<i>` ne trouve rien, et chercher `Bonjour` trouve un « Bonjour » dont la
  moitié est en italique ;
- **on remplace dans le texte source**, celui du fichier, en gardant les balises là où elles
  étaient. Une balise qui coupe le mot cherché englobe le remplacement entier ; un style qui
  touche la correspondance couvre tout le remplacement ; ce que la correspondance ne touche pas
  ne bouge pas. Les règles, cas par cas, sont celles de la recherche de la fenêtre.

**Il n'y a pas de sous-commande `find`** : pour voir où un motif se trouve sans rien changer,
c'est [`--dry-run`](invocation.md#voir-avant-décrire) qui liste chaque sous-titre concerné.

<!-- exemple: subedit-cli replace --help -->
```console
$ subedit-cli replace --help
Replace a text in the subtitles, without breaking a tag
Usage: subedit-cli replace [OPTIONS] pattern replacement files...

Positionals:
  pattern TEXT REQUIRED       Text to look for in what is shown
  replacement TEXT REQUIRED   What replaces it, tags allowed
  files TEXT ... REQUIRED     Subtitle files to change

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --regex                     Read the text to look for as a regular expression
  --case-sensitive            Tell capitals from small letters
  --range N-M|N-              Act only on subtitles N to M, or N to the end
  --output TEXT               File to write, for a single input
  --output-dir TEXT           Directory to write into
  --in-place                  Write back over the inputs
  --dry-run                   Work out and say what would be written, and write nothing
```

## Arguments et options

| Option | Requis | Valeurs | Défaut |
| :----- | :----- | :------ | :----- |
| `<motif>` | oui | le texte à chercher dans ce qui est affiché ; **non vide** | — |
| `<remplacement>` | oui | ce qui le remplace, balises permises ; peut être vide | — |
| `<fichier>...` | oui | un ou plusieurs chemins | — |
| `--recursive`, `-r` | non | un drapeau | désactivé — voir [Traiter un arbre](lots.md) |
| `--regex` | non | un drapeau : le motif est une expression régulière ICU | du texte simple |
| `--case-sensitive` | non | un drapeau : distingue majuscules et minuscules | elles sont ignorées |
| `--range` | non | `N-M`, ou `N-` jusqu'à la fin — voir [`adjust`](adjust.md#--range) | tout le fichier |
| `--output` / `--output-dir` / `--in-place` | **l'une des trois**, sauf avec `--dry-run` | voir [Invocation](invocation.md#la-destination) | — |
| `--dry-run` | non | un drapeau : calcule et dit, n'écrit rien — voir [Voir avant d'écrire](invocation.md#voir-avant-décrire) | désactivé |

**Un motif qui commence par un tiret** s'écrit après `--` : `subedit-cli replace -- -- "—" film.srt`.

**Le texte principal, et lui seul.** Un document de traduction n'est pas touché, la ligne de
commande n'ayant à ce jour aucun moyen d'en désigner un.

### Texte simple, ou expression

Par défaut le motif est **lu tel quel** : `a.b` cherche un `a`, un point, un `b`. Les
majuscules sont ignorées avec la même finesse que la fenêtre — `É` et `é` sont une seule lettre.

Avec `--regex`, le motif est une expression régulière **ICU**, lue comme Gaupol la lit : `.` traverse
un saut de ligne, `^` et `$` valent à chaque ligne d'un sous-titre. Dans le remplacement, et
seulement alors :

| Écrire | Pour dire |
| :----- | :-------- |
| `$1` à `$9` | les groupes de la correspondance |
| `$0` | la correspondance entière |
| `\n` | un saut de ligne |
| `\$`, `\\` | un `$`, une barre oblique inverse |

Pensez aux guillemets du shell : `'$1 EUR'`, et non `"$1 EUR"` qui laisserait le shell lire `$1`.

<!-- exemple: printf '1\n00:00:01,000 --> 00:00:02,000\nPrix: 12 euros, et 7 euros.\n\n' > prix.srt; subedit-cli replace --regex '([0-9]+) euros' '$1 EUR' --output euros.srt prix.srt; cat euros.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nPrix: 12 euros, et 7 euros.\n\n' > prix.srt; subedit-cli replace --regex '([0-9]+) euros' '$1 EUR' --output euros.srt prix.srt; cat euros.srt
prix.srt: replaced 2 matches -> euros.srt
1
00:00:01,000 --> 00:00:02,000
Prix: 12 EUR, et 7 EUR.
```

## Les balises

Un texte dont une balise coupe le mot cherché :

<!-- exemple: printf '1\n00:00:01,000 --> 00:00:02,000\n<i>Bon</i>jour tout le monde\n\n2\n00:00:03,000 --> 00:00:04,000\nBonjour <i>Marie</i>, ca va\n\n' > balises.srt; subedit-cli replace Bonjour Salut --output salut.srt balises.srt; cat salut.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n<i>Bon</i>jour tout le monde\n\n2\n00:00:03,000 --> 00:00:04,000\nBonjour <i>Marie</i>, ca va\n\n' > balises.srt; subedit-cli replace Bonjour Salut --output salut.srt balises.srt; cat salut.srt
balises.srt: replaced 2 matches -> salut.srt
1
00:00:01,000 --> 00:00:02,000
<i>Salut</i> tout le monde

2
00:00:03,000 --> 00:00:04,000
Salut <i>Marie</i>, ca va
```

Dans le premier sous-titre, la balise coupait « Bonjour » : elle englobe désormais « Salut ».
Dans le second, elle n'est pas touchée et reste où elle est. **Une balise laissée ouverte le
reste** : réparer un balisage bancal serait une autre opération.

Le remplacement se lit dans **les balises du format du fichier** : `<i>…</i>` pour du SubRip,
`{\i1}…` pour de l'Advanced SSA. C'est le seul moyen de mettre en forme ce qu'on écrit.

## Rien trouvé, rien à changer

Deux réponses qui ne sont pas des échecs — le code de retour est `0` et **le fichier est écrit
quand même**, une destination donnée étant une destination écrite :

| Situation | Phrase |
| :-------- | :----- |
| le motif n'est nulle part | `"zzz" not found` |
| il est là, mais chaque correspondance se remplace par elle-même | `nothing to change` |

<!-- exemple: printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\n\n' > a.srt; subedit-cli replace zzz x --output b.srt a.srt; subedit-cli replace Bonjour Bonjour --output c.srt a.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\n\n' > a.srt; subedit-cli replace zzz x --output b.srt a.srt; subedit-cli replace Bonjour Bonjour --output c.srt a.srt
a.srt: "zzz" not found -> b.srt
a.srt: nothing to change -> c.srt
```

## `--dry-run`

La sortie standard liste **chaque sous-titre dont le texte changerait**, avant et après, tels
qu'ils sont dans le fichier — balises comprises :

<!-- exemple: printf '1\n00:00:01,000 --> 00:00:02,000\n<i>Bon</i>jour tout le monde\n\n2\n00:00:03,000 --> 00:00:04,000\nBonjour <i>Marie</i>, ca va\n\n' > balises.srt; subedit-cli replace --dry-run Bonjour Salut balises.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n<i>Bon</i>jour tout le monde\n\n2\n00:00:03,000 --> 00:00:04,000\nBonjour <i>Marie</i>, ca va\n\n' > balises.srt; subedit-cli replace --dry-run Bonjour Salut balises.srt
balises.srt: replaced 2 matches (dry run, nothing written)
balises.srt: subtitle 1
- <i>Bon</i>jour tout le monde
+ <i>Salut</i> tout le monde
balises.srt: subtitle 2
- Bonjour <i>Marie</i>, ca va
+ Salut <i>Marie</i>, ca va
```

La forme de la liste est décrite dans [Voir avant d'écrire](invocation.md#voir-avant-décrire) ;
en JSON, c'est `changes`.

## Sortie

**Sortie standard** — rien : le résultat est le fichier écrit. Avec `--dry-run`, la liste des
changements ci-dessus.

| Niveau | Ce qui s'ajoute sur la sortie d'erreur |
| :----- | :------------------------------------- |
| 1 | `<chemin>: replaced N matches -> <destination>`, ou `"<motif>" not found`, ou `nothing to change` |
| 2 | `<chemin>: SubRip, UTF-8, no BOM, LF line endings kept` — le format, l'encodage, la marque et les fins de ligne du fichier lu, remis tels quels |
| 3 | `<chemin>: N bytes read, M written`, puis **chaque diagnostic de lecture** — voir [Invocation](invocation.md#les-diagnostics-de-lecture) |

`N` compte **les correspondances remplacées dans les sous-titres dont le texte a changé** — c'est le
nombre que la fenêtre dit aussi.

### En JSON

Avec [`--format json`](invocation.md#sortie-lisible-par-un-script), l'objet de chaque fichier porte :

| Clé | Contenu |
| :-- | :------ |
| `counts.replaced` | les correspondances remplacées, dans les sous-titres dont le texte a changé : le `N` de la phrase |
| `counts.matched` | les correspondances trouvées, **que leur remplacement ait changé le texte ou non** — c'est ce qui distingue « rien trouvé » (`0`) de « rien à changer » (`replaced` à `0`, `matched` positif) |
| `changes` | les sous-titres changés : `{"subtitle", "document", "before", "after"}`, **vide** quand rien n'a changé |

## Codes de retour

Ceux de l'outil : `0` si tous les fichiers ont été traités — un fichier où rien n'a été trouvé l'est
—, `2` si aucun, `3` si certains seulement, `1` sur une erreur d'usage.

## Erreurs

| Ce qui la déclenche | Message |
| :------------------ | :------ |
| motif vide | `pattern: nothing to look for` |
| expression illisible | `pattern: not a regular expression (<raison d'ICU>)` |
| plage mal écrite ou hors du fichier | voir [`adjust`](adjust.md#--range) |

**Le motif est lu avant le premier fichier** : une expression illisible est une erreur d'usage
(code `1`), rien n'est lu ni écrit.

Les erreurs de la destination sont communes aux sous-commandes qui écrivent :
voir [Invocation](invocation.md#la-destination).
