# 0003 — Cibler Linux d'abord, sans fermer la porte aux autres systèmes

**Date :** 2026-08-04
**Statut :** acceptée

## Contexte

L'utilisateur final travaille sous Linux, comme Gaupol dont les installateurs
Windows ne sont plus produits depuis la version 1.3.1. Cibler trois systèmes dès
le départ multiplierait le coût de chaque décision — dépendances, empaquetage,
lecteur vidéo, tests — pour un besoin qui n'existe pas encore.

Mais un portage ultérieur ne doit pas exiger de changer de socle technique.

## Décision

Linux est la seule plateforme construite, testée et empaquetée. En revanche,
**aucun choix ne doit rendre un portage coûteux** : à qualité comparable, on
retient systématiquement l'option portable.

## Alternatives écartées

- **Linux exclusivement, sans égard à la portabilité** — le plus simple à court
  terme. Écarté parce que les choix non portables se découvrent tard, quand ils
  sont enchâssés : c'est ce qui a coûté Windows à Gaupol.
- **Trois plateformes dès le départ** — multiplierait le travail sur chaque
  phase pour un besoin hypothétique.

## Conséquences

Cette décision oriente concrètement plusieurs choix ultérieurs :

- Qt plutôt que GTK — voir [0001](0001-cpp20-et-qt6.md) ;
- pas d'appel direct aux API POSIX là où la bibliothèque standard ou Qt offrent
  un équivalent ;
- `std::filesystem` plutôt que les fonctions POSIX de manipulation de chemins ;
- le choix du backend vidéo (phase 6) devra être arbitré en tenant compte de sa
  disponibilité sous Windows.

Le déclencheur qui rouvrirait cette décision : un besoin réel exprimé pour
Windows. Il rouvrirait aussi [0004](0004-gestion-des-dependances.md), puisque
les dépendances devraient alors être versionnées de façon identique sur les deux
systèmes.
