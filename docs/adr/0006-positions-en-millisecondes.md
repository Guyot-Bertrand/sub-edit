# 0006 — Représenter les positions en millisecondes entières, avec des types forts

**Date :** 2026-08-05
**Statut :** acceptée

## Contexte

Gaupol encode le *type* d'une position dans le **type Python de la valeur** :
un `int` est un numéro d'image, un `float` des secondes, une `str` un temps
`"HH:MM:SS.mmm"`. Chaque méthode de son `Calculator` commence par une chaîne de
dispatch, et les temps sont découpés par indices de caractères :

```python
float(time[:2]) * 3600 + float(time[3:5]) * 60 + float(time[6:8]) + float(time[9:]) / 1000
```

Un sous-titre stocke ses positions « en unités natives » — chaîne de temps ou
entier d'images selon un attribut `mode` porté par le sous-titre lui-même.

Le contour du MVP impose par ailleurs la conversion de fréquence d'image, donc
des allers-retours entre temps et images qui ne doivent pas dériver.

## Décision

Trois types distincts, non convertibles implicitement entre eux ni vers un type
entier :

- **`Timestamp`** — position absolue, `std::int64_t` de millisecondes signées ;
- **`Duration`** — durée, `std::int64_t` de millisecondes signées ;
- **`FrameRate`** — fréquence d'image, **rationnel exact** `numérateur /
  dénominateur`, par exemple `24000/1001` pour 23,976.

L'arithmétique autorisée est celle qui a un sens :

```
Timestamp − Timestamp → Duration
Timestamp ± Duration  → Timestamp
Duration  ± Duration  → Duration
Duration  × scalaire  → Duration
Timestamp + Timestamp → ne compile pas
```

Le numéro d'image n'est **pas** une représentation interne : c'est une vue,
obtenue par conversion explicite avec une `FrameRate`.

**Les positions négatives sont valides.** Un décalage peut légitimement faire
passer un sous-titre avant l'origine, et Gaupol le permet déjà.

**Arrondi : à l'entier le plus proche, la moitié s'éloignant de zéro.** Ce choix
est symétrique autour de zéro, donc décaler de la même quantité un sous-titre
avant et après l'origine donne des résultats cohérents. Gaupol hérite de
l'arrondi au pair le plus proche de Python, qui ne l'est pas.

## Alternatives écartées

- **Reproduire le modèle de Gaupol** — unités natives par sous-titre, type porté
  par la valeur. Écarté sans hésitation : c'est le contre-modèle nommé dans
  [`../principes-de-conception.md`](../principes-de-conception.md), et il oblige
  chaque fonction à un dispatch à l'exécution que le compilateur devrait faire.
- **Secondes en virgule flottante** — simple, mais l'accumulation d'erreurs sur
  un décalage puis une transformation affine est exactement ce qu'un éditeur de
  sous-titres ne doit pas produire. Deux positions égales à l'affichage
  pourraient différer en mémoire.
- **`std::chrono::milliseconds`** — sémantiquement juste et tentant. Écarté
  parce que `chrono` ne distingue pas un instant d'une durée sans passer par
  `time_point`, dont l'horloge n'a aucun sens ici, et parce que le formatage vers
  `HH:MM:SS,mmm` et l'arithmétique avec une fréquence rationnelle demanderaient
  de toute façon un type propre. Rien n'interdit de s'appuyer dessus en interne.
- **Fréquence d'image en `double`** — ce que fait Gaupol. Écarté : `24/1.001`
  n'est pas représentable exactement, et l'erreur se propage à chaque conversion.
  Un rationnel exact supprime la question.

## Conséquences

**Un aller-retour image → millisecondes → image est exact** tant que la fréquence
ne change pas. À 60 images par seconde, une image dure 16,67 ms : la grille des
millisecondes est plus fine que celle des images, donc rien ne se perd. C'est ce
qui rend sûr d'unifier la représentation interne sur les millisecondes, y compris
pour un format nativement en images comme MicroDVD en phase 9.

**L'aller-retour inverse ne l'est pas** — millisecondes → image → millisecondes
perd la précision infra-image. C'est inhérent, pas un défaut de conception, et
la spec de la phase 1 l'énonce comme comportement attendu et testé.

Confondre une durée et une position devient une erreur de compilation. C'est la
faute classique dans un éditeur de sous-titres, et elle disparaît.

En contrepartie, chaque conversion doit être écrite. C'est voulu : une conversion
implicite est une décision d'arrondi que personne n'a prise.

La saturation à `99:59:59.999` de Gaupol est une contrainte de **formatage**, pas
de représentation : `int64_t` couvre bien au-delà. Elle s'applique à l'écriture.
