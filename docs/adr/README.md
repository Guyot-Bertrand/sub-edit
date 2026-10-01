# Décisions d'architecture

Une décision par fichier, numérotée. Le format est dans
[0000-modele.md](0000-modele.md).

**Une ADR n'est jamais modifiée.** Quand une décision change, on en écrit une
nouvelle qui remplace l'ancienne, et l'ancienne prend le statut « remplacée ».
Le raisonnement reste ainsi lisible, y compris celui qui s'est révélé faux.

Ce que ces fichiers apportent que l'historique git n'apporte pas : l'historique
dit *ce qui* a été fait ; l'ADR dit *pourquoi les autres options ont été
écartées*. C'est cette information-là qui manque six mois plus tard.

| N° | Décision | Statut |
| :- | :------- | :----- |
| [0001](0001-cpp20-et-qt6.md) | Écrire subedit en C++20 avec Qt 6 | acceptée |
| [0002](0002-licence-gpl3.md) | Publier sous GPL-3.0-or-later | acceptée |
| [0003](0003-linux-d-abord.md) | Cibler Linux d'abord, sans fermer la porte | acceptée |
| [0004](0004-gestion-des-dependances.md) | Résoudre les dépendances par les paquets système | acceptée |
| [0005](0005-catch2-pour-les-tests.md) | Utiliser Catch2 v3 pour les tests et benchmarks | acceptée |
| [0006](0006-positions-en-millisecondes.md) | Positions en millisecondes entières, types forts | acceptée |
| [0007](0007-cpp23-et-std-expected.md) | C++23 et `std::expected` | acceptée |
| [0008](0008-lecture-au-mieux-avec-diagnostics.md) | Ouvrir au mieux, rapporter des diagnostics | acceptée |
| [0009](0009-texte-en-chaine-brute.md) | Texte en chaîne brute portant les balises du format | acceptée |
| [0010](0010-annulation-par-commandes.md) | Commandes portant leur propre inverse | acceptée |
| [0011](0011-numero-d-image-en-type-fort.md) | Numéro d'image en type fort `Frame` | acceptée |
| [0012](0012-ordre-des-sous-titres-par-composition.md) | Ne pas trier de soi-même, mode strict par composition | acceptée |
| [0013](0013-mise-a-l-echelle-exacte-des-positions.md) | Mise à l'échelle par un rationnel exact, arrondi une fois | acceptée |
| [0014](0014-registre-d-exigences.md) | Registre d'exigences plat, cité par un tag de test | acceptée |
| [0015](0015-memoire-des-mesures.md) | Cliquet sur les lignes non couvertes, historique des performances | acceptée |
| [0016](0016-cli11-pour-l-analyse-d-arguments.md) | CLI11 pour l'analyse d'arguments, aide engendrée | acceptée |
| [0017](0017-analyseur-de-mentions-ecrit-a-la-main.md) | Balayage écrit à la main, sans moteur d'expressions rationnelles | acceptée |
| [0018](0018-vocabulaire-des-formats-dans-le-modele.md) | Séparer le vocabulaire des formats de leurs opérations | acceptée |
| [0019](0019-table-en-adaptateur-mince.md) | Lire à travers le modèle du noyau, qui rend ses changements | acceptée |
| [0020](0020-libmpv-pour-le-lecteur-integre.md) | libmpv pour le lecteur intégré | acceptée |
| [0021](0021-analyse-du-document-a-l-ouverture.md) | Une analyse du document, calculée à l'ouverture | acceptée |
| [0022](0022-configuration-au-noyau-et-tolerance-par-option.md) | Configuration au noyau, tolérante option par option | acceptée |
| [0023](0023-deb-et-rpm-pour-la-premiere-livraison.md) | Deux paquets natifs, `.deb` et `.rpm`, pour la première livraison | acceptée |
| [0024](0024-captures-engendrees-et-ou-elles-font-foi.md) | Des captures engendrées, et l'environnement où elles font foi | acceptée |
| [0025](0025-une-recette-d-ouverture-au-noyau.md) | Une seule recette d'ouverture, au noyau, et l'erreur qui en sort | acceptée |
| [0026](0026-le-graphe-d-inclusions-reste-tel-quel.md) | Laisser le graphe d'inclusions tel quel, et pourquoi | acceptée |
| [0027](0027-icu-pour-les-encodages.md) | ICU pour lire et écrire les encodages, en dépendance du noyau | acceptée |
| [0028](0028-peser-les-lignes-qui-discriminent.md) | La détection ne pèse que les lignes qui discriminent | acceptée |
| [0029](0029-fins-deduites-et-annoncees.md) | Déduire les fins que le fichier ne porte pas, et l'annoncer | acceptée |
| [0030](0030-ce-qu-un-document-retient-de-son-fichier.md) | Ce qu'un document retient de son fichier, la fréquence comprise | acceptée |
| [0031](0031-pivot-de-balises-a-la-conversion.md) | Un pivot de balises, pour la seule conversion | acceptée |
| [0032](0032-un-document-un-fichier.md) | Retenir un fichier par document | acceptée |
| [0033](0033-un-projet-est-une-page.md) | Faire d'un projet une page de la fenêtre | acceptée |
| [0034](0034-trois-collaborateurs-de-la-fenetre.md) | Trois collaborateurs de la fenêtre, qui reçoivent la page qu'ils visent | acceptée |
| [0035](0035-les-operations-sortent-de-la-fenetre.md) | Les opérations de `Tools` sortent de la fenêtre | acceptée |
| [0036](0036-icu-pour-les-motifs-de-correction.md) | ICU pour appliquer les motifs de correction | acceptée |
| [0037](0037-lire-les-motifs-de-gaupol-tels-quels.md) | Lire les fichiers de motifs de Gaupol tels quels | acceptée |
| [0038](0038-sortie-json-lines-versionnee.md) | Offrir une sortie lisible par un script : JSON Lines, versionnée | acceptée |
| [0039](0039-le-lot-collisions-dossiers-et-arborescence.md) | Un lot sûr : collisions refusées, dossier créé, arborescence conservée | acceptée |
| [0040](0040-correct-ecrit-directement-dry-run-propose.md) | Écrire directement, et proposer par `--dry-run` | acceptée |

