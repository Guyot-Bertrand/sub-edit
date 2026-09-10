# Ouvrir et enregistrer

Le menu **File** porte les trois commandes.

| Commande | Raccourci | Ce qu'elle fait |
| :------- | :-------- | :-------------- |
| `Open…` | `Ctrl+O` | choisit un fichier et l'ouvre à la place du courant |
| `Save` | `Ctrl+S` | réécrit le fichier ouvert |
| `Save As…` | `Ctrl+Maj+S` | choisit un chemin et un format, puis écrit |

`Open…` et `Save` sont aussi dans la barre d'outils.

## Ouvrir

Le dialogue filtre **les neuf formats lus**, une entrée chacun, et une première
entrée qui les montre tous. Le format est ensuite reconnu **au contenu, pas à
l'extension** : un `.txt` qui contient du SubRip s'ouvre, et le filtre ne décide
rien — il ne fait que cacher des fichiers.

| Entrée du filtre | Extension |
| :--------------- | :-------- |
| `SubRip` | `.srt` |
| `WebVTT` | `.vtt` |
| `SubViewer 2` | `.sub` |
| `Sub Station Alpha` | `.ssa` |
| `Advanced SSA` | `.ass` |
| `MicroDVD` | `.sub` |
| `MPL2` | `.txt` |
| `TMPlayer` | `.txt` |
| `LRC` | `.lrc` |

**Deux extensions désignent deux formats chacune** — `.sub` est SubViewer 2 et
MicroDVD, `.txt` est MPL2 et TMPlayer. C'est sans conséquence à l'ouverture,
puisque c'est le contenu qui tranche ; à l'enregistrement, c'est le **nom** de
l'entrée choisie qui dit le format, jamais son motif.

**Un fichier illisible ne remplace rien.** Absent, refusé par le système, écrit
dans un encodage sous lequel ses octets ne se décodent pas, ou d'aucun format
connu : une modale **nomme la cause**, et la fenêtre garde ce qu'elle avait,
sous-titres, historique et modifications comprises.

| Message | Ce qui s'est passé |
| :------ | :----------------- |
| `does not exist` | le chemin ne désigne aucun fichier |
| `cannot be opened: permission denied` | le système refuse de l'ouvrir |
| `cannot be read` | le système a refusé pour une autre raison |
| `cannot be decoded in the chosen encoding` | les octets ne se lisent pas dans l'encodage retenu |
| `is in no format this tool knows` | aucun format ne reconnaît le contenu |
| `holds nothing recognisable as a subtitle` | le format est reconnu, mais rien n'y est un sous-titre |

Le message est précédé du chemin : `notes.txt: is in no format this tool knows`.
Ce sont les mots de la ligne de commande, et ce n'est pas un hasard — les deux
surfaces lisent un fichier par la même recette.

Ouvrir depuis la ligne de commande fonctionne toujours : `subedit-gui film.srt`.
Là, la même raison est écrite sur la sortie d'erreur, et la fenêtre s'ouvre
vide — voir [Invocation](invocation.md#quand-louverture-échoue).

## Les diagnostics d'une lecture

Un fichier réel est rarement parfait, et la lecture s'en remet plutôt que
d'abandonner : un numéro absent, une ligne qui ne va nulle part, des fins de
ligne mélangées. **Ce qu'elle a rencontré s'affiche sous la table**, dans un
panneau replié qui s'ouvre d'un clic.

```
line 5: a SubRip block without its number, settled by the reader
```

Chaque ligne porte **le numéro de ligne du fichier** — celui qu'un éditeur de
texte montrerait —, ce qui a été rencontré, et ce qui en a été fait. Un extrait
du fichier suit entre guillemets quand il apporte quelque chose ; il est tronqué
au-delà de quatre-vingts caractères.

**Une seule ligne n'a pas de numéro**, celle de l'encodage : il a été proposé en
pesant les octets, avant qu'une seule ligne du fichier existe.

```
an encoding nothing declared ("windows-1252"), settled by the reader
```

Elle ne s'affiche que lorsque l'encodage a été **deviné et n'est pas de
l'UTF-8** — un fichier UTF-8 lu comme tel n'est pas un événement, et le dire à
chaque ouverture mettrait un panneau sous la table de tous les documents
ordinaires. Un fichier qui porte une marque d'ordre des octets ne la déclenche
pas non plus : il a déclaré son encodage, rien n'a été deviné.

Le panneau **n'apparaît pas** quand la lecture n'a rien à signaler.

## L'encodage dans la barre d'état

**La barre d'état dit toujours l'encodage du document ouvert**, à gauche de ce
qu'elle dit de la [grille d'images](grille.md) et de la
[vidéo associée](video.md) :

| Situation | Ce qui est écrit |
| :-------- | :--------------- |
| le fichier a été lu en UTF-8, sans marque | `Encoding: UTF-8, no BOM` |
| … avec une marque | `Encoding: UTF-8, BOM` |
| un fichier en Latin-1 | `Encoding: ISO-8859-1, no BOM` |

