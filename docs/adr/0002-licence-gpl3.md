# 0002 — Publier subedit sous GPL-3.0-or-later

**Date :** 2026-08-04
**Statut :** acceptée

## Contexte

Gaupol est publié sous GPL-3.0-or-later. Le code n'est pas repris — subedit est
une réécriture indépendante — mais Gaupol embarque des **données** sous la même
licence, qui représentent des années de travail linguistique :

- les motifs de correction (`aeidon/data/patterns/`) : erreurs courantes
  classées Humain/OCR, remise en majuscule, suppression des mentions pour
  malentendants, pénalités de coupure de ligne, déclinés par script, langue et
  pays ;
- les en-têtes par défaut des formats (`aeidon/data/headers/`) ;
- les traductions de l'interface (`po/`, 20 locales).

Reproduire ce corpus de zéro représenterait un effort considérable, pour un
résultat de moindre qualité, et éloignerait le projet de son objectif
d'iso-fonctionnalité.

## Décision

subedit est publié sous **GPL-3.0-or-later**, ce qui autorise la réutilisation
directe des données de Gaupol, avec attribution.

## Alternatives écartées

- **Licence permissive (MIT, Apache-2.0) avec motifs réécrits** — offrirait une
  liberté d'usage totale, mais imposerait de reconstruire tout le travail
  linguistique. Le moteur de correction est la partie la plus riche en valeur
  métier : la repartir de zéro dégraderait le produit sans bénéfice pour son
  utilisateur.
- **Licence permissive sans moteur de correction** — reviendrait à renoncer à
  l'iso-fonctionnalité, qui est l'objectif du projet.

## Conséquences

Toute distribution de subedit doit s'accompagner de ses sources. Le projet ne
peut pas être intégré à un produit propriétaire — ce qui n'est ni un objectif
ni une contrainte pour un éditeur de sous-titres destiné à un usage personnel.

Les dépendances doivent rester compatibles GPL-3.0 : Qt en LGPLv3 lié
dynamiquement l'est, ce qui interdit en revanche un lien statique avec Qt.

L'origine des données reprises est à créditer explicitement dans le dépôt.
