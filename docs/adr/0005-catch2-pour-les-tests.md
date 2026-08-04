# 0005 — Utiliser Catch2 v3 pour les tests et les benchmarks

**Date :** 2026-08-04
**Statut :** acceptée

## Contexte

La performance étant un objectif de conception transversal, les benchmarks ne
sont pas un ajout tardif : ils doivent être outillés dès les fondations, pour
que les optimisations ultérieures soient argumentées par des mesures plutôt que
par des intuitions.

## Décision

Catch2 v3, pour les tests unitaires **et** les micro-benchmarks.

## Alternatives écartées

- **GoogleTest + Google Benchmark** — le duo le plus répandu, avec gmock en
  prime. Écarté parce qu'il fait deux dépendances au lieu d'une pour un besoin
  que Catch2 couvre seul, et parce que la simulation d'objets sert peu ici : le
  cœur est une bibliothèque de transformation de données, testable sur des
  valeurs réelles.
- **doctest + nanobench** — compilation nettement plus rapide et empreinte plus
  légère. Écarté sur la maturité de l'écosystème et de la documentation ; le
  gain de compilation ne se ferait sentir qu'à une échelle que le projet
  n'atteint pas.

## Conséquences

Écrire un benchmark à côté d'un test ne demande aucune dépendance
supplémentaire, ce qui supprime le principal frein à en écrire.

`catch_discover_tests` enregistre chaque cas individuellement dans CTest : un
échec désigne le cas fautif, et un cas isolé se rejoue seul.

Les benchmarks forment un exécutable distinct, non enregistré dans CTest : ils
se mesurent en Release, jamais pendant une passe de tests instrumentée par les
sanitizers ou la couverture, qui fausseraient les chiffres.
