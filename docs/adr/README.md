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

## Décisions attendues

Points ouverts identifiés, qui feront l'objet d'une ADR le moment venu :

- **Moteur d'expressions régulières** — PCRE2, compatible avec la syntaxe Python
  des motifs de Gaupol, ou RE2, plus rapide mais sans références arrière, que
  ces motifs utilisent. Phase 12, après mesure. La phase 4 s'en passe et dit
  pourquoi — [0017](0017-analyseur-de-mentions-ecrit-a-la-main.md) — donc la
  question reste entière, à trancher avec les critères de la phase 12 sous les
  yeux plutôt qu'avec deux délimiteurs littéraux pour seul usage.
- **Internationalisation** — Qt Linguist ou gettext. Phase 15.
