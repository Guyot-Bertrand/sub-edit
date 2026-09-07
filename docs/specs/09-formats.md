# Phase 9 — Formats complémentaires et balises riches

Cadrage, issue #337. Ce document dit ce que la phase livre, ce qu'elle ne livre
pas, et pourquoi.

## Ce que la feuille de route promet, et ce qui est déjà là

> SubViewer 2, Sub Station Alpha, Advanced SSA — les trois formats cités comme
> secondaires — puis MicroDVD, MPL2, TMPlayer et LRC.

**Sept formats pour deux existants**, et un renvoi de la phase 5 : le mode
d'édition en images.

**L'outillage est en place avant le premier lecteur**, ce qui est nouveau et
change la façon dont cette phase se mesure. Les trois issues d'initialisation
(#336) ont livré :

| Outil | Ce qu'il tient |
| :---- | :------------- |
| [`src/test/data/formats/`](../../src/test/data/formats/) — #338 | la même scène dans les neuf formats, et une table de promesses, une ligne par format |
| [`conversion.md`](../mesures/conversion.md) — #339 | ce qu'un aller-retour perd, paire par paire, et le refus d'une perte qui s'aggrave |
| [`detection-de-format.md`](../mesures/detection-de-format.md) — #340 | 12/19, sept refus, aucune confusion |

**Chaque issue de format déplace trois chiffres**, et c'est ainsi que
l'avancement de la phase se lit : une ligne de la table de promesses passe de
`NotYet` à `Readable`, le score de détection monte d'un rendu, et la matrice de
conversion gagne une ligne et une colonne. Aucune de ces trois mesures n'est à
écrire à la main.

## Analyse préalable — ce que Gaupol fait

`aeidon/files/` porte un module par format, `aeidon/markups/` un module par
vocabulaire de balises, et `aeidon/markupconv.py` la traduction de l'un à
l'autre.

| Format | Mode | Positions | Fins | Sauts de ligne | Italique | En-tête |
| :----- | :--- | :-------- | :--- | :------------- | :------- | :------ |
| SubRip | temps | milliseconde | portées | ⏎ | `<i>…</i>` | aucun |
| WebVTT | temps | milliseconde | portées | ⏎ | `<i>…</i>` | libre |
| SubViewer 2 | temps | centième | portées | `[br]` | `<i>…</i>` | sections `[…]` |
| Sub Station Alpha | temps | centième | portées | `\N` | `{\i1}…{\i0}` | sections, styles nommés |
| Advanced SSA | temps | centième | portées | `\N` | `{\i1}…{\i0}` | idem, `v4.00+` |
| MicroDVD | **images** | image | portées | `\|` | `{Y:i}` | ligne `{DEFAULT}` |
| MPL2 | temps | dixième | portées | `\|` | `/` en tête de ligne | aucun |
| TMPlayer | temps | seconde | **absentes** | `\|` | **aucun vocabulaire** | aucun |
| LRC | temps | centième | **absentes** | **absents** | **aucun vocabulaire** | libre |

Trois choses en sortent, et ce sont les trois décisions coûteuses de la phase :
**deux formats ne portent pas de fin**, **un compte en images**, et **cinq
vocabulaires de balises coexistent** là où deux se confondaient.

## D1 — Les fins que le fichier ne porte pas sont déduites, et la lecture le dit

Tranchée, et elle a son ADR : [0029](../adr/0029-fins-deduites-et-annoncees.md).

`lrc.py` et `tmplayer.py` ne lisent qu'une position par ligne. La fin d'une
réplique est le début de la suivante, la dernière reçoit cinq secondes.

**Les trois voies du cadrage, et ce qui les départage :**

| Voie | Ce qu'elle coûte |
| :--- | :--------------- |
| déduire à la lecture, comme Gaupol | le modèle porte une valeur que le fichier ne portait pas |
| rendre `end` optionnel | vingt-huit endroits de la bibliothèque et de l'exécutable, et une politique à inventer dans chacun |
| refuser les deux formats | l'iso-fonctionnalité |

**Ce qui décide n'est pas le compte, c'est qu'il n'y a rien à décider vingt-huit
fois.** Que fait un décalage sur un sous-titre sans fin ? Rien. Qu'affiche la
colonne Durée ? Rien. Que signifie un chevauchement ? La question ne se pose
pas. Chacune de ces réponses est un vide qu'il faudrait écrire, tester et
maintenir, pour un état que **deux formats sur neuf** savent produire et qui
disparaît dès que le document est enregistré ailleurs.

**Ce que la déduction doit à la règle du projet est de se dire.** ADR 0008 :
dire plutôt que taire. Une lecture qui invente des fins émet un diagnostic qui
le nomme, et l'aller-retour reste octet pour octet — l'écriture ne rend que des
débuts, donc rien de ce qui a été inventé ne se retrouve dans le fichier.

C'est ce que la table de promesses de #338 appelle **l'octet à vide** : la
comparaison passerait quoi que la lecture ait inventé, et c'est pourquoi les
quatre fins attendues y sont écrites à la main.

## D2 — Une fréquence lue avec le fichier, et déclarée

Tranchée, et elle a son ADR : [0030](../adr/0030-ce-qu-un-document-retient-de-son-fichier.md).

MicroDVD compte en images ; le modèle compte en millisecondes depuis
l'[ADR 0006](../adr/0006-positions-en-millisecondes.md). **Un fichier MicroDVD
n'énonce aucune fréquence**, et la déduction de la phase 16 ne peut rien pour
lui : elle cherche une grille dans des millisecondes, et des numéros d'image
tombent sur toutes les grilles à la fois.

La fréquence vient donc du dehors, et elle est **portée par le document** au
même titre que son encodage et ses fins de ligne — c'est ce qui rend
l'aller-retour exact, puisque l'écriture reprend celle qui a servi à lire.

**Trois sources, dans cet ordre :**

1. ce que l'utilisateur dit — `--frame-rate` sur la ligne de commande, le
   dialogue dans la fenêtre ;
2. ce que la vidéo associée déclare, quand il y en a une — la phase 6 sait déjà
   la lire ;
3. **un défaut déclaré, 23,976**, que la lecture annonce par un diagnostic.

Le défaut est celui de Gaupol — `aeidon.framerates.FPS_23_976`, et
l'iso-fonctionnalité est la seule raison de le préférer à 25. **Refuser d'ouvrir
faute de fréquence a été écarté** : un fichier qu'on vient de double-cliquer
doit s'ouvrir, et une valeur annoncée qu'on peut corriger vaut mieux qu'un refus.

## D3 — Ce qu'un document retient de son fichier

Même ADR que D2, et c'est la raison de les avoir réunies : la fréquence n'est
pas un cas particulier, c'est le troisième d'une famille.

**Trois choses qu'un fichier porte et qu'aucun sous-titre ne porte :**

| Ce que c'est | Chez Gaupol | Sans quoi |
| :----------- | :---------- | :-------- |
| l'en-tête libre — `[Script Info]`, `[INFORMATION]`, le texte après `WEBVTT` | `header` | l'aller-retour perd des blocs entiers |
| l'ordre des champs d'événement SSA — la ligne `Format:` | `event_fields` | l'écriture réordonne les colonnes de tout le fichier |
| deux réglages de forme — `two_digit_hour` de TMPlayer, la fréquence de MicroDVD | attributs de fichier | l'aller-retour change des chiffres |

`header` existe et est une chaîne : **elle suffit à ce qu'un aller-retour doit
rendre**, parce que rien n'a besoin de la comprendre. Ce qu'elle ne porte pas
est le reste, et ce reste prend la forme d'un `variant` par format, jumeau de
`FormatExtras` au niveau du fichier.

**`FormatExtras` ne devient pas un variant à neuf branches**, et le compter le
dit : cinq des neuf formats ne portent rien par sous-titre — SubViewer 2,
MicroDVD, MPL2, TMPlayer et LRC restent sur `std::monostate`. SSA et ASS
partagent une branche. Le variant passe de trois à quatre branches, et la garde
du compilateur que son commentaire revendique tient sans changer de forme.

## D4 — Un pivot de balises, pour la seule conversion

Tranchée, et elle a son ADR : [0031](../adr/0031-pivot-de-balises-a-la-conversion.md).

L'[ADR 0009](../adr/0009-texte-en-chaine-brute.md) a été écrite pour cette
phase-ci — elle nomme ASS — et elle promet un modèle structuré servant de pivot
**uniquement lors d'une conversion**. Ce pivot n'existe pas : rien ne l'avait
demandé tant que les deux formats partageaient `<i>`.

Il devient indispensable, et la démonstration tient en une ligne : sans lui, un
SSA converti en SubRip rend `{\i1}Il ne dit rien.{\i0}` — du texte que
l'utilisateur voit, dans un fichier qui ne saura jamais l'interpréter. C'est
pire qu'une perte, c'est une salissure.

**Sa portée est celle de Gaupol : gras, italique, souligné, couleur, police,
taille.** Rien d'autre n'est traduit. Ce qui n'est pas reconnu est retiré, et
l'ADR 0009 dit pourquoi c'est acceptable : la perte n'existe qu'à la conversion,
jamais à l'ouverture et à l'enregistrement d'un même format, où la chaîne brute
passe intacte.

**Ce que nous ajoutons à Gaupol est de le dire.** Voir D5.

## D5 — La politique de dégradation, et ce qu'elle dit

**Une conversion ne refuse jamais, et elle ne se tait jamais.** C'est ADR 0008
appliqué à une opération plutôt qu'à une lecture, et la phase 8 a donné la forme
que cela prend : un `reportOutcome` que les trois niveaux de verbosité et la
fenêtre lisent également.

Ce qui est dit, poste par poste, et **seulement quand il y a eu perte** :

| Ce qui a été perdu | Comment c'est dit |
| :----------------- | :---------------- |
| les fins, pour un format qui n'en porte pas | « ends are not carried by LRC » |
| les sauts de ligne | « line breaks were joined » |
| des balises que le format d'arrivée ne sait pas écrire | leur nombre |
| l'en-tête, les données propres au format | ce qui est parti, nommé |
| de la précision | l'écart le plus grand, en millisecondes |

**Ce sur quoi on se tait, et pourquoi.** Une conversion qui ne perd rien ne dit
rien : un rapport qui s'affiche à chaque appel est un rapport que personne ne
lit. Et la perte de précision **n'est annoncée que si elle a lieu** — la moitié
des conversions entre formats au centième portent des positions déjà rondes.

**La déclaration de ce que chaque format oblige à perdre est écrite**, dans
[`conversion.md`](../mesures/conversion.md), et c'est sur elle que cette
politique se décide plutôt que sur une intuition. Ce que l'outil mesure est
l'écart entre elle et la réalité.

## D6 — La détection à neuf candidats garde sa règle

**`detectFormat` promet « rien plutôt qu'une supposition », et la règle tient.**
Les neuf signatures se séparent sur le contenu, et le rapprochement que le
cadrage craignait n'en est pas un :

| Ce qui se ressemble | Ce qui les sépare |
| :------------------ | :---------------- |
| `{25}{75}` de MicroDVD, `[10][30]` de MPL2 | la forme des crochets, et rien d'autre n'a cette grammaire |
| `[00:01.00]` de LRC | un seul crochet, et un horodatage dedans |
| `00:00:01:` de TMPlayer | trois champs et un deux-points final, en tête de ligne |
| SSA et ASS | `ScriptType: v4.00+` et `[V4+ Styles]` |
| SubViewer 2 | `[INFORMATION]`, et une ligne `HH:MM:SS.cc,HH:MM:SS.cc` |

**Ce n'est pas une affirmation, c'est une hypothèse mesurée** — c'est à cela que
sert #340. Chaque format livré fait monter le score, et **une confusion échoue à
elle seule**, quel que soit le taux. Si deux formats devaient revendiquer le
même fichier, la réponse est celle d'aujourd'hui : rien, et `inspect` dit qu'il
n'a pas su.

L'extension n'entre pas dans la décision, et c'est écrit pour être su : le
scorimètre passe chaque fichier sous un nom neutre. Y revenir demanderait de
changer le scorimètre du même geste, ce qui rend le choix visible.

## D7 — Les deux surfaces

**`--to` passe de deux valeurs à neuf**, et le manuel garde sa promesse
d'ensemble fermé énuméré.

**`--frame-rate R` entre sur `convert`**, et il sert des deux côtés : il impose
la fréquence de lecture d'un fichier en images, et celle d'écriture d'une
conversion vers un format en images.

**Sans lui, une conversion vers un format en images prend la grille déduite** —
c'est la phase 16 qui sert, et c'est exactement ce pour quoi elle a été écrite :
la grille d'un fichier temporel *est* la fréquence à laquelle il a été calé. La
conversion le dit. **Sans fréquence et sans grille, elle refuse**, parce que le
seul geste restant serait d'inventer un chiffre qui déplacerait chaque réplique.

`inspect` nomme le format parmi les neuf, et pour un fichier en images il nomme
la fréquence **et d'où elle vient** — la même forme que la ligne d'encodage
depuis la phase 8.

Côté fenêtre : le filtre d'ouverture liste les neuf extensions, `Save As…`
propose les neuf formats, et **un format à perte annonce ce qu'il va perdre
avant d'écrire**, pas après.

## Ce que la phase ne livre pas

- **Le mode d'édition en images.** Renvoyé à la phase 14, et la raison a changé.
  La phase 5 l'avait renvoyé faute de format à images ; celui-ci arrive. Ce qui
  reste vrai est que **MicroDVD porte des numéros d'image et pas de fréquence** :
  la bascule montrerait les chiffres du fichier pour un MicroDVD, et une
  conversion contre une fréquence choisie pour les huit autres. La question est
  donc mieux posée qu'avant, et elle reste une question d'édition fine — le
  sujet de la phase 14, qui porte déjà « avancer, reculer image par image ».
- **Les balises que le pivot ne connaît pas.** `{\pos(x,y)}`, `{\t(...)}`,
  `<ruby>`, les horodatages internes de WebVTT : ils traversent intacts un
  aller-retour dans leur propre format, et disparaissent à la conversion. C'est
  la portée de Gaupol, et l'élargir demanderait un modèle de mise en page que
  rien ne réclame.
- **La traduction des styles nommés SSA.** Un `Style: Default` porte une police,
  un corps, quatre couleurs et des marges. Les convertir en `<font>` SubRip
  serait deviner ; les perdre est ce que Gaupol fait, et la conversion le dit.
- **Un dixième format.** La liste est celle de Gaupol, et elle est close.

## Exigences

| Identifiant | Ce qu'il promet |
| :---------- | :-------------- |
| `CLI-FORMAT-01` | chacun des neuf formats s'ouvre, et se réécrit octet pour octet |
| `CLI-FORMAT-02` | un fichier qu'aucun des neuf ne revendique est refusé, sans supposition |
| `CLI-FORMAT-03` | `inspect` nomme le format lu parmi les neuf |
| `CLI-FORMAT-04` | un format sans fin s'ouvre, ses fins sont déduites, et la lecture le dit |
| `CLI-FRAMES-01` | un fichier en images s'ouvre à une fréquence déclarée, que la lecture nomme |
| `CLI-FRAMES-02` | `--frame-rate` impose la fréquence, à la lecture comme à l'écriture |
| `CLI-CONVERT-04` | `--to` accepte les neuf formats |
| `CLI-CONVERT-05` | convertir vers un format en images sans fréquence prend la grille déduite, et le dit |
| `CLI-CONVERT-06` | sans fréquence ni grille, une conversion vers un format en images est refusée |
| `CLI-CONVERT-07` | une conversion qui perd quelque chose le dit, poste par poste |
| `GUI-FORMAT-01` | la fenêtre ouvre les neuf formats |
| `GUI-FORMAT-02` | `Save As…` propose les neuf formats |
| `GUI-FORMAT-03` | enregistrer dans un format à perte annonce ce qui sera perdu avant d'écrire |
| `GUI-FRAMES-01` | ouvrir un fichier en images annonce la fréquence retenue, et laisse la changer |

## Découpage

L'ordre suit ce que chaque format apprend au modèle, du moins coûteux au plus
coûteux, et **le vocabulaire passe avant tout le reste** : rien ne peut lire un
`.ass` tant que `SubtitleFormat` ne sait pas ce que c'est.

| Issue | Ce qu'elle fait | Ce qu'elle apprend au modèle |
| :---- | :-------------- | :--------------------------- |
| #344 | les neuf formats entrent dans le vocabulaire | `SubtitleFormat`, `nameOf`, `extensionOf`, ce qu'un document retient de son fichier |
| #345 | SubViewer 2 | un en-tête de sections, les centièmes, `[br]` |
| #346 | Sub Station Alpha et Advanced SSA | les champs d'événement, les styles nommés, la branche d'extras |
| #347 | MPL2 | les dixièmes, `\|`, une balise en tête de ligne |
| #348 | MicroDVD | les images, et la fréquence portée par le document |
| #349 | TMPlayer et LRC | les fins déduites, et le diagnostic qui les annonce |
| #350 | le pivot de balises, et ce qu'une conversion dit qu'elle perd | — |
| #351 | la surface de la ligne de commande | — |
| #352 | la surface de la fenêtre | — |

La phase se clôt sur #353, la relecture de fin — la troisième des trois issues
qui l'encadrent.

**Les issues #345 à #349 se ressemblent, et c'est voulu** : chacune ajoute un lecteur,
un écrivain, sa ligne dans la table de promesses de #338, et fait bouger les
deux relevés. Ce qui les distingue est écrit dans la dernière colonne — ce
qu'elles ont à apprendre au modèle, et qu'aucune autre ne lui apprend.

**#350 ne peut pas venir plus tôt** : un pivot de balises se juge sur les
vocabulaires qu'il relie, et ils n'existent pas avant.
