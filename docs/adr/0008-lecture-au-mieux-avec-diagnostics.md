# 0008 — Ouvrir les fichiers au mieux, en rapportant des diagnostics

**Date :** 2026-08-05
**Statut :** acceptée

## Contexte

Les fichiers de sous-titres réels sont souvent imparfaits : issus d'OCR,
récupérés sur le web, produits par des outils approximatifs, édités à la main.
L'utilisateur visé travaille précisément sur de tels fichiers — la suppression
des mentions pour malentendants figure dans ses priorités, ce qui suppose des
sources récupérées.

L'analyse de Gaupol montre une tolérance **asymétrique** : ses expressions
régulières sont délibérément permissives sur les horodatages — champs de 1 ou
2 chiffres, millisecondes de 1 à 3 chiffres, heures facultatives en WebVTT,
virgule ou point, temps négatifs — mais sa structure n'est pas protégée. Un
`.srt` dont le premier bloc non vide n'est pas un horodatage produit un
`IndexError`, alors que les exceptions documentées sont `IOError` et
`UnicodeError`.

Autrement dit : Gaupol est tolérant sur ce qu'il a prévu et échoue de façon
inexploitable sur le reste.

## Décision

La lecture **ouvre au mieux et rapporte**. Deux niveaux nettement séparés :

- **Échec** — le fichier est illisible ou indécodable : accès refusé, séquence
  d'octets invalide pour l'encodage, aucun sous-titre reconnaissable. La lecture
  renvoie une erreur, rien n'est ouvert.
- **Diagnostic** — le fichier est exploitable mais quelque chose n'a pas été
  compris ou paraît douteux : ligne ignorée, horodatage incohérent, fin
  antérieure au début, chevauchement, bloc de format inconnu. Les sous-titres
  sont renvoyés **avec** la liste des diagnostics.

Chaque diagnostic porte une **gravité**, un **numéro de ligne**, une **catégorie
énumérée** et un détail facultatif. La catégorie est un `enum class`, pas une
chaîne : elle doit être traduisible et testable sans comparer du texte.

## Alternatives écartées

- **Échec net à la moindre anomalie** — plus sûr en apparence. Écarté : refuser
  d'ouvrir un fichier de deux mille sous-titres à cause d'une ligne douteuse
  laisse l'utilisateur sans recours, alors que l'outil qu'il tient en main est
  précisément un éditeur.
- **Tolérance silencieuse, comme Gaupol** — le plus simple. Écarté parce que
  l'utilisateur ne peut pas savoir qu'il a perdu quelque chose. Un fichier
  ouvert puis enregistré en ayant discrètement laissé tomber trois lignes est un
  défaut plus grave qu'un refus d'ouvrir.
- **Réparer automatiquement** — inverser une fin antérieure au début, corriger
  les chevauchements. Écarté : c'est une décision éditoriale, elle appartient à
  l'utilisateur. Les opérations correspondantes existeront comme commandes
  explicites et annulables.

## Conséquences

Toutes les fonctions de lecture renvoient un `std::expected` — voir
[0007](0007-cpp23-et-std-expected.md) — dont le succès transporte les
diagnostics. La signature dit donc la vérité : le succès n'implique pas
l'absence de problème.

**Un invariant est délibérément non imposé** : `fin ≥ début` n'est pas garanti
par le modèle. Les principes de conception demandent de rendre les états
invalides non représentables, mais un fichier réel peut contenir un sous-titre
dont la fin précède le début, et l'utilisateur doit pouvoir le voir pour le
corriger. Rendre cet état impossible rendrait le fichier impossible à ouvrir.
C'est une exception assumée, et la seule de ce type.

L'interface de la phase 5 devra présenter ces diagnostics — un bandeau, une
liste consultable — sans bloquer l'ouverture. La ligne de commande de la
phase 3 les écrira sur la sortie d'erreur et les comptera dans son code de
retour.

Le corpus de test doit contenir des fichiers réellement malformés, faute de quoi
ce mécanisme ne serait jamais éprouvé.
