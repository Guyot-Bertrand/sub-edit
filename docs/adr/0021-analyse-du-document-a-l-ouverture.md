# 0021 — Une analyse du document, calculée à l'ouverture

**Date :** 2026-08-25
**Statut :** acceptée

## Contexte

La phase 16 apporte une chose que le noyau n'a jamais eue : **un jugement sur un
document**. Jusqu'ici, tout ce qu'il produisait était soit une donnée lue — des
sous-titres, une fréquence déclarée par un conteneur — soit le résultat d'une
opération que l'utilisateur avait demandée. La déduction de fréquence n'est ni
l'un ni l'autre : personne ne l'a demandée, et elle ne figure dans aucun
fichier. Elle est **inférée**.

Deux questions se posent ensemble, et les séparer donnerait deux mauvaises
réponses.

**Où vit une inférence ?** `core/time/` porte les primitives — `Timestamp`,
`Duration`, `Frame`, `FrameRate`, `Ratio` — et une déduction n'en est pas une :
elle prend un document et rend un avis. `core/model/` porte ce qu'un document
*est*, et un avis n'en fait pas partie. `core/edit/` porte ce qui le *change*,
et une inférence ne change rien.

Il existe déjà un précédent qu'on n'a pas su nommer : `core/model/anomaly.hpp`
est une inférence, elle aussi — « ce sous-titre finit avant de commencer » n'est
pas une donnée du fichier, c'est un jugement porté sur lui. Elle a atterri dans
le modèle faute d'un meilleur endroit.

**Quand se calcule-t-elle ?** À l'ouverture, ou à la demande ? La question n'est
pas de goût : elle décide ce que les surfaces ont le droit de montrer sans rien
réclamer à l'utilisateur, et elle pose un coût sur le chemin de lecture, qui est
mesuré depuis la phase 1.

## Décision

**Un répertoire `core/analysis/`, et la déduction calculée à l'ouverture, sous
un budget explicite.**

`core/analysis/` accueille ce qui **observe** un document sans le changer ni le
décrire : la déduction de fréquence aujourd'hui, les anomalies le jour où on les
déplacera. Une fonction d'analyse est pure, ne connaît ni `Project`, ni
`Session`, ni l'historique, et ne dépend que des primitives de `core/time/` et
des types de valeur du modèle.

**La déduction tourne à l'ouverture du fichier.** Son budget est **un cinquième
du coût de la lecture** — soit 0,5 ms pour 4000 sous-titres, la lecture en
coûtant 2,5. Le budget est vérifié au benchmark, et non espéré.

## Pourquoi à l'ouverture

Parce que **les trois surfaces qui la montrent la montrent sans être
sollicitées** : la barre d'état porte le verdict en permanence, le dialogue de
conversion pré-remplit son entrée au moment où il s'ouvre, et `inspect` la
rapporte dans sa sortie ordinaire. Une déduction paresseuse rendrait chacune de
ces trois surfaces responsable de la déclencher, donc de décider quand — et
trois décisions séparées sur le même sujet finissent par diverger.

Le coût est réel et il est petit. La méthode est une passe sur les débuts, huit
fois, et une somme de vecteurs : elle est du même ordre que la lecture, en
nettement plus simple. Si le budget ne tenait pas, la réponse serait de rendre
la déduction paresseuse derrière un cache porté par le document — et c'est
précisément parce que cette porte de sortie existe qu'on peut commencer par le
choix simple.

## Alternatives écartées

**Ranger la déduction dans `core/time/`.** C'est là que vit `FrameRate`, et la
tentation est grande. Mais `core/time/` est un ensemble de primitives sans
mémoire : chacune répond à une question fermée sur une valeur. Une déduction
répond à une question ouverte sur une collection, et l'y mettre ferait de
`core/time/` un endroit où l'on range ce qui parle de temps — c'est-à-dire à peu
près tout.

**Ranger la déduction dans `core/model/`, avec les anomalies.** Ce serait
cohérent avec l'existant, et c'est justement le problème : l'existant est un
défaut qu'on reproduirait. Le modèle décrit ce qu'un document est ; une anomalie
et une déduction disent ce qu'on en pense. Créer `core/analysis/` maintenant
donne un endroit à ce déplacement, sans l'imposer aujourd'hui — déplacer
`anomaly.hpp` toucherait la table, la CLI et leurs tests pour un gain nul tant
qu'il n'y a qu'une seule analyse.

**La calculer à la demande.** Elle ne coûterait rien à l'ouverture, et chaque
surface paierait la première fois. Écartée pour la raison ci-dessus : trois
surfaces, trois décisions, une divergence. Et le gain se chiffre à une fraction
de milliseconde sur une opération qui en coûte déjà deux et demie.

**La calculer à l'ouverture et la ranger dans le document.** C'est la variante
qui reviendra si le budget ne tient pas. Elle est écartée aujourd'hui parce
qu'elle fait entrer dans le modèle une valeur dérivée, donc une invalidation à
tenir : toute édition d'une position la périme. Une fonction pure qu'on rappelle
n'a pas ce problème, et tant qu'elle est assez rapide pour être rappelée, elle
est le choix sûr.

## Conséquences

**Un répertoire de plus dans le noyau**, et une place pour ce qui n'en avait
pas. `check-architecture.sh` n'a rien à y changer : l'invariant qui compte —
`subedit_core` libre de toute dépendance d'interface — vaut pour lui comme pour
les autres.

**Un coût sur le chemin de lecture**, petit et mesuré. `lecture de 4000
sous-titres` est le relevé qui le porte, et c'est lui qui dira si le budget est
tenu. Le journal des mesures rend ce contrôle continu plutôt que ponctuel.

**Une invalidation qu'on n'a pas à tenir**, tant que la déduction reste une
fonction pure. C'est ce qui rend le choix réversible : passer au cache est une
addition, pas une réécriture.

**Un déplacement placé et non fait.** `core/model/anomaly.hpp` a désormais une
destination écrite. Le faire demande une seconde analyse pour en valoir la
peine ; le jour où elle existe, la question est déjà tranchée.

> **Fait depuis, par la #227.** La phase 16 a posé la déduction de fréquence et
> la correction de grille : la seconde analyse existe, donc la condition écrite
> ci-dessus est remplie. `anomaly.hpp` et `anomaly.cpp` vivent dans
> `core/analysis/`, les appelants ont suivi, et rien de leur comportement n'a
> changé. La décision, elle, n'est pas revenue en discussion — c'est le propre
> d'un déclencheur écrit à l'avance.

Le déclencheur qui rouvrirait cette décision : le budget d'un cinquième non
tenu, ou une seconde analyse assez coûteuse pour qu'aucune ne puisse plus tourner
à l'ouverture. Aucun des deux n'est en vue.
