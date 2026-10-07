# Invocation

```
subedit-cli [options globales] <sous-commande> [options] <fichier>...
```

Sans sous-commande, l'outil écrit son aide et s'arrête.

<!-- exemple: subedit-cli -->
```console
$ subedit-cli
Read, inspect and retime subtitle files.
Usage: subedit-cli [OPTIONS] [SUBCOMMAND]

Options:
  -h,--help                   Print this help message and exit
  --version                   Display program version information and exit
  -v                          Say more: -v is the default, -vv details, -vvv debugs
  -q,--quiet                  Say nothing but errors
  --encoding NAME             Encoding to read the files in; detected by default
  --format text|json          Form of the result on standard output: text or json
  --list-encodings            Write the encodings that can be read and written, one a line, and stop

Subcommands:
  inspect                     Report what a subtitle file is made of
  convert                     Write a subtitle file out in another format or shape
  shift                       Move every position of a file by a fixed amount
  transform                   Correct every position from two points known to be right
  framerate                   Re-time a file mastered at one frame rate for another
  snap                        Move every position onto the nearest frame of a frame rate (see framerate)
  hearing-impaired            Remove the sounds described between brackets or parentheses
  adjust                      Bring the duration of every subtitle within a reading speed and limits
  replace                     Replace a text in the subtitles, without breaking a tag
  case                        Put the texts in title, sentence, upper or lower case, tags intact
  italics                     Put the texts in italics, or take their italics out
  dialogue-dashes             Put dialogue dashes on the lines, or take them off
  sort                        Put the subtitles in the order of their start, keeping ties as they are
  correct                     Correct the texts with the patterns of the correction assistant
  append                      Put subtitle files one after another into a single file
  split-file                  Cut a subtitle file in two, a head and a tail
  pair                        Write a translation at the positions of its main file, and say how it lined up
```

## La ligne de commande est en anglais

Noms de sous-commandes, noms d'options, aide et messages d'erreur sont en
anglais. Ce manuel reste en français et cite la sortie telle qu'elle est
produite — ces blocs sont engendrés par `make manual`, jamais recopiés.

## Options globales

**Elles s'écrivent avant le nom de la sous-commande, et là seulement.**
`subedit-cli --encoding cp1252 inspect film.srt` est la forme ;
`subedit-cli inspect --encoding cp1252 film.srt` est refusé, avec
`The following argument was not expected: --encoding`. Il en va de même de
`--version`, de `-v` et de `--quiet`.

**`--help` est l'exception, et elle n'en est pas une** : `subedit-cli inspect
--help` est accepté parce que chaque sous-commande porte le sien, et c'est celui
de la sous-commande qu'il écrit, pas celui de l'outil.

