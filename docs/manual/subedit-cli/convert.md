# `convert`

```
subedit-cli convert --to <format>
                    [--line-endings unix|windows|mac] [--to-encoding NOM]
                    [--bom | --no-bom]
                    (--output FICHIER | --output-dir DOSSIER | --in-place)
                    <fichier>...
```

Écrit chaque fichier dans le format et la forme demandés. **Ne modifie jamais
l'entrée**, sauf si `--in-place` le demande explicitement.

<!-- exemple: subedit-cli convert --help -->
```console
$ subedit-cli convert --help
Write a subtitle file out in another format or shape
Usage: subedit-cli convert [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to convert

Options:
  -h,--help                   Print this help message and exit
  --to TEXT:{srt,vtt,subviewer2,ssa,ass,mpl2,microdvd,tmplayer,lrc} REQUIRED
                              Format to write
  --line-endings TEXT:{unix,windows,mac}
                              Line endings to write; the source's by default
  --to-encoding NAME          Encoding to write; the source's by default
  --frame-rate RATE           Frame rate of a file counted in frames: 25, 23.976
  --bom                       Write a byte order mark
  --no-bom                    Write no byte order mark
  --output TEXT               File to write, for a single input
  --output-dir TEXT           Directory to write into
  --in-place                  Write back over the inputs
```

## Arguments et options

| Option | Requis | Valeurs | Défaut |
| :----- | :----- | :------ | :----- |
| `<fichier>...` | oui | un ou plusieurs chemins | — |
| `--to` | **oui** | un nom de la table ci-dessous, et rien d'autre | — |
| `--line-endings` | non | `unix`, `windows` ou `mac` | celles du fichier lu |
| `--to-encoding` | non | tout encodage qu'ICU sait écrire, sauf ceux qui écrivent leur propre marque | celui du fichier lu |
| `--bom` / `--no-bom` | non | drapeaux, exclusifs l'un de l'autre | ce que portait le fichier lu |
| `--output` / `--output-dir` / `--in-place` | l'une des trois | voir [Invocation](invocation.md#la-destination) | — |

`mac` désigne le retour chariot seul (`\r`), la fin de ligne du Mac OS classique.

## Les formats que `--to` accepte

| Nom | Format | Extension écrite |
| :-- | :----- | :--------------- |
| `srt` | SubRip | `.srt` |
| `vtt` | WebVTT | `.vtt` |
| `subviewer2` | SubViewer 2.0 | `.sub` |
| `ssa` | Sub Station Alpha | `.ssa` |
| `ass` | Advanced SSA | `.ass` |
| `mpl2` | MPL2 | `.txt` |
| `microdvd` | MicroDVD | `.sub` |
| `tmplayer` | TMPlayer | `.txt` |
| `lrc` | LRC | `.lrc` |

**Un nom, et pas une extension.** Deux extensions désignent deux formats
chacune — `.sub` est aussi celle de MicroDVD, `.txt` celle de MPL2 et de
TMPlayer — donc une option qui prendrait des extensions ne saurait pas lequel
est demandé. Les noms ci-dessus sont sans ambiguïté, et coïncident avec
l'extension partout où celle-ci l'est aussi.

**La liste est close.** C'est celle de Gaupol, et les neuf y sont.

## Les deux formats qui ne portent pas de fin

**TMPlayer et LRC n'écrivent qu'une position par réplique**, son début. Cela se
lit des deux côtés.

**En lecture**, les fins sont déduites : celle d'une réplique est le début de
la suivante, et la dernière reçoit cinq secondes. C'est ce que fait Gaupol.
Nous le disons en plus, par un diagnostic que `--verbose` affiche sur les trois
sous-commandes :

```
fichier.lrc: carries no end times; each one was taken from the next start, settled by the reader
```

**En écriture**, les fins ne sont pas écrites, faute d'endroit où les mettre.
Convertir vers l'un de ces deux formats les perd donc, et un aller-retour ne
les retrouve pas — il les redéduit.

**LRC perd une chose de plus** : il n'a aucun moyen de porter un saut de ligne.
Une réplique de deux lignes s'écrit sur une seule, les deux lignes jointes par
une espace. Cette perte-là ne se répare pas non plus.

Ni l'un ni l'autre n'a de vocabulaire pour l'italique.

