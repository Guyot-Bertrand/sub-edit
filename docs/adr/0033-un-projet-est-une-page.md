# 0033 — Faire d'un projet une page de la fenêtre

Statut : proposée — 2026-09-20
Proposée en cadrant la phase 11, issue #426. **Acceptée ou abandonnée par #435**, la porte qui précède
la troisième tranche de la phase.

## Contexte

La feuille de route promet des **onglets**, et dit de la phase qui les porte que « le besoin n'est pas
confirmé ». Cette ADR décrit ce qu'ils coûteraient, pour que #435 décide en le sachant ; **elle ne
décide pas qu'ils se font.**

`MainWindow` fait 1 951 lignes et retient, dans un seul objet, **deux sortes d'état** :

| À la fenêtre | À un projet |
| :----------- | :---------- |
| les menus et leurs actions, la barre d'état, le titre | la session et son historique |
| le lecteur, la vue vidéo, la minuterie | le modèle et la table |
| le thème, les répertoires, les réglages de l'ajustement | la ligne où la lecture a été placée |
| le presse-papiers, la boîte de recherche | la vidéo associée, la réplique que l'image porte |
| | la correspondance de la recherche |

Aujourd'hui la fenêtre a **un** projet, et la distinction n'a jamais eu à être faite. Gaupol a des
onglets dans une fenêtre, chacun une `Page`, et c'est là que vit son état de projet.

## Décision

**Un projet est une page.** `ProjectPage` porte la colonne de droite — la session, le modèle, la table,
l'état de lecture et de recherche —, et **la fenêtre en possède une ou plusieurs**. Quand la page courante
change, la fenêtre **recalcule ses menus, sa barre d'état, son titre et son film**.

**Le lecteur n'est pas dupliqué** : un seul, dont le film se change avec la page. Ce que la fenêtre sait
déjà faire — `syncVideo` compare l'association voulue à celle qui est ouverte, et rouvre si elle diffère
— sert donc à changer d'onglet. **La position de lecture ne survit pas au changement d'onglet**, et le manuel
le dit.

**Le presse-papiers reste à la fenêtre** : copier dans un projet et coller dans un autre est ce pour quoi
on en ouvre deux.

**L'extraction se fait d'abord, seule**, sous la forme d'une fenêtre qui possède une seule page, sans aucun
comportement neuf. Elle prouve la séparation avant qu'un second projet ne l'éprouve.

## Alternatives écartées

- **Une fenêtre par projet.** Moins chère aujourd'hui : `MainWindow` telle qu'elle est, et un registre
  d'application. Écartée parce que « fermer tout », « enregistrer tout » et « projet suivant » deviennent
  alors des gestes **entre** fenêtres, avec un menu par fenêtre qui doit savoir ce que font les autres ; et
  parce que la feuille de route dit « onglets », que Gaupol en a, et qu'une fenêtre par projet est une autre
  interface. *Si #435 confirme le besoin de plusieurs projets mais pas d'onglets, c'est la voie de repli.*
- **Un `QTabWidget` dont on échange l'état de la fenêtre à chaque changement d'onglet.** Sans objet de page,
  la fenêtre reste le lieu de tout, et **chaque changement d'onglet est une copie d'état dans les deux sens** —
  précisément là où vivent les fuites d'un projet dans l'autre : une correspondance de recherche, une ligne
  placée, un film qu'on n'a pas changé.
- **Un lecteur par page.** Un processus mpv par onglet : de la mémoire pour des films qu'on ne regarde pas, et
  deux sons possibles à la fois. Le prix d'un seul lecteur, la position perdue au changement d'onglet, est
  plus faible.

## Conséquences

**Facile** : un projet de plus est une page de plus ; l'annulation est déjà par session, donc par onglet.

**Difficile** : c'est **la plus grosse réécriture de la phase** — `main_window.cpp` en est traversé —, et le
seul morceau dont on ne revient pas facilement. Le critère qui la rend sûre est écrit dans l'issue d'extraction :
**les tests de la fenêtre passent sans qu'on en réécrive un.** Si l'un d'eux doit changer, la séparation n'était
pas celle qu'on croyait, et l'ADR se rouvre.

**Le déclencheur d'abandon** est la réponse de #435 : sans multi-projets, cette ADR devient « abandonnée », avec
la raison, et `MainWindow` reste un objet à un projet.