| Option | Effet |
| :----- | :---- |
| `-h`, `--help` | écrit l'aide et s'arrête ; celle de la sous-commande si l'on en nomme une |
| `--version` | écrit `subedit <version>` et s'arrête |
| `-q`, `--quiet` | niveau 0 — plus aucune narration |
| `-v`, `-vv`, `-vvv` | niveaux 1 à 3 ; le niveau 1 est celui par défaut |
| `--encoding NOM` | lit les fichiers dans cet encodage, au lieu de le deviner |
| `--list-encodings` | écrit les encodages que l'outil sait lire et écrire, un par ligne, et **s'arrête** (code `0`) — voir [`--list-encodings`](#--list-encodings) |
| `--format text\|json` | forme du résultat sur la sortie standard : le texte, ou un objet JSON par fichier — voir [Sortie lisible par un script](#sortie-lisible-par-un-script) |

`--quiet` et `-v` dans la même invocation sont refusés : deux intentions
opposées ne sont pas arbitrées au profit de la dernière écrite.

### `--list-encodings`

**Les encodages que l'outil sait lire et écrire, un par ligne, triés par nom**, sur la sortie standard —
puis l'outil s'arrête, avec le code `0`, comme `--version` : c'est la réponse, non une étape. Une
sous-commande donnée en plus n'est pas exécutée.

<!-- exemple: subedit-cli --list-encodings | grep '^UTF' | head -4 -->
```console
$ subedit-cli --list-encodings | grep '^UTF' | head -4
UTF-16BE
UTF-16LE
UTF-32BE
UTF-32LE
```

- **Ce sont les noms que `--encoding` accepte**, canoniques ; ses alias (`cp1252`) y sont acceptés aussi,
  sans y figurer.
- **Le nombre dépend de l'ICU installée**, et aucun nom n'est promis hors de ceux que toute ICU a
  (`UTF-8`, `UTF-16LE`, `windows-1252`…). Le manuel ne donne donc pas de nombre.
- **`UTF-16` et `UTF-32` n'y sont pas** : ICU les connaît, mais leur convertisseur écrit sa propre marque,
  et le modèle les refuse — voir plus bas. Une liste où l'on choisit ne propose pas ce que le champ suivant
  refuserait.
- **Avec `--format json`**, **un objet par encodage** :
  `{"schema":1,"command":"list-encodings","name":"UTF-8"}`. Ce n'est pas le compte rendu d'un fichier : ces
  objets **ne portent ni `file` ni `ok`**, la seule exception à l'enveloppe de
  [la forme](#la-forme). Ils restent un objet par ligne, un par entrée.
- **Le niveau de narration n'y change rien** : une liste est un résultat, et `-q` ne l'enlève pas.

### `--encoding`, et ce qu'il ne peut pas forcer

**Il vaut pour toutes les sous-commandes**, parce que toutes lisent : un fichier
dont l'encodage est mal deviné l'est quoi qu'on lui fasse, et une option que
seul `inspect` porterait laisserait un décalage sans recours.

**Le nom est celui d'ICU, et tous ses alias sont acceptés** : `cp1252`,
`windows-1252` et `WINDOWS 1252` désignent le même encodage. L'ensemble n'est
écrit nulle part : c'est celui que l'ICU de la machine sait convertir, et
[`--list-encodings`](#--list-encodings) l'énumère. Un nom qu'elle ne sait pas
convertir est refusé avant qu'un seul fichier soit lu :

```console
$ subedit-cli --encoding klingon-1 inspect film.srt
no encoding is named "klingon-1"
```

**Une exception, et une seule : l'encodage doit nommer son ordre d'octets.**
`UTF-16` et `UTF-32` sont des noms qu'ICU connaît et dont le convertisseur
écrit **sa propre marque**. Un encodage pareil ne peut pas vivre dans ce
modèle — la marque cesse d'être un choix, `--no-bom` serait accepté et
désobéi — donc les deux sont refusés, à la lecture comme à l'écriture :

```console
$ subedit-cli --encoding UTF-16 inspect film.srt
"UTF-16" writes a byte order mark of its own; name the byte order, as UTF-16LE and UTF-16BE do
```

`UTF-16LE` et `UTF-16BE` lisent et écrivent les mêmes octets, marque comprise ;
ce que le nom précis ajoute est qu'on sache lesquels.

**Une marque d'ordre des octets l'emporte sur l'option.** C'est la seule chose
qu'un fichier de sous-titres déclare de lui-même, et le lire autrement qu'il ne
se déclare serait obéir à l'appelant contre le fichier. L'écart est **dit**, en
diagnostic :

```console
$ subedit-cli -vvv --encoding cp1252 inspect avec-bom.srt
avec-bom.srt: a byte order mark that contradicts the encoding asked for ("UTF-16LE"), settled by the reader
```

C'est ce que fait Gaupol — il recommence la lecture avec l'encodage de la
marque — mais sans le dire ; ici l'écart entre ce qui a été demandé et ce qui a
été fait n'est pas un silence.

<!-- exemple: subedit-cli --version -->
```console
$ subedit-cli --version
subedit 0.14.8
```

## Sous-commandes

| Sous-commande | Ce qu'elle fait | Écrit-elle ? |
| :------------ | :-------------- | :----------- |
| [`inspect`](inspect.md) | rapporte ce qu'un fichier contient, sans rien modifier | non |
| [`convert`](convert.md) | écrit un fichier dans un autre format, ou une autre forme | oui |
| [`shift`](shift.md) | décale toutes les positions d'une même durée | oui |
| [`transform`](transform.md) | corrige toutes les positions à partir de deux repères | oui |
| [`framerate`](framerate.md) | recale un fichier d'une cadence d'images vers une autre | oui |
| [`snap`](snap.md) | repose chaque horodatage sur l'image la plus proche | oui |
| [`hearing-impaired`](hearing-impaired.md) | retire les mentions pour malentendants | oui |
| [`adjust`](adjust.md) | ajuste les durées à une vitesse de lecture et à des bornes | oui |
| [`replace`](replace.md) | remplace un texte dans les sous-titres, sans casser une balise | oui |
| [`case`](case.md) | met les textes en casse de titre, de phrase, en capitales ou en minuscules | oui |
| [`italics`](italics.md) | met les textes en italique, ou retire leur italique | oui |
| [`dialogue-dashes`](dialogue-dashes.md) | pose ou retire les tirets de dialogue | oui |
| [`sort`](sort.md) | remet les sous-titres dans l'ordre de leur début | oui |
| [`correct`](correct.md) | corrige les textes avec les motifs de Gaupol : mentions, jonction et scission de mots, erreurs courantes, majuscules, découpage de lignes | oui |
| [`append`](append.md) | met des fichiers à la suite du premier, dans **un seul** fichier | oui |
| [`split-file`](split-file.md) | coupe un fichier en **deux** : une tête et une queue | oui |
| [`pair`](pair.md) | écrit une traduction aux positions de son principal, et dit comment ses lignes se sont posées | oui |

Les dix-sept sont là ; l'aide de l'outil les énumère dans le même ordre.

**Une seule ne touche à rien**, et c'est la colonne de droite : `inspect` lit et
rapporte. Les seize autres écrivent, et ce qui suit vaut pour quatorze d'entre elles :
[`append`](append.md) et [`split-file`](split-file.md) ont chacune une arité à elles — N fichiers pour
**un** résultat, un fichier pour **deux** — et donc leur propre destination, qu'elles décrivent sur leur page.

## La destination

Les quatorze sous-commandes qui écrivent — [`convert`](convert.md),
[`shift`](shift.md), [`transform`](transform.md), [`framerate`](framerate.md),
[`snap`](snap.md), [`hearing-impaired`](hearing-impaired.md), [`adjust`](adjust.md),
[`replace`](replace.md), [`case`](case.md), [`italics`](italics.md),
[`dialogue-dashes`](dialogue-dashes.md), [`sort`](sort.md), [`correct`](correct.md),
[`pair`](pair.md) — prennent leur destination de la même façon. ([`append`](append.md) et
[`split-file`](split-file.md) ont la leur, sur leur page.) [`inspect`](inspect.md) n'écrit aucun fichier et
n'accepte aucune de ces options.

**Une destination qui existe déjà est écrasée**, sans question ni option pour
l'éviter : l'écriture est atomique — le fichier n'est jamais écrit à moitié, et
une écriture interrompue laisse l'ancien intact —, mais le contenu d'avant est
perdu. Ce n'est pas un défaut qu'on garde : c'est la règle, et rejouer une
commande sur le même dossier de sortie la rejoue par-dessus elle-même.

**Rien n'est jamais écrit sans destination explicite.** Les trois façons de la
donner s'excluent, et il en faut une : sans elle, rien n'est écrit et le code de
retour est `1` — sauf avec [`--dry-run`](#voir-avant-décrire), qui n'écrit rien
et n'en exige donc aucune.

| Option | Où va le résultat |
| :----- | :---------------- |
| `--output FICHIER` | dans ce fichier, sous ce nom exact ; une seule entrée |
| `--output-dir DOSSIER` | dans ce dossier, sous le nom de base de l'entrée |
| `--in-place` | par-dessus l'entrée, par écriture atomique |

Un outil qui écrase son entrée parce qu'on ne lui a rien dit est un outil qu'on
cesse d'utiliser à la deuxième fois. Le refus coûte une ligne et sauve le
fichier.

`--output` avec plusieurs entrées est refusé : le dernier fichier s'écrirait sur
les précédents. Avec un lot, `--output-dir` est le seul des trois qui ait un
sens, avec `--in-place`.

**Le dossier de sortie est créé** s'il manque, parents compris — pour
`--output-dir`, et pour le dossier où `--output` écrit. C'est fait une fois la
ligne de commande validée et **avant le premier fichier**. S'il ne peut pas
l'être, c'est dit une fois, aucun fichier n'est lu, et le code de retour est
`2`.

**Le lot est jugé en entier avant d'écrire.** La destination de chaque entrée
est calculée d'abord, et deux refus en découlent — des erreurs d'usage, code
`1`, **aucun fichier écrit** et aucun dossier créé :

- **deux entrées qui aboutiraient au même fichier** — `a/film.srt` et
  `b/film.srt` avec `--output-dir`, ou `a/film.srt` et `b/film.vtt` converties
  toutes deux en WebVTT, dont la destination finale a changé d'extension. Le
  message nomme la destination et les deux entrées ; renommer la seconde ou
  laisser la dernière gagner perdrait un fichier sans le dire, et le refuser
  coûte une ligne ;
- **une destination qui est l'une des entrées**, sans `--in-place` — y compris
  par `--output-dir .`, par `--output` ou par un lien symbolique. Écrire par-dessus
  son entrée est un geste qu'on nomme.

Deux chemins désignent le même fichier quand le système le dit, non quand ils
s'écrivent pareil : `in/../in/film.srt` et `in/film.srt` sont un seul fichier.

**L'extension suit le format écrit.** Elle ne change que pour
[`convert`](convert.md), seule sous-commande qui change de format ; les
autres conservent celui du fichier lu, donc son extension.

| Ce qui la déclenche | Message |
| :------------------ | :------ |
| aucune destination, sans `--dry-run` | `no destination given: use --output, --output-dir or --in-place` |
| deux destinations | `--output, --output-dir and --in-place exclude one another` |
| `--output` sur un lot | `--output names one file but several were given: use --output-dir instead` |
| deux entrées, une destination | `<destination>: would be written by both <entrée> and <entrée>` |
| une entrée serait écrasée | `<destination>: would be written over the input <entrée>` — la même phrase pour [`append`](append.md) et [`split-file`](split-file.md) |
| dossier de sortie impossible à créer | `<dossier>: cannot be created: permission denied`, ou `: cannot be created` |
| destination non inscriptible | `<chemin>: <destination>: cannot be written: permission denied`, ou `: cannot be written` |
| caractère absent de l'encodage écrit | `<chemin>: holds a character the chosen encoding cannot write` |

**Le dernier mérite une phrase.** Un fichier est réécrit dans **l'encodage où il
a été lu**, comme il l'est avec ses fins de ligne et sa marque. Si une
modification y a introduit un caractère que cet encodage ne connaît pas — un
`ł` dans du Latin-1 —, l'écriture échoue et **rien n'est écrit**. Le remplacer
par un `?` serait perdre du texte sous les yeux de qui vient de l'écrire.

## Voir avant d'écrire

**`--dry-run`** est accepté par chacune des seize sous-commandes qui écrivent, et dit la même
chose partout : **lire, calculer, rendre compte, n'écrire aucun fichier**. On l'ajoute à la
ligne qu'on s'apprêtait à lancer ; appliquer, c'est relancer la même ligne sans lui. Le calcul est
déterministe : ce qu'un `--dry-run` a montré est ce que le lancement suivant écrira, tant que le
fichier n'a pas changé.

- **Il n'écrit aucun fichier et ne crée aucun dossier**, `--in-place` compris : le fichier
  d'entrée reste tel qu'il était.
- **Il n'exige aucune destination.** On peut en donner une, et **elle est alors vérifiée comme sans
  `--dry-run`** : deux entrées pour une même destination, une entrée qui serait écrasée,
  `--output` sur un lot, deux façons de dire où — une ligne de commande refusée sans `--dry-run`
  l'est avec, par la même phrase et le même code `1`.
- **Le code de retour est celui d'un vrai lancement** : `0` quand tout s'est calculé, `2` ou `3`
  quand des fichiers n'ont pas pu être lus ou traités, `1` pour l'usage. Un caractère que
  l'encodage du fichier ne saurait pas écrire fait échouer un lancement à blanc comme un vrai.
  **Que des changements existent ne change pas le code** : les quatre codes disent si l'outil a
  réussi, non si le fichier est conforme.
- **La narration le dit.** Là où un vrai lancement écrit `-> <destination>`, un lancement à
  blanc écrit `(dry run, nothing written)`.

<!-- exemple: printf '1\n00:00:10,000 --> 00:00:12,000\nBonjour.\n\n' > a.srt; subedit-cli shift --by 1 --dry-run a.srt; ls -- *.srt -->
```console
$ printf '1\n00:00:10,000 --> 00:00:12,000\nBonjour.\n\n' > a.srt; subedit-cli shift --by 1 --dry-run a.srt; ls -- *.srt
a.srt: 1 subtitle shifted by 1.000 s (dry run, nothing written)
a.srt
```

**Sur une sous-commande qui change des textes** —
[`hearing-impaired`](hearing-impaired.md), [`replace`](replace.md), [`case`](case.md),
[`italics`](italics.md), [`dialogue-dashes`](dialogue-dashes.md) et [`correct`](correct.md) —, **la sortie standard porte les changements
proposés**. Un bloc par sous-titre changé, dans l'ordre du fichier :

```
<chemin>: subtitle <numéro>
- <le texte d'avant, une ligne par ligne>
+ <le texte d'après, une ligne par ligne>
```

- le **numéro** est celui que le sous-titre a dans le fichier lu, compté depuis un ;
- chaque ligne du texte d'avant porte `- ` devant, chaque ligne du texte d'après `+ ` : un texte
  de plusieurs lignes se lit tel quel, et une ligne qui commence elle-même par un tiret de
  dialogue donne `- - ` ;
- **un sous-titre que l'opération supprimerait** le dit sur sa première ligne,
  `<chemin>: subtitle <numéro> (removed)`, et n'a pas de lignes `+` ;
- un sous-titre de la traduction, avec `--document translation`, s'annonce `(translation)` à la même
  place — voir [Une traduction](#une-traduction) ;
- la première ligne de chaque bloc commence par le chemin, de sorte que `grep '^film.srt: '`
  retrouve les blocs d'un fichier dans un lot ;
- **aucun bloc** quand rien ne changerait.

<!-- exemple: printf '1\n00:00:01,000 --> 00:00:03,000\n[Bruit de pas]\n\n2\n00:00:04,000 --> 00:00:06,000\nAttends [il tousse] Marie.\n\n3\n00:00:07,000 --> 00:00:09,000\n- [Grincement]\n- Qui est là ?\n\n' > mentions.srt; subedit-cli hearing-impaired --dry-run mentions.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\n[Bruit de pas]\n\n2\n00:00:04,000 --> 00:00:06,000\nAttends [il tousse] Marie.\n\n3\n00:00:07,000 --> 00:00:09,000\n- [Grincement]\n- Qui est là ?\n\n' > mentions.srt; subedit-cli hearing-impaired --dry-run mentions.srt
mentions.srt: 2 subtitles cleaned, 1 removed (dry run, nothing written)
mentions.srt: subtitle 1 (removed)
- [Bruit de pas]
mentions.srt: subtitle 2
- Attends [il tousse] Marie.
+ Attends Marie.
mentions.srt: subtitle 3
- - [Grincement]
- - Qui est là ?
+ Qui est là ?
```

**La liste est la même en JSON** : l'objet du fichier porte un tableau `changes`, voir
[la forme](#la-forme), avec `after` valant `null` pour un sous-titre supprimé. **Elle n'est
calculée que quand `--dry-run` ou `--format json` la demande** : un lot de milliers de
sous-titres ne la construit pas pour rien.

Sur **les autres sous-commandes** — [`convert`](convert.md), [`shift`](shift.md),
[`transform`](transform.md), [`framerate`](framerate.md), [`snap`](snap.md),
[`adjust`](adjust.md), [`sort`](sort.md), [`append`](append.md), [`split-file`](split-file.md), [`pair`](pair.md) —, la sortie
standard porte ce que porterait celle d'un vrai lancement, c'est-à-dire rien en texte ; en
JSON, l'objet de chaque fichier avec `"dry_run":true`, `"destination":null` et les mêmes
`counts`.

## Deux sorties, deux rôles

| Sortie | Ce qu'elle porte |
| :----- | :--------------- |
| standard | **le résultat, et lui seul** — le rapport d'`inspect`, les changements proposés d'un [`--dry-run`](#voir-avant-décrire) de texte ; avec `--format json`, un objet par fichier — un par lancement pour [`append`](append.md), [`split-file`](split-file.md) et [`pair`](pair.md) |
| erreur | **tout le reste** — la narration, les avertissements, les erreurs |

C'est ce partage qui permet de rediriger le résultat sans y récupérer le récit :

```console
$ subedit-cli inspect *.srt > rapport.txt
```

Le rapport part dans le fichier, la narration reste à l'écran.

## Sortie lisible par un script

**`--format json`** remplace le texte de la sortie standard par **un objet JSON par
fichier d'entrée**, au format JSON Lines : un objet compact par ligne, en UTF-8 sans
marque, la fin de ligne est `\n`. `text` est le défaut et ne change pas. C'est une
option globale, qui s'écrit donc avant la sous-commande ; une valeur autre que
`text` et `json` est une erreur d'usage, code `1`.

<!-- exemple: printf '1\n00:00:10,000 --> 00:00:12,000\nUn.\n\n' > a.srt; subedit-cli --format json shift --by 1 --output-dir sortie a.srt absent.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:10,000 --> 00:00:12,000\nUn.\n\n' > a.srt; subedit-cli --format json shift --by 1 --output-dir sortie a.srt absent.srt; echo "code=$?"
a.srt: 1 subtitle shifted by 1.000 s -> sortie/a.srt
{"schema":1,"command":"shift","file":"a.srt","ok":true,"dry_run":false,"destination":"sortie/a.srt","counts":{"subtitles":1,"shifted_by_ms":1000},"warnings":[]}
absent.srt: does not exist
{"schema":1,"command":"shift","file":"absent.srt","ok":false,"error":{"kind":"not-found","message":"absent.srt: does not exist"}}
1 of 2 files shifted, 1 failed
code=3
```

**Un objet par entrée, jamais zéro, jamais deux**, que l'entrée ait réussi ou non, dans
l'ordre où les entrées ont été données — ou trouvées, avec [`--recursive`](lots.md). On
compte les lignes, on retrouve ses fichiers. **Trois sous-commandes font exception, parce que leur
arité n'est pas celle d'un lot : [`append`](append.md) (N entrées, une sortie), [`split-file`](split-file.md)
(une entrée, deux sorties) et [`pair`](pair.md) (deux fichiers, une sortie) écrivent **un objet pour tout le
lancement**, qui nomme le fichier que leur page dit.** **Les sous-commandes qui réécrivent écrivent
donc aussi sur la sortie standard**, ce que leur contrat ne faisait pas en texte : leur
fichier reste leur résultat, et l'objet en est le compte rendu.

**La narration ne change pas.** `-q`, `-v`, `-vv` et `-vvv` agissent sur la sortie
d'erreur, en texte, comme sans `--format` ; **la sortie standard ne varie pas avec eux**.
Les diagnostics de lecture, que le texte range au niveau 3, sont des données et sont
**toujours** dans `warnings`. Une erreur d'usage n'écrit aucun objet : elle est détectée
avant de toucher un fichier, il n'y a donc rien à quoi l'attacher ; elle va sur la sortie
d'erreur avec le code `1`, et la sortie standard reste vide.

### La forme

| Clé | Contenu | Présente |
| :-- | :------ | :------- |
| `schema` | entier, la version de cette forme — `1` | toujours |
| `command` | le nom de la sous-commande | toujours |
| `file` | le chemin **tel que donné**, ou tel que le parcours d'un répertoire l'a composé | toujours |
| `ok` | booléen | toujours |
| `error` | `{"kind", "message"}` | si `ok` est faux |
| `dry_run` | booléen : vrai pour un [`--dry-run`](#voir-avant-décrire) | si `ok` est vrai, sur une sous-commande qui écrit |
| `destination` | le chemin écrit, ou `null` quand `dry_run` est vrai : rien n'a été écrit | idem |
| `counts` | un objet d'**entiers**, propre à la sous-commande | idem |
| `changes` | un tableau de `{"subtitle", "document", "before", "after"}` : les textes changés, voir ci-dessous | sur une sous-commande qui change des textes |
| `warnings` | un tableau de `{"kind", "line"?, "detail"?, "settled"?}` | si `ok` est vrai |
| `alignment` | comment une traduction a été posée, voir [Une traduction](#une-traduction) | avec `-t`, et pour `pair` |
| `constraints` | les réglages employés | `adjust` |
| `inputs`, `tail` | un élément par fichier ajouté ; le chemin de la queue | `append` ; `split-file` |

`line` est absent d'un diagnostic qui parle du fichier entier ; `detail` l'est quand il n'y
a rien à ajouter ; `settled` vaut `true` quand la lecture a **tranché** quelque chose — une
numérotation régénérée —, et est absent quand elle a laissé tel quel.

**`changes`** liste, dans l'ordre du fichier, chaque sous-titre dont le texte change :
`subtitle` est son numéro dans le fichier lu, compté depuis un ; `document` vaut `"main"`
ou `"translation"` ; `before` est le texte d'avant ; `after` celui d'après, ou **`null` quand
le sous-titre est supprimé**. Les retours à la ligne d'un texte y sont des `\n`. Elle est
écrite pour un vrai lancement comme pour un [`--dry-run`](#voir-avant-décrire), et **vide
(`[]`) quand rien ne change** : sa présence dit que la sous-commande liste ses changements,
son absence qu'elle n'en a pas.

**Aucun nombre à virgule, nulle part.** Une position ou une durée est un entier de
**millisecondes** (`…_ms`) ; un compte est un entier ; une cadence est une **chaîne**
(`"25"`, `"24000/1001"`) parce qu'elle est exacte, un rationnel que la virgule flottante ne
tient pas ; une concentration de grille est un entier en **millièmes** — `1000` est une
grille parfaite. Deux sorties se comparent alors octet pour octet, sans arrondi.

**Un chemin qui n'est pas de l'UTF-8** — Linux le permet — ne se représente pas en JSON : il
est écrit avec U+FFFD à la place des octets invalides, et l'objet porte l'avertissement
`path-not-utf8`. La fidélité d'un tel chemin n'est pas promise, et c'est dit.

### Ce que chaque sous-commande met dans `counts`

| Sous-commande | Clés de `counts` |
| :------------ | :--------------- |
| [`shift`](shift.md) | `subtitles`, `shifted_by_ms` (signé) |
| [`transform`](transform.md) | `subtitles` |
| [`framerate`](framerate.md) | `subtitles` |
| [`snap`](snap.md) | `subtitles`, `moved` (positions déplacées), `furthest_ms` (le plus grand déplacement) |
| [`hearing-impaired`](hearing-impaired.md) | `cleaned` (textes réécrits), `removed` (sous-titres supprimés) |
| [`replace`](replace.md) | `replaced` (correspondances remplacées dans les sous-titres changés), `matched` (correspondances trouvées) |
| [`case`](case.md), [`italics`](italics.md), [`dialogue-dashes`](dialogue-dashes.md) | `changed` (sous-titres dont le texte a changé) |
| [`correct`](correct.md) | `corrected` (textes changés), `removed` (sous-titres supprimés) — jamais des correspondances |
| [`sort`](sort.md) | `subtitles`, `moved` (les places qui ont changé de sous-titre) |
| [`adjust`](adjust.md) | `subtitles` (les sous-titres visés), `adjusted` (ceux dont la fin a bougé), puis le groupe `sacrificed` : `speed`, `minimum`, `gap` ; et, hors de `counts`, `constraints` — les réglages employés |
| [`convert`](convert.md) | `subtitles`, puis ce que la conversion a perdu : `lost_ends` et `lost_header` (0 ou 1), `joined_lines`, `lost_tags`, `lost_fields`, `furthest_ms` |
| [`append`](append.md) | `files`, `subtitles`, `appended`, puis `lost_ends`, `lost_header`, `joined_lines`, `lost_tags`, `lost_fields`, `furthest_ms` sommés ; et, hors de `counts`, `inputs` — un élément par fichier ajouté |
| [`split-file`](split-file.md) | `subtitles`, `head`, `tail` ; et, hors de `counts`, `tail` — le chemin de la queue (`destination` est la tête) |
| [`pair`](pair.md) | `subtitles` (ceux du fichier écrit) ; et, hors de `counts`, `alignment` |
| [`inspect`](inspect.md) | pas de `counts` : la description du fichier, voir sa page |

### Les identifiants d'erreur

Un `error.kind` est l'un de ceux-ci ; **un lecteur traite un identifiant inconnu comme
« autre »**.

| `kind` | Ce que c'est |
| :----- | :----------- |
| `not-found`, `permission-denied`, `io` | le système a refusé de lire — ou d'écrire — le fichier |
| `unknown-format`, `no-subtitle-found`, `undecodable` | le fichier est lu et ne se laisse pas lire |
| `unencodable` | un caractère que l'encodage choisi ne sait pas écrire |
| `no-byte-order-mark`, `no-frame-rate` | ce que `convert` ne peut pas écrire sans qu'on le précise |
| `before-the-origin`, `beyond-the-end`, `no-transform`, `no-grid` | une opération qui ne peut pas s'appliquer à ce fichier |
| `range-out-of-bounds` | une plage `--range` que ce fichier ne contient pas |
| `no-style` | un format qui n'écrit aucun style (TMPlayer, LRC), pour `italics` |
| `refused` | tout autre refus d'une opération |

### Ce qui est promis, et ce qui ne l'est pas

**Promis tant que `schema` vaut `1`** : les noms des clés, leur type et leur unité ; le sens
de `ok`, de `file` et de `destination` ; **un objet par entrée, dans l'ordre des entrées** — un par lancement pour les trois sous-commandes à arité propre ;
les identifiants `kind` qui existent ; le déterminisme — mêmes entrées, mêmes arguments,
mêmes octets.

**Pas promis** : le **texte des messages** (`message`, `detail`), de l'anglais pour un humain
qu'on corrige quand il le faut — un script qui le compare est cassé d'avance ; l'**ordre des
clés** dans un objet, fixe mais que JSON ne garantit pas ; la présence de **clés qu'on n'a pas
encore ajoutées**.

**Un lecteur ignore donc une clé inconnue**, et traite un `kind` inconnu comme « autre » :
ajouter une clé ou un identifiant ne change pas `schema`. Retirer ou renommer une clé,
changer un type ou une unité, retirer un identifiant l'incrémentent — c'est une rupture,
annoncée au CHANGELOG et ici.

## Niveaux de narration

Chaque niveau **contient le précédent** : monter d'un cran ajoute, ne remplace
jamais.

| Niveau | Comment | Ce que la sortie d'erreur porte |
| :----- | :------ | :------------------------------ |
| 0 | `--quiet` | rien, **sauf les erreurs** |
| 1 | par défaut, ou `-v` | une ligne par fichier traité, et un bilan dès qu'il y en a plusieurs |
| 2 | `-vv` | et ce qui a été reconnu : format, encodage, BOM, fins de ligne |
| 3 | `-vvv` | et la trace de mise au point : octets lus et écrits, et **chaque diagnostic de lecture** |

**Les erreurs ne sont jamais tues, `--quiet` compris.** Une commande qui échoue
en silence ne laisserait que son code de retour.

Le bilan n'apparaît qu'à partir de deux fichiers : sur une entrée unique, il
répéterait la ligne qui le précède.

## Les diagnostics de lecture

Les formats de sous-titres se lisent **au mieux** : devant une anomalie, le
lecteur ne s'arrête pas — il décide, ou il laisse en l'état, et il le dit. Ce
qu'il a rencontré sort au **niveau 3**, une ligne par diagnostic, sur la sortie
d'erreur :

```
a.srt: 2 diagnostics while reading
a.srt: line 6: SubRip numbers that do not follow ("7"), settled by the reader
a.srt: line 9: a line that fits nowhere, left as it stands
```

Les dix-sept sous-commandes les rapportent, pas seulement [`inspect`](inspect.md) :
un fichier lu au mieux puis réécrit a subi les mêmes décisions, et les taire
laisserait croire que rien ne s'est passé.

**Un diagnostic n'est jamais un échec.** Le fichier a été lu, la commande a
abouti, le code de retour est `0`. C'est pourquoi ils vivent au niveau le plus
détaillé : plus bas, ils enterreraient la ligne qui dit ce qui a réellement été
fait.

### Ce que chaque ligne porte

| Partie | Ce qu'elle dit |
| :----- | :------------- |
| `line N` | où, compté à partir de 1, comme un éditeur l'affiche ; **absent** pour ce qui porte sur le fichier entier |
| la phrase | ce qui a été rencontré, parmi les douze catégories ci-dessous |
| `("…")` | le texte fautif du fichier, quand la catégorie ne suffit pas ; tronqué à 80 octets |
| la fin | ce qui en a été fait : `settled by the reader`, ou `left as it stands` |

**Deux catégories n'ont pas de ligne**, et ce sont les deux qui portent sur
l'encodage : les octets ont été pesés, ou leur marque lue, avant qu'une seule
ligne du fichier existe. Leur ligne commence donc par la phrase, sans `line N`
devant — il n'y a pas de place à nommer.

**La fin de la ligne est le plus important.** `settled by the reader` veut dire
que le lecteur a tranché et que le fichier écrit porte sa décision — une
numérotation absente est régénérée. `left as it stands` veut dire qu'il n'a rien
touché parce que **vous seul pouvez décider** : un sous-titre qui finit avant de
commencer reste tel quel.

### Le numéro de ligne d'un bloc

Une anomalie qui porte sur **un bloc entier** est ancrée sur sa ligne
d'horodatage, et non sur la ligne exacte qui la déclenche. Une numérotation
incohérente écrite ligne 5 se rapporte donc ligne 6, celle de l'horodatage qui
suit — le numéro fautif est dans le `("…")`. Seules les anomalies qui portent sur
**une ligne** — un horodatage illisible, du texte avant le premier — se
rapportent sur elles-mêmes.

### Les douze catégories

| Phrase | Ce qui la déclenche |
| :----- | :------------------ |
| `a line that fits nowhere` | une ligne qui n'entre dans aucun bloc |
| `a timing line that could not be read` | une ligne d'horodatage illisible |
| `a SubRip block without its number` | numérotation absente, régénérée à l'écriture |
| `SubRip numbers that do not follow` | numérotation qui saute |
| `text before the first timing line` | du texte avant le premier horodatage |
| `a WebVTT block of an unknown kind` | un bloc WebVTT non reconnu |
| `declares an event column this tool cannot fill` | une colonne d'événement Sub Station Alpha que rien ici ne remplit |
| `counts in frames and states no rate; it was read at` | un fichier en images, qui n'énonce aucune fréquence — celle retenue est le détail |
| `carries no end times; each one was taken from the next start` | un format sans fin, TMPlayer ou LRC : **aucune fin de la table ne vient du fichier** |
| `more than one kind of line ending` | des fins de ligne mélangées |
| `an encoding nothing declared` | l'encodage a été proposé en pesant les octets, et il n'est pas de l'UTF-8 |
| `a byte order mark that contradicts the encoding asked for` | le fichier porte une marque, et `--encoding` en nommait un autre — la marque l'emporte |

**Les trois du milieu sont celles de la phase 9**, et les deux dernières de ces
trois-là ne parlent pas d'une ligne mais du fichier entier : elles disent qu'une
valeur affichée n'a pas été lue. Une colonne `End` remplie sur un LRC, une
position sur un MicroDVD — le fichier n'en portait rien.

**Une opération peut ajouter le sien**, qui n'est pas un diagnostic de lecture :
[`correct`](correct.md#un-motif-qui-ne-peut-pas-sappliquer) dit, avec le sous-titre, le motif
qui a renoncé. Il est dit au niveau 1 et porte, dans l'objet du fichier, l'identifiant
`pattern-failed` — jamais un échec du fichier.

## Une traduction

Un fichier de traduction se traite **comme un fichier de sous-titres quelconque** : `subedit-cli
case --to sentence film.fr.srt` le change sans rien de nouveau. Ce que l'appariement ajoute, et ce que
trois options disent, est étroit : **savoir comment les lignes d'une traduction tombent sur les
sous-titres du principal** — c'est `inspect -t` —, et **changer la traduction pendant qu'on la tient
posée sur son principal**, avec les mêmes règles que la fenêtre.

| Option | Où | Valeur | Défaut |
| :----- | :---- | :----- | :----- |
| `-t`, `--translation-file` | `inspect`, `hearing-impaired`, `replace`, `case`, `italics`, `dialogue-dashes`, `correct`, `pair` | le fichier de traduction | — |
| `--document` | les six sous-commandes de texte, pas `inspect` ni `pair` | `main` ou `translation` | `main` |
| `--align-method` | comme `-t` | `position` ou `number` | `position` |

- **`position`** compare le milieu de chaque ligne aux bornes des sous-titres : une ligne qui manque ne
  décale pas celles qui suivent. **`number`** envoie la ligne *n* au sous-titre *n*, sans regarder les
  positions. La position est le défaut, pour la raison que la fenêtre a : une ligne manquante au
  milieu fait glisser tout le reste d'une traduction appariée par numéro.
- **`--document translation` exige `-t`, et `-t` exige `--document translation`** sur une sous-commande
  de texte — l'un sans l'autre est une erreur d'usage, code `1`, parce qu'un fichier lu qu'aucun geste
  n'emploie est une omission et non une préférence. `-t` sur `inspect` n'a pas de `--document` : il
  rapporte. **`--align-method` sans `-t` est aussi une erreur d'usage.**
- **`-t` nomme un fichier, et n'a donc de sens que pour une seule entrée** : avec plusieurs entrées, ou
  avec un répertoire pris par `--recursive`, il est refusé comme `--output`, **avant qu'un fichier soit
  lu**. Pour un lot, on passe une invocation par paire.
- **Un fichier de traduction identique au principal est refusé** — quelle que soit la façon dont son
  chemin est écrit : `<traduction>: the file is already open as the main document`, code `2`.
- **Seul le document visé est écrit**, à **son propre chemin et dans son propre format** : le format,
  l'encodage et les fins de ligne du fichier de traduction. `--output` nomme le fichier de la
  traduction, `--output-dir` y met son nom de base, `--in-place` la réécrit ; **le fichier principal
  n'est jamais touché**. Les chemins du rapport sont ceux de la traduction.
- **Les sous-titres que l'appariement fait naître** — une ligne que rien n'a atteint — sont écrits dans
  la traduction avec **leurs propres positions**, quelle que soit la méthode ; **un sous-titre que
  personne n'a traduit s'écrit comme un bloc sans texte**, qui se lit sans anomalie.
- **`--encoding` et `--frame-rate` valent pour les deux fichiers.**
- Les sous-commandes de **position** — `shift`, `transform`, `framerate`, `snap`, `adjust` — n'ont pas
  ces options : on leur donne les deux fichiers, comme à un lot, `subedit-cli shift --by 2 film.srt
  film.fr.srt --output-dir out/`.
- **Écrire la traduction recalée sur le principal** — la seule chose que l'appariement fait et qu'un fichier seul
  ne fait pas — est le travail de [`pair`](pair.md), qui n'a ni `--document` ni `--recursive`.

<!-- exemple: printf '1\n00:00:01,000 --> 00:00:02,000\nHello.\n\n2\n00:00:03,000 --> 00:00:04,000\nBye.\n\n' > film.srt; printf '1\n00:00:01,000 --> 00:00:02,000\n[Un oiseau] Bonjour.\n\n2\n00:00:03,000 --> 00:00:04,000\nAu revoir.\n\n' > film.fr.srt; subedit-cli hearing-impaired film.srt -t film.fr.srt --document translation --output propre.fr.srt; cat propre.fr.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nHello.\n\n2\n00:00:03,000 --> 00:00:04,000\nBye.\n\n' > film.srt; printf '1\n00:00:01,000 --> 00:00:02,000\n[Un oiseau] Bonjour.\n\n2\n00:00:03,000 --> 00:00:04,000\nAu revoir.\n\n' > film.fr.srt; subedit-cli hearing-impaired film.srt -t film.fr.srt --document translation --output propre.fr.srt; cat propre.fr.srt
film.fr.srt: 1 subtitle cleaned, 0 removed -> propre.fr.srt
1
00:00:01,000 --> 00:00:02,000
Bonjour.

2
00:00:03,000 --> 00:00:04,000
Au revoir.
```

Avec `--format json`, l'objet de chaque fichier porte en plus la clé `alignment`, qui dit comment la
traduction a été posée — `{"file", "method", "attached", "born", "untranslated", "out_of_order"}`, comme
la clé `translation` de [`inspect`](inspect.md#en-json) — et, **avec `--dry-run`**, `changes` liste
les sous-titres de la traduction, chacun avec `"document":"translation"`.

## Plusieurs fichiers

Toutes les sous-commandes acceptent plusieurs chemins, sauf [`append`](append.md), [`split-file`](split-file.md) et
[`pair`](pair.md), dont l'arité est celle de leur page. Chacun est traité
indépendamment : l'échec de l'un n'interrompt pas les autres, et les échecs
sont rapportés en nommant le fichier et la raison.

**Un répertoire vaut les fichiers qu'il contient, avec `--recursive` (`-r`)** :
sans cette option, en donner un est une erreur d'usage, code `1`, et rien n'est
traité — le refuser vaut mieux que de n'en traiter rien et de rendre `0` :

<!-- exemple: mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; subedit-cli inspect films; echo "code=$?" -->
```console
$ mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; subedit-cli inspect films; echo "code=$?"
films: is a directory: use --recursive
code=1
```

Le parcours, l'arborescence conservée sous `--output-dir` et ce qui est laissé
de côté sont décrits dans [Traiter un arbre](lots.md).

## La page de manuel

Une installation dépose `subedit-cli(1)` sous `share/man/man1`, à côté de
`subedit-gui(1)` :

```console
$ man subedit-cli
```

Elle tient en une page ce que ce manuel dit en dix : la forme d'appel, ce que
chaque sous-commande fait, les options globales, les codes de retour — puis où
lire le reste. **Elle est courte délibérément** : la recopier entière ici en
produirait une seconde version à tenir, qui divergerait.

## Codes de retour

| Code | Signification |
| :--- | :------------ |
| `0` | tout a réussi |
| `1` | erreur d'usage — option inconnue, valeur invalide, combinaison interdite |
| `2` | aucun fichier n'a pu être traité |
| `3` | certains fichiers ont été traités, d'autres non |

`2` et `3` sont distingués pour qu'un script sache agir sans relire la sortie :
« rien n'a marché » et « il en manque un » n'appellent pas la même réaction.

Une erreur d'usage est détectée **avant tout traitement** : elle ne laisse
jamais un lot à moitié traité.

## Erreurs

| Ce qui la déclenche | Ce qui est écrit, sur la sortie d'erreur |
| :------------------ | :--------------------------------------- |
| option ou sous-commande inconnue | le nom fautif, et un renvoi à `--help` |
| `--quiet` avec `-v` | `--quiet and -v ask for opposite things; give one or the other` |
| fichier absent | `<chemin>: does not exist` |
| fichier illisible | `<chemin>: cannot be opened: permission denied` |
| encodage qui écrit sa propre marque | `"<nom>" writes a byte order mark of its own; name the byte order, as UTF-16LE and UTF-16BE do` |
| octets qui ne se décodent pas | `<chemin>: cannot be decoded in the chosen encoding` |
| format non reconnu | `<chemin>: is in no format this tool knows` |
| rien qui ressemble à un sous-titre | `<chemin>: holds nothing recognisable as a subtitle` |
| `--document translation` sans `-t` | `--document translation needs the translation file: use -t` |
| `-t` sans `--document translation` | `-t names a translation to change: use --document translation` |
| `--align-method` sans `-t` | `--align-method needs a translation file: use -t` |
| `-t` avec plusieurs entrées, ou un répertoire | `-t names one file but several inputs were given: use one invocation per pair` |
| traduction identique au principal | `<traduction>: the file is already open as the main document` |