## `--frame-rate`, et le seul format qui compte en images

**MicroDVD ne compte pas en secondes** : ses positions *sont* des numéros
d'image, et aucun fichier MicroDVD ne déclare la fréquence à laquelle ils ont
été comptés. `--frame-rate` la donne, et sert des deux côtés : elle est la
fréquence à laquelle un `.sub` en images est lu, et celle à laquelle une
conversion vers MicroDVD compte les images.

| Option | Requis | Valeurs | Défaut |
| :----- | :----- | :------ | :----- |
| `--frame-rate` | non | une fréquence en images par seconde : `25`, `23.976`, `29.97` | voir ci-dessous |

**À la lecture, sans `--frame-rate`, le fichier est lu à 23,976** — la valeur de
Gaupol — et le niveau `-vvv` le dit : `counts in frames and states no rate; it
was read at`. C'est le seul endroit de l'outil où toutes les positions affichées
reposent sur une hypothèse que le fichier ne peut pas confirmer.

**À l'écriture, sans `--frame-rate`, trois cas et trois réponses :**

| Le fichier de départ | Ce qui se passe |
| :------------------- | :-------------- |
| est déjà du MicroDVD | la fréquence qui l'a lu le réécrit, et les images reviennent identiques |
| est temporel, et ses positions tombent sur une grille | la grille est prise, et `-vv` dit laquelle |
| est temporel, sans grille | **refus**, code 2 : `writing frames needs a frame rate, and the positions fall on no grid to take one from — give --frame-rate` |

Le refus est délibéré. Le seul geste restant serait d'inventer un chiffre, et
chaque réplique du fichier bougerait de ce qu'il aurait de faux.

## Ce qu'une conversion perd

**Convertir vers un autre format n'est jamais sans perte**, et ce qui ne survit
pas dépend de la paire :

| Ce qui ne traverse pas | Pourquoi |
| :--------------------- | :------- |
| l'en-tête | il n'a de sens que dans son format : `[INFORMATION]` n'est pas une en-tête WebVTT |
| les données propres au format | les coordonnées de SubRip, l'identifiant et les réglages d'une cellule WebVTT n'ont pas d'équivalent ailleurs |
| la précision | SubViewer 2, les deux Sub Station Alpha et LRC écrivent au centième, MPL2 au dixième, TMPlayer à la seconde, MicroDVD à l'image ; `01:00:00,017` en revient à `01:00:00,020` |
| la fin d'une réplique | TMPlayer et LRC n'ont pas de champ pour elle ; la lecture suivante la redéduit |
| les sauts de ligne | LRC seul : ses répliques tiennent sur une ligne, et les deux sont recollées par une espace |
| les balises que le format d'arrivée ne sait pas écrire | WebVTT n'a pas de `<font color>`, TMPlayer et LRC n'ont rien du tout |
| la mise en page | `{\pos(x,y)}`, `{\an8}`, `<v Marie>`, `<ruby>` : ils traversent un aller-retour dans leur propre format, et disparaissent à la conversion |

**Réécrire un fichier dans son propre format ne perd rien** — c'est la garantie
qui tient tout le reste, et `--to srt` sur un `.srt` rend les mêmes octets. Rien
n'y est même décodé : le texte reste la chaîne brute qui a été lue.

## Les balises sont traduites, pas recopiées

**Chaque format écrit l'italique à sa façon**, et convertir sans traduire
laisserait un `{\i1}` dans un `.srt` : du texte que l'utilisateur voit, dans un
fichier qui ne saura jamais l'interpréter. Ce qui traverse, ce sont **six
choses** — gras, italique, souligné, couleur, police et taille :

