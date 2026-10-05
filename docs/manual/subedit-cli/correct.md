# `correct`

```
subedit-cli correct --tasks LISTE --code CODE
                    [--classes human|ocr|human,ocr]
                    [--enable NOM]... [--disable NOM]...
                    [--max-length N] [--max-lines N]
                    [--skip-length N|off] [--skip-lines N|off]
                    [--keep-blank-subtitles] [--range N-M|N-]
                    [--document main|translation] [-t FICHIER] [--align-method position|number]
                    (--output FICHIER | --output-dir DOSSIER | --in-place)
                    [--dry-run]
                    [--recursive]
                    <fichier>...
```

Corrige les textes avec les **motifs de Gaupol**, comme l'assistant
[`Correct Texts…`](../subedit-gui/correct-texts.md) de la fenêtre — mêmes motifs,
même moteur, mêmes comptes. Quatre tâches existent ici : les **mentions pour
malentendants**, les **erreurs courantes**, les **majuscules** et le **découpage de
lignes**. La jonction et la scission de mots ne sont pas offertes par cette
sous-commande.

<!-- exemple: subedit-cli correct --help -->
```console
$ subedit-cli correct --help
Correct the texts with the patterns of the correction assistant
Usage: subedit-cli correct [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to correct

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --tasks LIST                The tasks to run, in a list: none is chosen beforehand
  --code CODE                 The cascade of patterns the tasks read: Zyyy, Latn, Latn-en, Latn-en-US
  --classes human|ocr|human,ocr
                              The classes of common errors to apply: both by default
  --enable NAME               Switch a pattern on, by its English name or type:name; repeatable
  --disable NAME              Switch a pattern off, by its English name or type:name; repeatable
  --max-length N              Longest line of line-break, in characters; no default, Gaupol's is in ems
  --max-lines N               Most lines of line-break: 3, as Gaupol's
  --skip-length N|off         Leave alone a subtitle whose longest line is within this, or off: --max-length
  --skip-lines N|off          Leave alone a subtitle whose line count is within this, or off: --max-lines
  --keep-blank-subtitles      Leave the subtitles the correction empties, empty, instead of removing them
  --range N-M|N-              Act only on subtitles N to M, or N to the end
  --document main|translation The document to change: the main one, or the translation given by -t
  -t,--translation-file FILE  Translation file to lay over the subtitle file, for a single input
  --align-method position|number
                              How the lines of the translation find their subtitles
  --output TEXT               File to write, for a single input
  --output-dir TEXT           Directory to write into
  --in-place                  Write back over the inputs
  --dry-run                   Work out and say what would be written, and write nothing
```

## Ce qui est choisi, et ce qui ne l'est pas

**Rien n'est coché d'avance, et rien n'est lu des réglages de qui lance.** La fenêtre
ouvre ses cases dans l'état où l'utilisateur les a laissées ; une ligne de commande
n'a pas d'utilisateur de cette sorte. Le résultat dépend de ses arguments, des
fichiers qu'on lui donne, et de ce que l'installation fournit : ni les activations
cochées un jour dans la fenêtre, ni la liste de remplacements du correcteur ne sont
lues, et rien n'est écrit de ce côté.

