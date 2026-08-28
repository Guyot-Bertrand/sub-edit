# 0025 — Une seule recette d'ouverture, au noyau, et l'erreur qui en sort

Statut : acceptée — 2026-08-28
Décidée en réalisant l'issue #154.

## Contexte

Lire un fichier de sous-titres, c'est enchaîner trois appels dans cet ordre :
les octets sont demandés au système, `readSubtitles` les reconnaît, et le
projet garde ce qu'il faut pour réécrire le fichier tel qu'il est arrivé —
son format compris, depuis l'[ADR 0018](0018-vocabulaire-des-formats-dans-le-modele.md).

**Cet enchaînement était écrit quatre fois** : dans `gui/opening.cpp` et dans
les trois commandes de `cli/` qui lisent avant d'agir. Un seul des quatre sites
portait un commentaire disant pourquoi l'ordre est celui-là. C'est la définition
d'un contournement qui se répète.

La copie de la fenêtre en avait tiré une conséquence de plus, et coûteuse.
Elle traduisait **toute** `FileError` en `ReadErrorKind::NoSubtitleFound` :

```cpp
// A file that cannot be opened and a file that cannot be understood are
// both « nothing to show », and the window says so the same way.
```

Le raisonnement se défend pour ce que la fenêtre *fait* — dans les deux cas,
rien ne s'ouvre. Il ne se défend pas pour ce qu'elle *dit* : un fichier absent,
un fichier refusé par le système, des octets qui ne sont pas de l'UTF-8 et un
document Word recevaient la même phrase, et elle n'était vraie que du dernier.
La fenêtre avait donc **moins d'information que le terminal** exactement là où
l'utilisateur en a le plus besoin : au moment où il ne comprend pas pourquoi son
fichier ne s'ouvre pas.

## Décision

**La recette vit dans `core/format/`, sous le nom `openProject`, et les quatre
sites l'appellent.** Son pendant `saveProject` l'accompagne.

**Ce qu'elle rend en cas d'échec est une somme des deux erreurs qui peuvent se
produire**, et non l'une écrasée dans l'autre :

```cpp
using OpenError = std::variant<FileError, ReadError>;
```

## Pourquoi `format/` et non `io/`

C'est la question que l'issue laissait ouverte, parce qu'aucune des deux
réponses n'est évidente : la fonction touche au système de fichiers, ce que
`readSubtitles` ne fait pas, et elle connaît les formats, ce que `io/` ne fait
pas.

**Ce qui tranche est le type qu'elle rend.** `io/` rend des octets et des
erreurs de système ; `format/` rend des modèles. `openProject` rend un `Project`
— un modèle — donc elle est du côté du format, qui se sert de `io/` comme
`writeAtomically` se sert de `FileSystem`. Le dépendant nomme le répertoire, pas
la dépendance.

Le pendant existait déjà et penchait dans l'autre sens en apparence :
`writeAtomically` vit dans `io/`. Ce n'est pas une contradiction, c'est la même
règle appliquée : `writeAtomically` rend des octets à un fichier et ne sait rien
d'un sous-titre. `saveProject`, qui part d'un `Project`, est dans `format/` avec
`openProject`.

## Pourquoi une somme, et non une troisième énumération

Les deux énumérations suivent la même règle, écrite dans `read_error.hpp` :
**chacune de leurs valeurs est produite par quelque chose.** Une valeur que rien
n'émet est une promesse non tenue — chaque appelant qui la traduit en message
porte une branche qu'aucun test ne peut atteindre. `ReadErrorKind` a d'ailleurs
perdu un membre pour cette raison, `FileNotReadable`, que rien n'émettait.

Fondre les sept causes en une liste unique demanderait de maintenir une
correspondance dans les deux sens, et d'y ajouter, à chaque cause nouvelle d'un
côté, une valeur de l'autre. La somme ne produit rien de neuf : elle exprime le
choix, et rien que le choix.

**Les mots existaient déjà.** `core/wording.hpp` savait dire les sept ; seule la
fenêtre ne pouvait pas les atteindre. Un `reasonOf(const OpenError&)` de trois
lignes suffit, et c'est tout ce que la moitié visible du ticket a coûté.

## Conséquences

**`src/lib/subedit/gui/` ne contient plus de recette de noyau.** C'est ce qui
rend la frontière lisible, et `check-architecture.sh` la tient déjà dans l'autre
sens en refusant Qt sous `core/`.

Une exception subsiste et elle est nommée : `mpv_player` ne contient pas une
ligne de Qt et reste dans `gui/`, pour la raison que
l'[ADR 0020](0020-libmpv-pour-le-lecteur-integre.md) écrit — un lecteur est un
élément d'interface, il n'existe que pour la fenêtre, il en reçoit la sienne. La
règle n'est donc **pas mécanisée** : un contrôle qui refuserait tout fichier
sans Qt sous `gui/` signalerait celui-là à chaque exécution, et un contrôle qui
crie au loup finit désactivé.

**La fenêtre dit ce que disait déjà le terminal.** Le manuel de la fenêtre
énumère désormais les messages qu'elle écrit vraiment ; il disait auparavant que
le message était « le même pour les quatre cas », ce qui était exact et
regrettable.

**L'écriture n'a pas été unifiée avec celle de la ligne de commande**, et ce
n'était pas demandé sans réserve : l'issue disait de regarder, et de ne le faire
que si ça tenait. Ça ne tient pas. La ligne de commande écrit vers une
destination calculée, en lot, avec une forme que ses options peuvent changer ;
la fenêtre écrit un fichier vers un chemin que l'utilisateur vient de choisir.
Ce qu'elles partagent est `writeSubtitles` puis `writeAtomically`, qu'elles
appellent déjà toutes les deux. `saveProject` descend au noyau parce que sa
place était mal choisie, pas parce qu'un second appelant l'attendait.
