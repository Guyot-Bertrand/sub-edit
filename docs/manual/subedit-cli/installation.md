# Installation

subedit n'est pas encore empaqueté. L'outil se construit depuis les sources.

## Prérequis

- CMake ≥ 3.28
- un compilateur C++23 — GCC 13 convient
- `make`
- **une connexion réseau au premier `cmake`** : la bibliothèque de tests Catch2
  est récupérée depuis GitHub à la configuration. Elle n'est plus retéléchargée
  ensuite. Pour construire l'outil seul, sans réseau, voir plus bas.

## Construire

```bash
git clone git@github.com:Guyot-Bertrand/sub-edit.git subedit
cd subedit
make build
```

Le binaire est produit dans `build/dev/bin/subedit-cli`.

Pour une version optimisée :

```bash
make release
```

Les deux binaires sont alors dans `build/release/bin/` — `subedit-cli` et
`subedit-gui`. La cible ne construit rien d'autre : ni le banc de mesures, ni le
harnais de bout en bout, qui ont leurs propres cibles.

**Préférer cette cible aux deux commandes `cmake` équivalentes**, qui n'ont pas
de quoi savoir combien de processus l'optimisation entre modules a le droit de
lancer : à chaque édition de liens, elle en démarre autant qu'il y a de cœurs.
`make release` lui passe `JOBS`, comme au reste de la construction.

## Construire l'outil seul

Les tests sont construits par défaut, et ce sont eux qui réclament le réseau —
`make release` ne les construit pas, mais il configure le projet, et c'est la
configuration qui va chercher Catch2. Les désactiver une fois suffit : le
réglage reste dans le cache CMake, et les constructions suivantes le gardent.

```bash
cmake --preset release -DSUBEDIT_BUILD_TESTS=OFF
make release
```

Cela ne dispense de rien pour qui contribue : la porte de qualité, elle, les
exige.

## Si la compilation échoue au moment de l'édition des liens

Une avalanche de « référence indéfinie vers `std::cout` » sur du code valide
indique en général que l'alternative `c++` du système pointe sur `gcc` au lieu
de `g++` : le C++ compile, mais la bibliothèque standard n'est pas liée.

```bash
ls -l /etc/alternatives/c++                                       # vérifier
sudo update-alternatives --install /usr/bin/c++ c++ /usr/bin/g++ 100   # corriger
CXX=g++ make build                                                # contourner
```

La configuration CMake détecte ce cas et s'arrête avec ce message plutôt que de
laisser l'édition des liens échouer.