**Le diagnostic et cette ligne ne répondent pas à la même question**, et c'est
pourquoi les deux existent. Le diagnostic dit ce que la lecture **a fait** — elle
a deviné, et vous voudrez peut-être vérifier ; il ne s'affiche donc que quand il
y a eu une devinette. La barre d'état dit ce que le document **est**, en
permanence, y compris pour un fichier qui déclare son encodage lui-même et sur
lequel la lecture n'a rien à raconter.

**La provenance de la réponse n'y figure pas.** `inspect` la donne — `detected`,
`from its byte order mark`, `as asked for` — parce qu'un rapport a la place de
la dire ; une ligne de barre d'état, non, et le panneau la porte déjà quand elle
compte.

## Enregistrer

`Save` réécrit le fichier ouvert **dans sa forme d'origine** : son format, son
encodage, ses fins de ligne, sa marque d'ordre des octets et son en-tête. Un
fichier ouvert puis enregistré sans modification ne bouge pas d'un octet — voir
la garantie exacte plus bas.

L'écriture est **atomique** : elle passe par un fichier temporaire renommé
par-dessus. Une sauvegarde interrompue à n'importe quel moment laisse la version
précédente intacte.

Enregistrer un document qui n'a jamais été sur disque revient à `Save As…`.

**Une écriture qui échoue le dit et ne perd rien** : le disque plein, un fichier
en lecture seule, un répertoire absent — ou **un caractère que l'encodage du
fichier ne sait pas écrire**, un `ł` dans un fichier en Latin-1. Le message est
alors `holds a character the chosen encoding cannot write`, et rien n'est
écrit : remplacer le caractère par un `?` perdrait du texte sous les yeux de qui
vient de le taper. Le message nomme le fichier et la raison, la fenêtre garde
ses modifications, et la marque du titre reste.

## Enregistrer sous

`Save As…` demande un chemin, un format, **et la forme des octets écrits** :
l'encodage, les fins de ligne, la marque d'ordre des octets. Le document **vit
ensuite là** : le titre change, `Save` vise le nouveau fichier et écrit dans la
forme choisie, et le fichier d'origine reste tel qu'il était.

**Les neuf formats sont proposés**, et l'entrée sélectionnée à l'ouverture de la
boîte est **celle du document**. Enregistrer sous un autre nom sans toucher au
filtre ne change donc pas de format.

![Le bas de la boîte « Save As… », palette claire : le nom du fichier et son
format, puis l'encodage, les fins de ligne et la marque, sur les mêmes
colonnes.](captures/enregistrer-sous.png)

![Le même bandeau sous la palette sombre.](captures/enregistrer-sous-sombre.png)

**Les images ne montrent que le bas de la boîte**, et pas la liste des fichiers
au-dessus : celle-ci dépend de la machine — ses répertoires, ses dates, ses
raccourcis latéraux —, donc sa photographie ne serait jamais deux fois la même.
Ce bandeau-ci l'est, et c'est là que tout se joue.

| Champ | Ce qu'il propose | Défaut |
| :---- | :--------------- | :----- |
| `Encoding` | quatorze encodages qu'un fichier de sous-titres porte en pratique, plus `Other…` | **celui du fichier lu** |
| `Other…` | un champ où taper tout encodage qu'ICU sait écrire, **avec complétion** — `cp1257`, `EUC-KR` | — |
| `Line endings` | `LF`, `CRLF` ou `CR` | celles du fichier lu |
| `Byte order mark` | la marque, **éteinte pour un encodage qui n'en porte pas** | celle du fichier lu |

**Les défauts sont ceux du fichier lu, et c'est une garantie plutôt qu'une
commodité** : un fichier ouvert puis réenregistré sans qu'on touche à ces trois
champs rend les mêmes octets.

**La liste est courte, et ce n'est pas un plafond.** ICU en connaît deux cent
vingt et un ; ce que le menu propose est ce qu'un fichier de sous-titres porte
en pratique, parce qu'un menu de deux cents entrées n'aide personne. `Other…`
ouvre le reste.

**Le champ complète sur les deux cent vingt et un**, donc il n'y a pas à
connaître le nom exact : taper `1252` propose `windows-1252`, taper `koi`
propose `KOI8-R` et `KOI8-U`. La complétion porte sur ce que le nom **contient**
et non sur ce par quoi il commence — le nom canonique de Windows Occidental est
`windows-1252`, et `cp1252` est celui qu'on a en tête. Les deux s'écrivent
d'ailleurs en entier sans complétion : ICU accepte ses propres alias.

**Un nom que personne ne connaît n'écrit rien.** Taper `klingon-1` ne fait pas
retomber sur l'UTF-8 : rien n'est écrit, et le message le dit.

