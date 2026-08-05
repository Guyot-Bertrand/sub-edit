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

[0007](0007-cpp23-et-std-expected.md) remplace partiellement
[0001](0001-cpp20-et-qt6.md) sur le point de la norme : C++23 et non C++20. Le
reste de 0001 — Qt 6, cœur sans dépendance à l'interface — reste en vigueur.

## Décisions attendues

Points ouverts identifiés, qui feront l'objet d'une ADR le moment venu :

- **Moteur d'expressions régulières** — PCRE2, compatible avec la syntaxe Python
  des motifs de Gaupol, ou RE2, plus rapide mais sans références arrière, que
  ces motifs utilisent. Phase 12, après mesure.
- **Backend vidéo** — libmpv ou QtMultimedia. Phase 14.
- **Internationalisation** — Qt Linguist ou gettext. Phase 15.
