# 0012 — Ne pas trier de soi-même, et faire du mode strict une composition

**Date :** 2026-08-08
**Statut :** acceptée

## Contexte

Un sous-titre dont on corrige le début peut se retrouver placé avant celui qui
le précède. La question se pose avant d'écrire la première opération d'édition :
« trié par début » est-il un invariant du projet ?

Gaupol répond oui, et le tient à deux endroits. À l'ouverture, il trie, compte
les sous-titres déplacés et ouvre une boîte de dialogue — *« Ouvrir le fichier
non trié ? L'ordre de N sous-titres doit être changé »* — que l'utilisateur peut
refuser. À l'édition, `set_start` appelle `_move_if_needed`, qui déplace la ligne.

**Cet invariant n'en est pas un.** `replace_positions`, par où passent le
décalage, la transformation et la conversion de fréquence, ne trie pas : décaler
une partie d'une sélection peut désordonner le fichier sans que rien ne le
remarque. Et le comptage à l'ouverture compte en images ce que le tri compare en
secondes ; deux sous-titres inversés de moins d'une image sont donc réordonnés
sans un mot, précisément dans le cas le plus difficile à repérer à l'œil.

Notre lecteur, lui, ne trie pas et garantit un aller-retour fidèle octet pour
octet.

## Décision

**Le noyau ne trie jamais de lui-même.** L'ordre est celui du fichier, une
édition ne déplace aucune ligne, et le désordre produit un diagnostic à la
lecture — `OutOfOrder` — comme toute autre anomalie.

Deux modes sont offerts à l'utilisateur, portés par la session :

- **souple**, par défaut — rien ne bouge sans qu'on le demande ;
- **strict** — après une opération susceptible de rompre l'ordre, la session
  enchaîne une **commande de tri dans la même composite**.

Le mode strict est donc une **politique de composition**, et non un invariant du
modèle. Le tri reste une commande ordinaire, annulable, comptant pour une seule
entrée d'historique avec l'opération qui l'a déclenchée.

## Alternatives écartées

- **Invariant tenu par `Project`**, à la manière de Gaupol. Écarté sur le coût :
  les indices bougeraient sous les pieds de l'appelant, `Change` devrait exprimer
  un déplacement et l'annulation restituer un *ordre* plutôt qu'une valeur.
  Chaque commande paierait ce prix, définitivement, pour un cas rare.
- **Deux invariants selon le mode.** Ce ne serait pas un réglage mais deux
  modèles de données : chaque commande devrait demander dans lequel elle
  s'exécute, et le code écrit sous l'un se tromperait sous l'autre.
- **Trier à l'ouverture, en avertissant.** C'est la réponse de Gaupol. Écartée
  pour deux raisons : elle casserait l'aller-retour octet pour octet, seule
  promesse forte de la bibliothèque aujourd'hui ; et un dialogue est un
  *événement*, qui ne survit pas à sa fermeture, là où un diagnostic est un
  *état* qui persiste dans le résultat de lecture.
- **Refuser d'ouvrir un fichier trop désordonné**, ce que Gaupol propose.
  Écarté : le seuil serait arbitraire, et l'ADR 0008 pose que le noyau ouvre au
  mieux et rapporte.

## Conséquences

**Les indices restent stables à l'intérieur d'une commande.** C'est la propriété
porteuse : `Change` transporte des `SubtitleIndex`, l'historique rejoue des
commandes, l'interface rafraîchira par indice. Aucune opération n'a à se
demander si elle a déplacé quelque chose.

**Le tri devient annulable**, ce qu'il n'est pas chez Gaupol, où il précède
l'existence de l'historique.

**Le compteur de modification dit la vérité.** Ouvrir un fichier désordonné en
mode strict marque le document modifié — ce qu'il est, puisqu'il diffère du
disque. Gaupol modifie en mémoire sans que rien ne l'indique jusqu'à
l'enregistrement.

En contrepartie, un sous-titre peut se retrouver affiché hors ordre en mode
souple. L'interface doit le rendre visible, et `Project::outOfOrder()` lui en
donne les moyens sans que la règle vive dans l'interface.

Quand le tri se déclenche, la composite déplace des lignes et son `Change` doit
l'exprimer. La différence avec un invariant est que ce cas vient d'**une
commande identifiée**, traitée une fois, au lieu de peser sur toutes.

Défaire cette décision reviendrait à rendre l'ordre invariant, donc à reprendre
chaque commande. Le déclencheur serait un usage montrant que les utilisateurs
attendent le déplacement automatique — auquel cas le mode strict, déjà là,
deviendrait le défaut avant qu'on envisage l'invariant.