| Format | Gras | Italique | Souligné | Couleur | Police | Taille |
| :----- | :--: | :------: | :------: | :-----: | :----: | :----: |
| SubRip | `<b>` | `<i>` | `<u>` | `<font color="#RRGGBB">` | — | — |
| WebVTT | `<b>` | `<i>` | `<u>` | — | — | — |
| SubViewer 2 | `<b>` | `<i>` | `<u>` | `<font color="#RRGGBB">` | — | — |
| Sub Station Alpha | `{\b1}` | `{\i1}` | — | `{\c&HBBGGRR&}` | `{\fnNOM}` | `{\fsN}` |
| Advanced SSA | `{\b1}` | `{\i1}` | `{\u1}` | `{\c&HBBGGRR&}` | `{\fnNOM}` | `{\fsN}` |
| MicroDVD | `{Y:b}` | `{Y:i}` | `{Y:u}` | `{C:$BBGGRR}` | `{F:NOM}` | `{S:N}` |
| MPL2 | `\` | `/` | `_` | `{C:$BBGGRR}` | `{F:NOM}` | `{S:N}` |
| TMPlayer | — | — | — | — | — | — |
| LRC | — | — | — | — | — | — |

**Un tiret est une perte annoncée.** Convertir un `.srt` coloré en WebVTT retire
la couleur et le dit ; le fichier produit est du WebVTT, pas du SubRip déguisé.

**MicroDVD et MPL2 ne savent pas styler un morceau de ligne.** Leurs balises
n'ont pas de fin : `{y:i}` penche tout ce qui suit jusqu'au bout de la ligne, et
`{Y:i}` jusqu'au bout de la réplique. Une ligne dont une moitié seulement était
en gras s'écrit donc sans gras du tout, plutôt qu'avec un gras qui déborde.

## Ce que la conversion dit qu'elle a perdu

**Elle le dit quand elle perd, et se tait quand elle ne perd rien.** Un rapport
qui s'affiche à chaque appel est un rapport que personne ne lit.

<!-- exemple: printf '1\n00:00:01,000 --> 00:00:03,500\n<i>sur deux</i>\nlignes\n\n' > a.srt; subedit-cli convert --to lrc --output b.lrc a.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,500\n<i>sur deux</i>\nlignes\n\n' > a.srt; subedit-cli convert --to lrc --output b.lrc a.srt
a.srt: 1 subtitle written as LRC -> b.lrc
a.srt: ends are not carried by LRC, line breaks were joined in 1 subtitle, 1 tag dropped
```

La ligne paraît au niveau de bavardage par défaut, et à tous ceux au-dessus.
Les postes, dans cet ordre :

| Poste | Ce qu'il dit |
| :---- | :----------- |
| `ends are not carried by <format>` | le format d'arrivée n'écrit aucune fin |
| `line breaks were joined in N subtitles` | LRC seul : N répliques tenaient sur plusieurs lignes |
| `N tags dropped` | N balises n'ont pas su être écrites, mise en page comprise |
| `the header was dropped` | l'en-tête ne traverse pas une frontière de format |
| `the <format> fields of N subtitles were dropped` | N répliques portaient des données propres à leur format |
| `positions moved by up to N ms` | la plus grande distance dont une position a bougé |

## Changer d'encodage

**`--to-encoding` choisit l'encodage écrit ; `--encoding` dit celui qui est
lu.** Les deux ensemble réencodent un fichier :

```console
$ subedit-cli convert --to srt --encoding cp1252 --to-encoding utf-8 \
      --output-dir sortie film.srt
```

Sans `--to-encoding`, le fichier est réécrit dans **l'encodage où il a été lu**,
comme il l'est avec ses fins de ligne et sa marque : un fichier converti sans
consigne d'encodage rend les mêmes caractères, aux mêmes octets.

**Un caractère que l'encodage cible ne sait pas écrire arrête le fichier**, et
rien n'est écrit. Un `ł` n'a pas de place en Latin-1 ; le remplacer par un `?`
perdrait du texte entre la lecture et l'écriture, sans que personne le voie.

**Une marque d'ordre des octets n'existe que pour les encodages Unicode.**
`--bom` avec un `--to-encoding` qui n'en porte pas est refusé plutôt qu'ignoré
— voir la table des erreurs ci-dessous.

**Et l'encodage doit nommer son ordre d'octets.** `UTF-16` et `UTF-32` sont des
noms qu'ICU connaît, dont le convertisseur écrit **sa propre marque**, quoi
qu'on lui demande : `--no-bom` serait alors accepté et désobéi. Les deux sont
donc refusés, et le message nomme la sortie :

```console
$ subedit-cli convert --to srt --to-encoding UTF-16 --output copie.srt film.srt
"UTF-16" writes a byte order mark of its own; name the byte order, as UTF-16LE and UTF-16BE do
```