**Un nom qui ne dit pas son ordre d'octets non plus.** `UTF-16` et `UTF-32` sont
des noms qu'ICU connaît, dont le convertisseur écrit **sa propre marque** : la
case `Byte order mark` cesserait alors de décider quoi que ce soit. Les deux
sont refusés, et le message envoie vers `UTF-16LE` ou `UTF-16BE`, qui écrivent
les mêmes octets sous le contrôle de la case. C'est le même refus, dans les
mêmes mots, que celui de la ligne de commande.

**Un caractère que l'encodage choisi ne sait pas écrire arrête l'enregistrement**
— un `ł` dans du Latin-1. Le message est
`holds a character the chosen encoding cannot write`, la fenêtre garde ses
modifications, et le fichier visé n'est pas touché.

**Le document ne déménage pas non plus** : il reste sur son fichier, dans son
format et son encodage, et le `Save` suivant réécrit celui qu'on avait ouvert.
Un enregistrement qui n'a pas eu lieu ne change rien du tout — ni sur le disque,
ni dans la fenêtre.

Changer de format change ce que la table montre — le séparateur décimal suit le
format, virgule pour SubRip, point pour WebVTT, et **les balises sont réécrites
dans le vocabulaire du format d'arrivée** : un `<i>` devient `{\i1}` en Sub
Station Alpha, `{Y:i}` en MicroDVD, `/` en tête de ligne en MPL2. Le document
**est devenu** ce fichier, donc la table montre ce que le fichier porte.

**Ce n'est pas une commande, et `Ctrl+Z` ne le défait pas.** Un document dont le
format dirait LRC et dont les textes diraient encore `<i>` serait un document
incohérent ; il n'y a pas de moitié à annuler.

## Ce qu'un format ne portera pas, dit avant d'écrire

**Une modale prévient, et on peut encore répondre non.** Elle liste ce que le
format d'arrivée ne tiendra pas, poste par poste, dans les mots que la ligne de
commande écrit après coup :

```
ends are not carried by LRC, line breaks were joined in 2 subtitles, 1 tag dropped
```

| Bouton | Effet |
| :----- | :---- |
| `Save` | écrit quand même, la perte assumée |
| `Cancel` | n'écrit rien ; le document ne bouge pas, ni de fichier, ni de format |

`Cancel` est le bouton par défaut : une touche Entrée pressée sans lire garde le
fichier tel qu'il est.

**Rien n'est demandé quand rien n'est perdu.** Une boîte qui annoncerait « rien
ne sera perdu » apprendrait à congédier celle qui compte.

Ce qui peut être annoncé :

| Poste | Quand |
| :---- | :---- |
| les fins | le format d'arrivée n'en porte pas — TMPlayer, LRC |
| les sauts de ligne | le format d'arrivée n'en porte pas — LRC |
| les balises | une balise du départ n'a pas d'équivalent à l'arrivée, ou le pivot ne la connaît pas |
| l'en-tête | le fichier lu en portait un, et il ne traverse pas |
| les champs déclarés | l'ordre des colonnes SSA, la forme de l'heure TMPlayer |
| la précision | le format d'arrivée compte plus gros — le dixième de MPL2, la seconde de TMPlayer |

**Ce qui appartient à l'autre format est laissé de côté.** Un fichier WebVTT
porte des identifiants de cue et des réglages de placement ; SubRip porte des
coordonnées d'affichage. Écrire dans l'autre format les ignore plutôt que de les
traduire au hasard — ils ne sont pas perdus pour autant, ils ne sont simplement
pas écrits.

## La garantie d'aller-retour

Elle a deux moitiés, et il faut les deux :

- **fidèle octet pour octet** sur un fichier déjà dans la disposition que
  `subedit` écrit — celle de Gaupol, ligne vide finale comprise —, et **quel
  que soit son encodage** : un fichier en CP1252 ou en UTF-16 est réécrit dans
  le sien, marque comprise ;
- **idempotent** sinon : le premier enregistrement normalise la disposition,
  aucun ensuite ne touche plus rien.

Concrètement, un fichier dont le dernier bloc n'est pas suivi d'une ligne vide en
gagne une, une fois. C'est le seul changement qu'une sauvegarde sans
modification peut produire.

## Les modifications non enregistrées

Fermer la fenêtre ou ouvrir un autre fichier alors que le document diffère de
celui du disque **demande confirmation**, avec trois issues :

| Réponse | Ce qui se passe |
| :------ | :-------------- |
| `Save` | le document est écrit, puis la fenêtre se ferme ou ouvre l'autre fichier |
| `Discard` | les modifications sont perdues, et l'action se poursuit |
| `Cancel` | rien ne se passe ; la fenêtre reste comme elle était |

Fermer la boîte sans répondre vaut `Cancel`.

**Si l'enregistrement échoue**, l'action ne se poursuit pas : le travail n'est ni
écrit ni perdu.