[0011](0011-numero-d-image-en-type-fort.md) complète
[0006](0006-positions-en-millisecondes.md) : elle donne un type à la « vue en
images » que 0006 nomme sans la typer. Aucun point de 0006 n'est remis en cause.

[0007](0007-cpp23-et-std-expected.md) remplace partiellement
[0001](0001-cpp20-et-qt6.md) sur le point de la norme : C++23 et non C++20. Le
reste de 0001 — Qt 6, cœur sans dépendance à l'interface — reste en vigueur.

[0021](0021-analyse-du-document-a-l-ouverture.md) donne un endroit à ce que
[0006](0006-positions-en-millisecondes.md) et
[0011](0011-numero-d-image-en-type-fort.md) rendaient possible sans le nommer :
une inférence sur un document, distincte de ce qu'il est et de ce qui le change.
Elle a placé le déplacement de `anomaly.hpp` vers `core/analysis/` sous
condition ; la #227 l'a fait le jour où la condition a été remplie.

[0028](0028-peser-les-lignes-qui-discriminent.md) complète
[0027](0027-icu-pour-les-encodages.md) sans la remplacer : ICU reste le
détecteur et le convertisseur, et ce qui change est ce qu'on lui soumet. Son
banc d'essai ne portait que des fichiers monolingues, et c'est en mesurant où la
détection cesse de savoir — #310 — qu'un fichier bilingue s'est révélé perdre
son second alphabet en entier.

[0031](0031-pivot-de-balises-a-la-conversion.md) **honore**
[0009](0009-texte-en-chaine-brute.md) sans la remplacer : le pivot qu'elle
promettait « uniquement lors d'une conversion » est construit en phase 9, parce
que neuf formats font cinq vocabulaires de balises là où deux en partageaient
un. La chaîne brute reste ce qu'un texte est à l'ouverture et à
l'enregistrement, et la porte de sortie à fragments opaques que 0009 décrit
reste ouverte.

**0031 dit le parseur conscient des balises « toujours pas écrit » ; il l'est
depuis la phase 10**, issue #378, dans `core/text/markup_parser`. Ce sont bien
les deux pièces que 0031 distinguait : le pivot traduit un vocabulaire en un
autre et retire ce qu'il ne sait pas porter, le parseur transporte les balises
autour d'un texte qu'on transforme et n'en retire aucune. La casse, les tirets,
la recherche et l'ajustement des durées passent par lui.

**Et un seul lecteur coupe les balises pour les deux**, depuis #403 —
`core/text/markup_reader`. Le pivot, le parseur, l'italique et l'analyseur de
mentions lisaient chacun les leurs, et se contredisaient : `<i >` était de
l'italique pour l'un et rien pour l'autre. Ce que dit une balise reste l'affaire
de chaque pièce ; où elle commence, où elle finit et ce qu'elle nomme se décide
en un endroit.

[0029](0029-fins-deduites-et-annoncees.md) et
[0030](0030-ce-qu-un-document-retient-de-son-fichier.md) tiennent le modèle en
place là où deux formats le tiraient dehors : `Subtitle::end` ne devient pas
optionnel pour LRC et TMPlayer, et les positions ne deviennent pas des images
pour MicroDVD. Ce qui bouge est ce qu'un **document** retient de son fichier,
pas ce qu'un **sous-titre** porte.

[0036](0036-icu-pour-les-motifs-de-correction.md) **rouvre** puis **maintient**
[0017](0017-analyseur-de-mentions-ecrit-a-la-main.md) : le déclencheur que 0017 s'était
donné — le troisième motif demandé — a joué à la phase 12, et le balayage écrit à la main
garde pourtant les crochets et les parenthèses, pour la raison qu'elle donnait. Les quatre
autres motifs de mentions passent par le moteur ; 0017 n'est pas remplacée.
[0037](0037-lire-les-motifs-de-gaupol-tels-quels.md) fixe, elle, ce que ce moteur lit.

[0038](0038-sortie-json-lines-versionnee.md) **lève** le point ouvert de la
[spec de la phase 3](../specs/03-cli.md) — « forme d'une sortie lisible par une
machine » — et complète [0008](0008-lecture-au-mieux-avec-diagnostics.md) : les diagnostics
de lecture, que le texte range au niveau 3, sont des données et vont toujours dans
`warnings`. [0039](0039-le-lot-collisions-dossiers-et-arborescence.md) tient pour le lot ce
que la phase 3 ne tenait que pour un fichier — une erreur d'usage ne laisse jamais un lot à
moitié écrit —, et [0040](0040-correct-ecrit-directement-dry-run-propose.md) répond à la
confirmation que [0036](0036-icu-pour-les-motifs-de-correction.md) et la phase 12 avaient
laissée à la ligne de commande.

## Décisions attendues

Points ouverts identifiés, qui feront l'objet d'une ADR le moment venu :

- **Internationalisation** — Qt Linguist ou gettext. Phase 15.