`UTF-16LE`, `UTF-16BE`, `UTF-32LE` et `UTF-32BE` écrivent exactement les mêmes
octets, marque comprise — mais sous le contrôle de `--bom` et de `--no-bom`. Le
refus ne coûte donc rien d'autre qu'un nom plus précis, et il vaut pour la
lecture comme pour l'écriture : `--encoding UTF-16` reçoit la même réponse.

## La destination

Les trois options et leurs règles sont communes aux six sous-commandes qui
écrivent : voir [Invocation](invocation.md#la-destination). Rien n'est écrit
sans l'une d'elles.

Ce qui est propre à `convert` : **`--output-dir` change l'extension avec le
format.** `convert --to vtt` sur `a.srt` écrit `a.vtt`, jamais `a.srt` contenant
du WebVTT — un fichier dont le nom ment fait trébucher tous les autres outils.

## Convertir sur place est refusé

`--in-place` avec un `--to` qui change le format donne le code `1` et n'écrit
rien : sur place, il n'y a pas de second nom pour porter le nouveau format, et
le fichier resterait sous une extension que son contenu ne justifie plus.

`--in-place` reste utilisable pour changer **la forme sans le format** —
réécrire un `.srt` en `.srt` avec d'autres fins de ligne, par exemple.

Le refus se décide sur **l'extension seule**, avant toute lecture : une erreur
d'usage ne doit jamais laisser un lot à moitié écrit.

## Fins de ligne et marque d'ordre des octets

Par défaut, **le fichier écrit reprend ce que portait le fichier lu**. Le modèle
retient les deux à la lecture ; les imposer par défaut perdrait à chaque
conversion une information conservée exprès.

<!-- exemple: printf '1\n00:00:01,000 --> 00:00:03,000\nBonjour.\n\n' > a.srt; subedit-cli convert --to vtt --output b.vtt a.srt; cat b.vtt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nBonjour.\n\n' > a.srt; subedit-cli convert --to vtt --output b.vtt a.srt; cat b.vtt
a.srt: 1 subtitle written as WebVTT -> b.vtt
WEBVTT

00:01.000 --> 00:03.000
Bonjour.
```

## Sortie

**Sortie standard** — rien : le résultat est le fichier écrit.

Sur la sortie d'erreur, selon le niveau :

| Niveau | Ce qui s'ajoute |
| :----- | :-------------- |
| 1 | `<chemin>: N subtitles written as WebVTT -> <destination>` |
| 2 | `<chemin>: SubRip -> WebVTT, UTF-8, no BOM, LF line endings` — l'encodage nommé est celui qui est écrit |
| 3 | `<chemin>: N bytes read, M written`, puis **chaque diagnostic de lecture** — voir [Invocation](invocation.md#les-diagnostics-de-lecture) |

## Codes de retour

Ceux de l'outil : `0` si tous les fichiers ont été écrits, `2` si aucun, `3` si
certains seulement, `1` sur une erreur d'usage.

## Erreurs

| Ce qui la déclenche | Message |
| :------------------ | :------ |
| `--bom` avec `--no-bom` | `--bom and --no-bom ask for opposite things; give one or the other` |
| `--in-place` qui change le format | `--in-place cannot change the format: the file would keep a name its content no longer matches` |
| `--bom` sur un encodage qui n'en a pas | `<chemin>: <encodage> has no byte order mark to write` |
| `--to-encoding` nommant un encodage inconnu | `no encoding is named "<nom>"` |
| `--to-encoding` nommant un encodage qui écrit sa propre marque | `"<nom>" writes a byte order mark of its own; name the byte order, as UTF-16LE and UTF-16BE do` |
| caractère absent de l'encodage écrit | `<chemin>: holds a character the chosen encoding cannot write` |

**Le refus de `--bom` sur un encodage qui n'en porte pas n'est pas une
pédanterie.** Une marque d'ordre des octets existe pour les encodages Unicode et
pour aucun autre : la demander sur un fichier en Windows-1252, c'est demander
quelque chose qui n'existe pas. Le fichier s'écrirait sans elle, et personne ne
saurait que la demande n'a pas été honorée.

Celles de la destination sont communes aux sous-commandes qui écrivent :
voir [Invocation](invocation.md#la-destination).

Les erreurs de lecture sont celles d'[`inspect`](inspect.md) : elles portent sur
le fichier d'entrée et ont les mêmes messages.