| Option | Requis | Valeurs | Défaut |
| :----- | :----- | :------ | :----- |
| `<fichier>...` | oui | un ou plusieurs chemins | — |
| `--tasks` | **oui** | une liste séparée par des virgules : `mentions`, `common-errors`, `capitalization`, `line-break` | aucune tâche |
| `--code` | **oui**, une tâche de motifs étant donnée | `Écriture[-langue[-PAYS]]` : `Zyyy`, `Latn`, `Latn-en`, `Latn-en-US` | — |
| `--classes` | non | `human`, `ocr` ou `human,ocr` | les deux |
| `--enable`, `--disable` | non, répétables | le nom anglais d'un motif, ou `type:nom` | les `.conf` livrés |
| `--max-length` | **oui** avec `line-break` | un nombre de **caractères** supérieur à zéro | **aucun** — voir plus bas |
| `--max-lines` | non | un nombre entier de lignes, supérieur à zéro | 3 |
| `--skip-length`, `--skip-lines` | non | un nombre, ou `off` | les mêmes bornes que `--max-length` et `--max-lines` |
| `--keep-blank-subtitles` | non | un drapeau | les sous-titres vidés sont retirés |
| `--range` | non | `N-M`, ou `N-` jusqu'à la fin — voir [`adjust`](adjust.md#--range) | tout le fichier |
| `--document` | non | `main` ou `translation` — voir [Une traduction](invocation.md#une-traduction) | `main` |
| `-t`, `--translation-file` | avec `--document translation` | un fichier de traduction, pour **une seule** entrée | — |
| `--align-method` | non, et seulement avec `-t` | `position` ou `number` | `position` |
| `--recursive`, `-r` | non | un drapeau | désactivé — voir [Traiter un arbre](lots.md) |
| `--output` / `--output-dir` / `--in-place` | **l'une des trois**, sauf avec `--dry-run` | voir [Invocation](invocation.md#la-destination) | — |
| `--dry-run` | non | un drapeau : calcule et dit, n'écrit rien — voir [Voir avant d'écrire](invocation.md#voir-avant-décrire) | désactivé |

### Les tâches

**L'ordre de la liste n'y change rien** : les tâches s'appliquent dans l'ordre de
Gaupol — les mentions, puis les erreurs courantes, puis les majuscules, puis le découpage —, chacune sur le
texte que la précédente a laissé. Un sous-titre que les mentions ont vidé ne joue aucun
rôle dans ce qui suit.

| Tâche | Ce qu'elle fait |
| :---- | :-------------- |
| `mentions` | retire les bruits entre crochets et parenthèses (le balayage de [`hearing-impaired`](hearing-impaired.md)), puis les motifs de paroles et de locuteurs |
| `common-errors` | corrige les erreurs courantes : espaces, ponctuation, ligatures, erreurs d'OCR |
| `capitalization` | met la majuscule où la langue la veut |
| `line-break` | recoupe les lignes d'un sous-titre pour qu'aucune ne dépasse `--max-length`, en pesant les motifs de coupure du code |

### Le code des motifs

**`--code` nomme la cascade** : `Zyyy` pour toute écriture, puis l'écriture, la langue
et le pays — `Latn-en-US` lit les motifs de `Zyyy`, de `Latn`, de `Latn-en` et de
`Latn-en-US`. **Il est requis**, parce qu'une ligne de commande n'a pas de langue ambiante :
un défaut universel appliquerait *moins* que ce qu'on croit, sans le dire. Il vaut pour
toutes les tâches d'un même lancement ; une tâche qui veut un code à elle est un second
lancement. Un code mal écrit est une erreur d'usage :

<!-- exemple: printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\n\n' > a.srt; subedit-cli correct --tasks common-errors --code francais --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\n\n' > a.srt; subedit-cli correct --tasks common-errors --code francais --output b.srt a.srt; echo "code=$?"
--code: "francais" is not a pattern code: expected Script[-language[-COUNTRY]], like Zyyy, Latn or Latn-en-US
code=1
```

Les motifs que l'utilisateur dépose dans `$XDG_DATA_HOME/subedit/patterns` (ou
`~/.local/share/subedit/patterns`) **s'ajoutent aux motifs livrés**, comme dans la
fenêtre, et un motif livré qui ne se lit pas est dit au niveau 1 : `patterns: <fichier>, line N (<raison>)`.

### Le découpage de lignes

**Il se mesure en caractères, et c'est l'écart avec la fenêtre.** La fenêtre mesure en *ems* —
une largeur calibrée sur la police, à 0,55 em par lettre —, et son 24 par défaut est une valeur
en ems. Une ligne de commande n'a pas de police : elle compte des caractères, ceux du
texte. **Recopier le 24 en caractères couperait presque tout, sans
le dire** ; il n'y a donc **aucune longueur par défaut**, et `--max-length` est requis avec
`line-break` :

<!-- exemple: printf '1\n00:00:01,000 --> 00:00:03,000\nUn texte\n\n' > a.srt; subedit-cli correct --tasks line-break --code Latn-en --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nUn texte\n\n' > a.srt; subedit-cli correct --tasks line-break --code Latn-en --output b.srt a.srt; echo "code=$?"
--max-length is required by the task line-break: Gaupol's 24 is a width in ems, and has no value in characters
code=1
```

**`--max-lines` vaut 3**, la valeur de Gaupol, que l'unité ne change pas ; le découpage en prend
une de plus quand trois ne suffisent pas. **Le saut** — un sous-titre déjà dans les limites est
laissé tel quel — reprend par défaut **les mêmes bornes** (`--skip-length` = `--max-length`,
`--skip-lines` = `--max-lines`), comme Gaupol ; `--skip-length off` et `--skip-lines off`
l'éteignent, et le texte est alors relu comme un seul et recoupé :

<!-- exemple: printf '1\n00:00:01,000 --> 00:00:03,000\nWhen the rain stopped, we walked home under the dark sky of the old town.\n\n' > a.srt; subedit-cli correct --tasks line-break --code Latn-en --max-length 24 --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nWhen the rain stopped, we walked home under the dark sky of the old town.\n\n' > a.srt; subedit-cli correct --tasks line-break --code Latn-en --max-length 24 --output b.srt a.srt; cat b.srt
a.srt: Edited 1 and removed 0 subtitles -> b.srt
1
00:00:01,000 --> 00:00:03,000
When the rain
stopped, we walked
home under the dark
sky of the old town.
```

Les motifs de coupure sont ceux du code : `Latn` ne coupe pas entre un nombre et son unité ni
après un tiret de dialogue, `Latn-en` ne coupe pas après un article, une préposition, un titre
ou un déterminant possessif. **Sous `Zyyy`, aucun motif de coupure n'existe** : les lignes sont
seulement équilibrées. **Cette tâche n'est jamais refusée faute de motif actif** : ses motifs
pèsent où couper, la coupure se fait sans eux. Les quatre options n'ont de sens qu'avec
`line-break` ; données sans lui, elles sont une erreur d'usage.

### Les classes, et l'activation par le nom

**`--classes` filtre les erreurs courantes pour de bon** : décocher `ocr` retire ses
motifs de l'application — la ligature `ﬁ` reste alors `ﬁ`. Les trois autres types de motifs
ne portent pas de classe.

**`--enable` et `--disable` règlent un motif par son nom anglais**, le nom étant la clé
(ADR 0037) ; un nom peut désigner plusieurs enregistrements, qui partagent alors une seule
case. C'est nécessaire, et c'est ce qui écarte les valeurs par défaut de la fenêtre : **les
`.conf` livrés désactivent les motifs de mentions**, si bien que `--tasks mentions` seul
n'appliquerait rien. Les deux motifs « Sound in brackets » et « Sound in parentheses »
commandent le balayage des crochets et des parenthèses, et se règlent par les mêmes
options.

Trois choses sont des **erreurs d'usage**, code `1`, dites avant qu'un fichier soit lu :

| Ce qui la déclenche | Message |
| :------------------ | :------ |
| un nom qui ne désigne aucun motif des tâches données, sous le code donné | `--enable: "<nom>" names no pattern of the tasks given, under the code <code>` |
| un nom qui désigne des motifs de deux types — il s'écrit alors `type:nom`, le type étant `common-error`, `capitalization`, `hearing-impaired` ou `line-break` | `--enable: "<nom>" names patterns of several types: write type:name, …` |
| un nom donné à `--enable` et à `--disable` | `"<nom>" is given to --enable and to --disable: choose one` |

**Une faute de frappe est bruyante** : la fenêtre ignore un réglage périmé parce qu'un
réglage peut survivre à un motif disparu ; une invocation n'a pas de survivant.

**Une tâche sans aucun motif actif est refusée**, code `1` — « rien à faire » est une
réponse qui se dit, pas un succès silencieux :

<!-- exemple: printf '1\n00:00:01,000 --> 00:00:02,000\n[Bruit] Bonjour\n\n' > a.srt; subedit-cli correct --tasks mentions --code Latn --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n[Bruit] Bonjour\n\n' > a.srt; subedit-cli correct --tasks mentions --code Latn --output b.srt a.srt; echo "code=$?"
mentions: no pattern is active under the code Latn; switch one on with --enable
code=1
```

Le même fichier, avec le balayage des crochets mis en route :

<!-- exemple: printf '1\n00:00:01,000 --> 00:00:02,000\n[Bruit] Bonjour\n\n' > a.srt; subedit-cli correct --tasks mentions --code Latn --enable "Sound in brackets" --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n[Bruit] Bonjour\n\n' > a.srt; subedit-cli correct --tasks mentions --code Latn --enable "Sound in brackets" --output b.srt a.srt; cat b.srt
a.srt: Edited 1 and removed 0 subtitles -> b.srt
1
00:00:01,000 --> 00:00:02,000
Bonjour
```

## Ce qui est écrit, et ce qui est dit

Le format, l'encodage et les fins de ligne du fichier lu sont **conservés**. **Les
sous-titres que la correction vide sont retirés**, comme la case par défaut de la
fenêtre ; `--keep-blank-subtitles` les laisse vides. **Une traduction vidée reste vidée**,
et garde son sous-titre.

**Le compte est celui de la fenêtre** — « Edited N and removed M subtitles » : des textes
changés et des sous-titres retirés, **jamais des correspondances**. Un fichier où rien ne
change est écrit quand même : une destination donnée est une destination écrite.

<!-- exemple: printf '1\n00:00:01,000 --> 00:00:02,000\neﬀet  de ﬁn\n\n2\n00:00:03,000 --> 00:00:04,000\nPropre.\n\n' > a.srt; subedit-cli correct --tasks common-errors --code Latn --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\neﬀet  de ﬁn\n\n2\n00:00:03,000 --> 00:00:04,000\nPropre.\n\n' > a.srt; subedit-cli correct --tasks common-errors --code Latn --output b.srt a.srt; cat b.srt
a.srt: Edited 1 and removed 0 subtitles -> b.srt
1
00:00:01,000 --> 00:00:02,000
effet de fin

2
00:00:03,000 --> 00:00:04,000
Propre.
```

### Un motif qui ne peut pas s'appliquer

**Un motif qui ne se lit pas, ne se traduit pas ou ne termine pas est nommé, avec le
sous-titre où il a renoncé**, et c'est ici que la ligne de commande dit plus que la
fenêtre : la page de confirmation nomme le motif une seule fois, sans le texte.

| Raison | Ce qu'elle veut dire |
| :----- | :------------------- |
| `cannot be translated` | son expression emploie ce que la traduction vers ICU refuse |
| `will not compile` | le moteur a refusé son expression |
| `has an invalid replacement` | son remplacement ne se lit pas comme Python le lit |
| `timed out` | le moteur a renoncé sur ce texte |
| `never settled` | `Repeat` changeait encore le texte après cent passes |
| `grew the text too long` | le texte dépassait ce qu'un sous-titre peut tenir |

Au niveau 1, sur la sortie d'erreur : `<chemin>: pattern "<nom>" (<raison>) was not applied to
subtitle N` — sans la fin quand le motif est écarté pour tous les textes. **Le numéro est celui du
fichier**, quelles que soient les tâches qui ont retiré des sous-titres avant. En JSON, c'est
un avertissement `pattern-failed` du fichier, dont `detail` porte la phrase. **Un motif
abandonné n'est pas un fichier en échec** : le fichier est écrit, et le code de retour est `0`.

## Sortie

**Sortie standard** — rien, le résultat étant le fichier écrit. **Avec `--dry-run`, ce sont les
changements proposés**, un bloc par sous-titre : son numéro, le texte d'avant, le texte d'après —
et `(removed)` sans ligne `+` pour un sous-titre que la correction retirerait ; avec
`--keep-blank-subtitles`, il reste, vide. Voir [Voir avant d'écrire](invocation.md#voir-avant-décrire).

| Niveau | Ce qui s'ajoute sur la sortie d'erreur |
| :----- | :------------------------------------- |
| 1 | `<chemin>: Edited N and removed M subtitles -> <destination>`, les motifs qui n'ont pas pu s'appliquer, les fichiers de motifs qui ne se lisent pas |
| 2 | `<chemin>: SubRip, UTF-8, no BOM, LF line endings kept` |
| 3 | `<chemin>: N bytes read, M written`, puis chaque diagnostic de lecture |

En JSON, `counts` porte `corrected` (les textes changés) et `removed` (les sous-titres
retirés), et `changes` les sous-titres touchés. Avec `-t`, la clé `alignment` dit comment la
traduction a été posée.

## Codes de retour

Ceux de l'outil : `0` si tous les fichiers ont été écrits, `2` si aucun, `3` si certains
seulement, `1` sur une erreur d'usage.

## Erreurs

| Ce qui la déclenche | Ce qui est écrit, sur la sortie d'erreur |
| :------------------ | :--------------------------------------- |
| `--tasks` absent | `--tasks is required`, suivi d'un renvoi à `--help` |
| une tâche inconnue | `--tasks: "<nom>" is not a task: expected mentions, common-errors, capitalization or line-break` |
| `--code` absent | `--code is required by the tasks that read patterns: <tâches>` |
| un code mal écrit | `--code: "<code>" is not a pattern code: expected Script[-language[-COUNTRY]], like Zyyy, Latn or Latn-en-US` |
| `--max-length` absent avec `line-break` | `--max-length is required by the task line-break: Gaupol's 24 is a width in ems, and has no value in characters` |
| `--max-length` qui n'est pas un nombre supérieur à zéro | `--max-length: "<valeur>" is not a length: expected a number of characters greater than zero` |
| `--max-lines`, `--skip-lines` qui ne sont pas un nombre entier de lignes | `--max-lines: "<valeur>" is not a number of lines: expected a whole number greater than zero` |
| `--skip-length` ou `--skip-lines` illisible | `--skip-length: "<valeur>" is not a bound: expected a number greater than zero, or off` |
| l'une des quatre options sans `line-break` | `--max-length is for the task line-break, which was not asked for` |
| une classe inconnue | `--classes: "<nom>" is not a class: expected human, ocr or human,ocr` |
| un nom de motif fautif | voir la table plus haut |
| une tâche sans motif actif | `<tâche>: no pattern is active under the code <code>; switch one on with --enable` |
| `--range`, `-t`, `--document`, la destination | ceux de [Invocation](invocation.md#erreurs) et de [Une traduction](invocation.md#une-traduction) |
