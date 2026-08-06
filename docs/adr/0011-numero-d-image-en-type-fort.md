# 0011 — Typer le numéro d'image plutôt que le laisser entier

**Date :** 2026-08-06
**Statut :** acceptée

## Contexte

[0006](0006-positions-en-millisecondes.md) pose trois types forts —
`Timestamp`, `Duration`, `FrameRate` — et énonce que le numéro d'image n'est
pas une représentation interne, mais « une vue, obtenue par conversion
explicite avec une `FrameRate` ». Il ne dit pas de quel type est cette vue.

La question se pose à l'écriture de l'issue #2, au moment de donner une
signature aux conversions temps ↔ images. Deux réponses se présentent : un
`std::int64_t` nu, ou un quatrième type.

Or les [principes de conception](../principes-de-conception.md) interdisent
« un `int` nu pour représenter une durée, une position, un numéro de sous-titre
ou un indice de piste ». Un numéro d'image est une position. Rendre la vue
entière reviendrait à réintroduire, à la frontière de conversion, exactement la
confusion que les trois autres types éliminent : rien n'empêcherait d'ajouter
un numéro d'image à un compte de millisecondes.

## Décision

Le numéro d'image est un type fort, **`Frame`**, non convertible implicitement
vers un entier ni vers les autres types de position.

Les conversions sont deux fonctions membres explicites de `Timestamp` :
`Timestamp::fromFrame(Frame, FrameRate)` et `Timestamp::toFrame(FrameRate)`.

Contrairement au temps, l'image n'a pas de sous-unité : le même type sert de
position et de compte, et `Frame + Frame` a un sens. C'est la seule différence
de forme avec le couple `Timestamp` / `Duration`.

## Alternatives écartées

- **`std::int64_t` nu** — fidèle à la lettre de [0006](0006-positions-en-millisecondes.md),
  qui ne nomme que trois types, et une surface d'API plus petite. Écarté :
  l'économie est d'une trentaine de lignes, le coût est un trou dans la seule
  garantie que ces types existent pour donner.
- **Deux types, `Frame` et `FrameCount`**, calqués sur `Timestamp` et
  `Duration`. Écarté : la distinction n'a pas de contrepartie physique — une
  image dure une image — et l'arithmétique deviendrait plus lourde que ce
  qu'elle protège.
- **Conversions en fonctions libres** plutôt que membres. Écarté sans grande
  conviction : `position.toFrame(rate)` se lit mieux que `toFrame(position,
  rate)`, et `time/` reste un module unique dont les types se connaissent.

## Conséquences

Confondre une image et une milliseconde devient une erreur de compilation, au
même titre que confondre une durée et une position.

La phase 9 accueille les formats nativement en images — MicroDVD — sans avoir
à choisir un type à ce moment-là.

En contrepartie, un quatrième type à maintenir, et une conversion à écrire
partout où un numéro d'image est affiché ou lu. C'est le prix déjà accepté en
[0006](0006-positions-en-millisecondes.md).

Défaire cette décision toucherait chaque site de conversion. Le déclencheur
serait un usage montrant que `Frame` gêne plus qu'il ne protège — par exemple
une arithmétique d'images assez riche pour que les fabriques nommées deviennent
du bruit.
