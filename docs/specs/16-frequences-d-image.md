# Phase 16 — Fréquences d'image : déduction et correction

**Milestone :** 16 — Fréquences d'image : déduction et correction
**Programmée entre la [6](06-lecteur-integre.md) et la 7.** Son numéro est le
premier libre au moment où elle a été ajoutée ; il l'identifie et ne dit pas son
rang.

## Objectif

Un fichier de sous-titres ne déclare pas la fréquence d'image contre laquelle
ses positions ont été calculées — SubRip n'a pas d'en-tête, celui de WebVTT est
du texte libre. Cette phase la **déduit des positions elles-mêmes**, et apporte
les deux opérations qui gravitent autour : ramener un fichier sur sa grille, et
l'aligner sur une autre.

## Ce qui la distingue

**Elle dépasse l'iso-fonctionnalité, et c'est la première.** Gaupol ne déduit
aucune fréquence, et la chaîne tient en trois sauts : `framerate_convert.py`
pré-remplit ses **deux** listes avec `page.project.framerate` ; `page.py` donne
cette valeur au projet depuis `conf.editor.framerate` à l'ouverture ; et
`config.py` fixe ce défaut à `FPS_23_976`. **Rien, nulle part, ne regarde le
fichier** — c'est une préférence globale, et elle n'a pas bougé en vingt ans.

**Il n'y a donc rien à lire dans `reference/gaupol`**, et l'étape d'analyse
préalable du déroulé d'une phase est vide, pour la première fois. Ce qui la
remplace : les fixtures de #198 et ce qu'elles donnent.

**Elle éprouve un jugement, non un comportement.** Les phases précédentes
éprouvaient ce que le logiciel *fait* — lire, écrire, décaler. Celle-ci éprouve
ce qu'il *conclut* : « ce fichier est calé sur une grille à 23,976, avec une
concentration de 99,9 ». Un jugement se compare à une attente, et l'attente est
désormais versionnée : `src/test/data/grilles/` et son `LISEZMOI.md`.

## D'où vient la méthode

**Convertir une fréquence suppose que les positions étaient calées sur une
grille d'images.** Quand c'est vrai, chaque position vaut `round(n × 1000 / R)`,
et cette grille se mesure.

Pour chacune des huit fréquences normalisées, on calcule la phase de chaque
début sur la grille correspondante, et on mesure **la concentration de ces
phases** — la longueur du vecteur résultant, de 0 à 100. Cent : une grille
parfaite, quelle que soit sa phase. Près de zéro : aucune structure.

Elle n'a **aucun paramètre de tolérance**, et elle est **insensible à un
décalage**. Ce second point a été payé : une première version cherchait une
grille de phase nulle et notait à zéro un fichier qui est un 24 images par
seconde parfait, translaté d'une milliseconde et demie.

## Portée

**Dans la phase :**

- la déduction, et ce qu'elle rend ;
- deux opérations — ramener un fichier sur sa grille, l'aligner sur une
  fréquence choisie ;
- la lecture croisée avec la fréquence que le conteneur vidéo déclare ;
- les surfaces : `inspect`, deux sous-commandes, la barre d'état, une modale
  d'analyse, et le pré-remplissage du dialogue de conversion.

**Hors de la phase :**

| Ce qui est renvoyé | Où |
| :----------------- | :- |
| retrouver la paire d'une conversion faite avec la mauvaise fréquence | phase 10, avec les opérations complémentaires — « le plus utile et le moins sûr » |
| marquer dans la table les positions qui sortent de la grille | nulle part : **D12** l'interdit, et dit pourquoi |
| le traitement par lot et la sortie exploitable par un script | phase 13 |
| l'édition en mode images | phase 9 |

## Décisions applicables

**D1 — la méthode est la concentration de phase, sans tolérance.** Aucun seuil
n'entre dans le calcul ; les seuils n'apparaissent qu'au moment de rendre un
verdict, et une seule fois (**D4**).

**D2 — l'ensemble des candidates est clos et petit : les huit de
`StandardFrameRate`.** Résoudre pour un `R` quelconque est un tout autre
problème, et sans objet — personne ne masterise à 26,3 images par seconde.
`src/test/data/grilles/grille-absurde.srt` est exactement ce fichier-là, et la
bonne réponse à son sujet est « je ne sais pas ».

**D3 — les débuts sont le signal ; les fins ne sont pas mesurées.** Partout où
une grille existe, les débuts y sont sans exception ; les fins vont de 55 à 100,
parce qu'un *cue-out* est souvent calculé par une règle de vitesse de lecture et
non posé sur une image. Les mélanger ferait baisser un signal net avec du bruit
connu.

**D4 — la déduction rend le classement complet des huit, et un verdict calculé
une seule fois.** Le classement ne cache rien ; le verdict — `nette`,
`partielle`, `muette` — évite que `inspect`, la modale d'analyse, la barre
d'état et le croisement avec `ffprobe` réimplémentent chacun les seuils et
finissent par ne plus dire la même chose.

**D5 — entre deux candidates harmoniques, la plus basse.** Une grille à 25 est
**incluse** dans une grille à 50 : un fichier calé sur 25 sort à 100 sur les
deux, et l'inverse est faux — `grille-50.srt` ne donne que 15,8 sur 25.
L'implication ne va que dans un sens, donc la plus basse est l'explication
parcimonieuse.

Ce n'est pas une précaution théorique. Prendre le maximum se trompe, et les
fixtures le montrent :

| Fixture | Calée sur | Ce que le maximum donne |
| :------ | :-------- | :---------------------- |
| `grille-25.srt` | 25 | 25 à 100,0 — **et 50 à 100,0** : pile ou face |
| `grille-30.srt` | 30 | 30 à 99,9, 60 à 99,5 — juste par chance |
| `melange-groupe.srt` | 29,97 | **59,94 à 66,4 contre 65,1 — faux** |

Les seules paires ambiguës de l'ensemble normalisé sont celles dont l'une est le
multiple entier de l'autre : 25 et 50, 29,97 et 59,94, 30 et 60. Aucune autre —
24 et 60 sont dans un rapport de deux et demi, donc une image sur deux seulement
coïncide. **L'ambiguïté est dite, jamais cachée** : la déduction nomme la
candidate écartée.

**Quand la paire compte-t-elle comme ambiguë ?** Précisé par #203, qui a dû
l'écrire : on compare les **bandes**, pas un écart. Un fichier réellement calé
sur la fréquence haute fait s'effondrer la basse vers le bruit, donc les deux
tombent dans des bandes différentes ; un fichier calé sur la basse les met dans
la même, quoi que l'arrondi ait fait de leur ordre. La règle ne demande alors
aucun seuil neuf — elle réutilise ceux du verdict — et elle tient sur les huit
cas que les fixtures offrent :

| Fixture | La basse | La haute | Même bande ? | Retenue |
| :------ | -------: | -------: | :----------- | :------ |
| `grille-25.srt` | 100,0 | 100,0 | oui | 25 |
| `grille-30.srt` | 99,9 | 99,5 | oui | 30 |
| `grille-29-97.srt` | 99,8 | 99,4 | oui | 29,97 |
| `grille-50.srt` | 15,8 | 100,0 | non | 50 |
| `grille-60.srt` | 1,3 | 99,5 | non | 60 |
| `grille-59-94.srt` | 4,8 | 99,4 | non | 59,94 |
| `melange-groupe.srt` | 65,1 | 66,4 | oui | 29,97 |
| `melange-disperse.srt` | 75,4 | 84,9 | oui | 25 |

Un écart chiffré n'aurait pas tenu : les deux dernières lignes sont séparées de
1,3 et de 9,5 points, et la quatrième de 84. Aucune marge ne passe entre 9,5
et 84 sans être choisie après coup pour ça.

**D6 — l'étendue est rendue, et les candidates qu'elle ne sépare pas sont
nommées.** 23,976 et 24 dérivent d'une milliseconde par seconde de film, et une
image en dure 41,7 : la phase sur la mauvaise candidate fait un tour complet
toutes les quarante-deux secondes. Sur un long métrage, elles se séparent sans
appel ; sur dix secondes, non — `grille-24-courte.srt` donne 24 à 99,9 et 23,976
à 92,7. Rendre « 24, confiance 99,9 » serait mentir par omission.

**D7 — un fichier partiel rend quels débuts sont hors grille.** Dire « les deux
tiers de vos débuts sont sur une grille à 29,97 » est vrai et n'aide personne.
La déduction rend donc, pour la candidate retenue, les indices des débuts qui
s'en écartent de plus d'une milliseconde, et la présentation en tire la forme.
Le nombre de suites consécutives suffit à distinguer les deux causes, et il ne
coûte qu'une passe :

| Fixture | Hors grille | Suites | Plus longue |
| :------ | ----------: | -----: | ----------: |
| `melange-disperse.srt` — édition à la main | 31 / 171 | **31** | **1** |
| `melange-groupe.srt` — section recalée | 52 / 168 | **5** | **27** |

**D8 — la phase est rendue, en millisecondes, et n'est jamais corrigée en
silence.** Un fichier dont les positions sont sur la grille à une constante près
a été décalé, et cette constante se mesure. La corriger est une opération que
l'utilisateur demande (**D10**), pas un effet de bord de la lecture.

**D9 — la déduction tourne à l'ouverture du fichier.** C'est ce qui permet à la
barre d'état, au dialogue de conversion et à `inspect` de la montrer sans que
l'utilisateur la réclame. Elle est donc **sur le chemin de lecture**, qui coûte
2,5 ms pour 4000 sous-titres : son budget est **un cinquième de la lecture**,
vérifié au benchmark et non espéré. Au-delà, elle devient paresseuse.

**Mesuré par #203 : 374 µs** au relevé de la version 0.6.3, contre 2,32 ms pour
la lecture du même relevé — **16 %, le budget est tenu.**

**La fenêtre va au-delà de « à l'ouverture », et c'est voulu.** Elle recalcule à
chaque changement du document, parce qu'un alignement change la grille et que la
barre d'état ne doit pas continuer à annoncer l'ancienne. C'est ce que
l'[ADR 0021](../adr/0021-analyse-du-document-a-l-ouverture.md) rend possible en
refusant de garder une valeur dérivée : il n'y a aucune invalidation à tenir, et
le coût reste celui d'une fraction de milliseconde.

Il ne l'était pas au premier jet. Le gain vient d'une propriété et non d'une
astuce : une position est un nombre entier de millisecondes, donc sur une
fréquence entière la phase ne prend que mille valeurs, et cinq des huit
candidates se répondent alors par une table **exacte**, pas approchée. Mesuré
dos à dos, elle divise le coût par deux.

**D10 — deux opérations, et elles ne font pas la même chose.**

| Opération | Ce qu'elle fait | Ce qu'elle préserve |
| :-------- | :-------------- | :------------------ |
| **ramener sur la grille** | décale tout le fichier de la phase mesurée | le minutage relatif, **exactement** |
| **aligner sur une fréquence** | porte chaque horodatage — début *et* fin — sur l'image la plus proche de la fréquence choisie | la position de chaque réplique, à une demi-image près |

Sur un fichier parfaitement calé, les deux donnent le même résultat : si la
phase est inférieure à une demi-image, porter chaque horodatage sur l'image la
plus proche revient à décaler l'ensemble de la même quantité. **Elles divergent
sur un fichier partiel**, et c'est là qu'est leur raison d'être : la première
laisse les écarts où ils sont, la seconde les absorbe. `melange-disperse.srt`
est le fichier qui montre la différence.

**D11 — aligner n'est pas convertir, et c'est le piège de cette phase.**

| | Convertir | Aligner |
| :--- | :-------- | :------ |
| formule | `t' = t × R_entrée / R_sortie` | `t' = round(t × R / 1000) × 1000 / R` |
| déplacement | proportionnel au temps — **plusieurs secondes** sur un long métrage | une demi-image au plus, **partout** |
| pour quel fichier | un fichier dont le minutage est **faux** | un fichier dont le minutage est **juste** et dont la grille est fausse |

Le cas d'usage de l'alignement, en toutes lettres : une vidéo à 25 images par
seconde, un fichier de sous-titres écrit sur une grille à 24, des répliques déjà
à peu près à leur place à quelques millisecondes près. Il n'y a rien à
re-minuter — seulement à reposer les horodatages sur la bonne grille.

Se tromper de l'une pour l'autre est le pire mode d'échec de la phase : les deux
acceptent les mêmes arguments et l'une décale le fichier entier sans rien
signaler. **Le manuel et l'aide en ligne les opposent explicitement**, l'un dans
la section de l'autre.

**D12 — la déduction ne marque jamais une ligne.** La phase 5 s'interdit
d'utiliser la grille pour marquer quoi que ce soit dans la table, et cette
interdiction tient. Dès qu'un utilisateur corrige une position à la main, elle
cesse d'être alignée : un détecteur naïf signalerait son travail comme une
anomalie.

**Une teinte de ligne existe pourtant depuis #211**, et elle ne vient pas
d'ici : c'est la réplique que le film montre à l'instant. Elle est transitoire,
elle suit une lecture et non une mesure, et une anomalie l'emporte sur elle. D12
interdit à la *déduction* de marquer une ligne, et cela reste vrai. `melange-disperse.srt` est précisément ce fichier, et accuser ses
trente-et-une lignes serait accuser l'utilisateur. **La déduction parle du
document, jamais d'une ligne** — sauf dans la modale d'analyse, que
l'utilisateur ouvre pour cela.

**D13 — un désaccord entre les deux sources est montré, et rien n'est choisi.**
Le conteneur *déclare* une fréquence (phase 6, `ffprobe`), les positions en
*trahissent* une autre. Ce ne sont pas la même donnée : la première dit à quelle
cadence le film tourne, la seconde sur quelle grille le fichier a été écrit. Un
fichier à 24 sur un film à 25 n'est pas une contradiction, c'est le cas d'usage
de l'alignement. La phase 6 avait posé la règle par avance — « une source qui
s'impose interdit de croiser » (D6 de sa spec) — et elle s'applique ici à la
troisième source.

**D14 — la déduction vit dans `core/analysis/`.** Ce n'est pas une primitive de
temps comme `FrameRate`, `Timestamp` ou `Ratio` : c'est une observation sur un
document. Le répertoire est neuf et accueillera plus tard les anomalies de
`core/model/anomaly.hpp`, qui sont la même famille — ce déplacement n'est pas de
cette phase. *(Fait depuis, hors phase, par la #227.)*

## Le noyau

### `core/analysis/frame_rate_deduction.hpp`

Une fonction pure sur les positions, sans état. Elle ne dépend que de
`core/time/` — `Duration`, `FrameRate` — et de `SubtitleIndex`, qui est un type
de valeur du modèle et non une opération. **Elle ne connaît ni `Project`, ni
`Session`, ni l'historique** : elle reçoit des positions et rend un jugement, ce
qui est exactement ce qui la rend éprouvable sur treize fichiers et une table de
constantes.

```cpp
struct GridFit {          // une candidate, et ce qu'elle vaut
    FrameRate rate;
    double concentration; // 0 à 100
    Duration phase;       // le décalage constant, en millisecondes
};

enum class GridVerdict { Clean, Partial, Silent };

struct FrameRateDeduction {
    std::array<GridFit, 8> ranked;         // décroissant, D4
    GridFit retained;                      // la réponse, après D5
    GridVerdict verdict;                   // D4
    std::optional<FrameRate> harmonic;     // la candidate écartée par D5
    std::vector<FrameRate> notSeparated;   // ce que l'étendue ne sépare pas, D6
    Duration span;                         // l'étendue vue, D6
    std::size_t starts;                    // combien de débuts l'ont donnée
    std::vector<SubtitleIndex> strays;     // les débuts hors grille, D7
};
```

**`retained` n'est pas `ranked.front()`, et les deux sont exposés.** C'est ce que
D5 impose : le classement reste l'ordre honnête des concentrations, la réponse
est celle que la règle harmonique désigne. Les confondre obligerait soit à
mentir sur l'ordre, soit à refaire la règle chez chaque appelant.

Les seuils — 90 pour `Clean`, 50 pour `Partial` — sont écrits **une seule fois**,
ici, et nulle part ailleurs. C'est la conclusion de #197 : le score d'un couple
fichier × fréquence est une constante de test, pas un matcher.

### Les deux opérations

Elles entrent dans l'historique comme les autres, par `Session::apply`, et sont
annulables.

**Ramener sur la grille** est un `ShiftCommand` dont le montant vient de la
mesure plutôt que de l'utilisateur. Aucune commande neuve : c'est un décalage,
et l'écrire autrement dupliquerait un comportement déjà éprouvé — y compris
l'avertissement de dépassement de la fin du film (phase 6, D4 de sa spec). Elle
**n'est pas disponible quand le verdict est `Silent`** : il n'y a pas de grille
à rejoindre.

**Aligner sur une fréquence** est une commande neuve, `SnapCommand`. Elle porte
chaque horodatage — début et fin — sur l'image la plus proche de la fréquence
donnée. La fréquence est choisie par l'utilisateur et **pré-remplie par celle
que la vidéo déclare**, non par celle que la déduction trouve : l'intention est
de rejoindre la grille du film, et la déduction dit celle qu'on quitte.

Deux propriétés à tenir, et à éprouver :

- **idempotence** — appliquer deux fois la même fréquence ne change rien la
  seconde fois ;
- **l'ordre est préservé** — la quantification est monotone, donc deux débuts
  distants d'au moins une image le restent. Deux débuts plus proches qu'une
  image peuvent se confondre, et c'est une anomalie que le noyau sait déjà
  nommer.

## La ligne de commande

`inspect` gagne une section, et deux sous-commandes apparaissent.

| Surface | Ce qu'elle porte |
| :------ | :--------------- |
| `inspect` | la candidate retenue, sa concentration, l'étendue, la phase, le verdict, et l'ambiguïté harmonique s'il y en a une |
| `shift --to-grid` | ramener sur la grille ; le montant vient de la mesure, et il est écrit avant d'être appliqué |
| `snap --rate <R>` | aligner sur une fréquence ; écrit combien de positions ont bougé et de combien au plus |

`--rate` accepte les six formes déjà acceptées par `framerate` (phase 2), et
**n'importe quelle cadence valide**, pas seulement les huit normalisées.

> **Correction, par #207.** Cette ligne disait l'inverse — « `snap` refuse une
> fréquence hors des huit normalisées, comme `framerate` » — et se trompait deux
> fois. `framerate` ne les refuse pas ; et l'ensemble clos des huit appartient à
> la **déduction**, qui doit choisir parmi des candidates, non à une opération
> dont l'utilisateur nomme lui-même la cible. Restreindre `snap` aurait ajouté
> une règle sans bénéfice et incohérente avec l'opération qu'elle jouxte.
>
> L'exigence `CLI-SNAP-03` qui en découlait est donc **retirée** avant d'avoir
> été écrite, plutôt que rendue vraie de force.

Un fichier muet fait dire à `inspect` qu'il n'a trouvé aucune grille, **et non
une fréquence à faible confiance** : c'est le sens de D2, et c'est la propriété
qui autorise à se servir de la déduction.

## La fenêtre

| Surface | Ce qu'elle porte |
| :------ | :--------------- |
| barre d'état | le verdict, à côté de `Video: film.mkv` — par exemple `Grid: 23.976 fps` ou `No grid` |
| `Frame Rate Analysis…` | la modale d'analyse : le classement des huit, l'étendue, la phase, les débuts hors grille et leur forme |
| `Convert Frame Rate…` | **l'entrée est désormais pré-remplie par la mesure**, et sa provenance est dite |
| `Shift onto Grid` | l'opération de D10, montant affiché avant d'être appliqué |
| `Snap to Frame Rate…` | l'opération de D10, fréquence pré-remplie par celle que la vidéo déclare |

Le dialogue de conversion change de nature, et le manuel d'interface l'annonce
déjà : il écrit aujourd'hui que le dialogue pré-remplit « la sortie, jamais
l'entrée », parce que « le fichier a été calé contre une fréquence que lui seul
connaît ». C'est exactement ce que cette phase apporte. **La ligne devient donc
une mesure, pas une préférence** — et elle reste modifiable, par D13.

Quand la mesure et ce que la vidéo déclare divergent, **les deux s'affichent** :
l'entrée porte la mesure, la sortie porte ce que le film annonce, et rien n'est
choisi à la place de l'utilisateur.

**Des corrections cosmétiques de la fenêtre seront intégrées à cette phase**,
listées avant la clôture. Elles n'ont pas de rapport avec la déduction et sont
regroupées ici parce que la phase touche déjà l'interface.

## Tests

Les fixtures existent : `src/test/data/grilles/`, treize fichiers, et leur
`LISEZMOI.md` porte les deux tables — ce que chacune est, et ce que chacune
donne sur les huit candidates.

| Ce qui est éprouvé | Sur quoi |
| :----------------- | :------- |
| la bonne candidate sort | les huit `grille-<R>.srt` |
| l'échec est bruyant | `grille-absurde.srt` — sous 16 partout |
| la phase n'est pas un déchet | `grille-24-decalee.srt` — 99,9 malgré 2 999 ms |
| l'ambiguïté harmonique et D5 | `grille-25.srt`, `grille-30.srt`, `melange-groupe.srt` |
| l'étendue et D6 | `grille-24-courte.srt` — 23,976 à 92,7 contre 99,9 |
| le partiel, ses deux causes et D7 | `melange-groupe.srt`, `melange-disperse.srt` |
| l'idempotence de l'alignement | toutes, appliquées deux fois |
| aligner n'est pas convertir | `grille-24.srt` aligné sur 25 bouge de moins d'une demi-image ; converti, de plusieurs secondes |

Les concentrations attendues sont des **constantes dans le code de test**,
reprises du `LISEZMOI.md`. Aucun matcher : neuf fixtures et huit candidates font
une table de nombres, et une table se lit mieux qu'un verbe.

Ce que les fixtures ne portent pas encore, et qu'il faudra leur ajouter si un
test le demande : des fins calculées par une règle de vitesse de lecture, et un
fichier écrit en millisecondes sans aucune grille.

## Mesures

| Mesure | Pourquoi |
| :----- | :------- |
| la déduction sur 4000 sous-titres | le budget de D9, un cinquième de la lecture |
| l'alignement sur 4000 sous-titres | une opération de plus, à comparer aux quatre existantes |

`lecture de 4000 sous-titres` porte déjà le chemin sur lequel D9 pose la
déduction : c'est ce relevé-là qui dira si le budget est tenu.

## Exigences

| Identifiant | Ce qu'il promet |
| :---------- | :-------------- |
| `CLI-INSPECT-05` | `inspect` écrit la grille déduite, sa concentration, l'étendue et le verdict |
| `CLI-INSPECT-06` | un fichier sans grille le dit, et ne nomme aucune fréquence |
| `CLI-INSPECT-07` | une ambiguïté harmonique est nommée, et la plus basse est retenue |
| `CLI-SHIFT-03` | `shift --to-grid` décale du montant mesuré, et l'écrit |
| `CLI-SHIFT-04` | `shift --to-grid` sur un fichier sans grille refuse et dit pourquoi |
| `CLI-SNAP-01` | `snap --rate R` porte chaque position sur l'image la plus proche de R |
| `CLI-SNAP-02` | `snap` écrit combien de positions ont bougé, et de combien au plus |
| `GUI-GRID-01` | la barre d'état porte le verdict de la déduction |
| `GUI-GRID-02` | la modale d'analyse montre les huit candidates, l'étendue et les écarts |
| `GUI-FRAMERATE-03` | le dialogue de conversion pré-remplit l'entrée avec la mesure, et le dit |
| `GUI-FRAMERATE-04` | un désaccord entre la mesure et ce que la vidéo déclare est montré sans être arbitré |
| `GUI-SNAP-01` | `Snap to Frame Rate…` aligne le document, et l'opération s'annule |
| `GUI-GRID-03` | ramener sur la grille affiche son montant, et s'éteint sans grille |

**Douze, et non treize.** `CLI-SNAP-03` a été retirée par #207 avant d'être
écrite — la raison est plus haut — et `GUI-GRID-03` est née en #210, quand
l'entrée de menu a dû dire son montant avant de l'appliquer. Les deux écarts
sont inscrits ici plutôt que corrigés en silence, et la relecture de fin de
phase (#222) a confronté cette table au registre ligne à ligne.

**Elles entrent au registre en début d'issue, à l'état `prévue`.** Le cadrage de
la phase 6 renvoyait ici la question : la règle le dit, la pratique ne l'a
jamais suivie sur une phase entière, et il fallait trancher. **La règle est
gardée**, parce que cette phase est la première où elle sert vraiment : les
treize lignes ci-dessus sont écrites *avant* le code et décident ce qui sera
fait, là où les sept de la phase 6, entrées à la fin, ne faisaient que décrire.
Si elle n'est pas tenue cette fois non plus, c'est la règle qu'il faudra retirer
et non la pratique qu'il faudra excuser.

## Manuel

| Section | Ce qui change |
| :------ | :------------ |
| `subedit-cli/inspect.md` | la section de déduction, avec un exemple réel |
| `subedit-cli/shift.md` | `--to-grid` |
| `subedit-cli/snap.md` | la sous-commande, **et son opposition explicite à `framerate`** (D11) |
| `subedit-cli/framerate.md` | le renvoi symétrique vers `snap` |
| `subedit-gui/operations.md` | les deux opérations, et le dialogue de conversion qui change de nature |
| `subedit-gui/table.md` ou une section neuve | la modale d'analyse et la barre d'état |

**Une question de manuel reste ouverte, renvoyée par #198 :** un exemple de
`inspect` a besoin d'un fichier calé sur une grille, et les exemples s'exécutent
dans un répertoire temporaire vide où l'entrée vient d'un `printf`. Deux
répliques y tiennent, mille non. Deux voies — une boucle de shell dans
l'exemple, auto-suffisante mais qui montre un générateur avant de montrer une
commande, ou une fixture versionnée, qui montre une commande sur un fichier que
le lecteur n'a pas. **À trancher dans l'issue qui écrit la section**, avec la
section sous les yeux.

## Découpage en issues

| Ordre | Issue | Ce qu'elle porte |
| ----: | :---- | :--------------- |
| 1 | #203 — Noyau : la déduction, son classement et son verdict | D1 à D8 et D14, `core/analysis/`, les seuils écrits une fois |
| 2 | #204 — Noyau : aligner les positions sur une fréquence | `SnapCommand`, D10 et D11, idempotence et ordre préservé |
| 3 | #205 — Noyau : ramener un fichier sur sa grille | le décalage dont le montant vient de la mesure |
| 4 | #206 — CLI : `inspect` dit la grille du fichier | D2, D5, D6, et le fichier muet qui le dit |
| 5 | #207 — CLI : aligner, ramener sur la grille | les deux sous-commandes, et l'opposition à `framerate` |
| 6 | #208 — Interface : le verdict et la modale d'analyse | la barre d'état, D7 sous forme lisible, D12 |
| 7 | #209 — Interface : la mesure dans le dialogue de conversion | D13, et le désaccord montré |
| 8 | #210 — Interface : aligner, ramener sur la grille | les entrées de menu, l'annulation |
| 9 | #211 — Interface : corrections cosmétiques | à lister avant la clôture |

#203 précède toutes les autres. #204 et #205 ne dépendent que d'elle et peuvent
aller en parallèle ; #206 et #207 dépendent des trois ; #208, #209 et #210
dépendent de tout ce qui précède. **#208 est celle qui rend la phase visible** —
avant elle, la déduction n'existe que pour qui lit une sortie de terminal. #211
ne dépend de rien et se remplit en chemin.

## Renvois

**Répercutés par la relecture de fin de phase (#222)**, qui a vérifié que chacun
désigne quelque chose de réel — une section de la feuille de route ou une issue
ouverte — plutôt qu'une intention.

| Ce qui est renvoyé | Où |
| :----------------- | :- |
| retrouver la paire d'une conversion faite avec la mauvaise fréquence | phase 10, dont la section le nomme désormais |
| des fixtures aux fins réalistes, et un fichier sans aucune grille | phase 10, même section |
| déplacer les anomalies de `core/model/` vers `core/analysis/` | #227 — une condition, pas une date : le jour où une seconde analyse existe |
| le traitement par lot de `snap` et la sortie exploitable | phase 13, dont la liste nomme désormais l'alignement |
| l'entrée `Help ▸ Manual`, présente et éteinte | phase 7, avec l'empaquetage qui décide où le manuel vit |
| la géométrie de la fenêtre, retenue d'une session à l'autre | phase 7, avec la configuration persistée |

**Et quatre axes sortis du regard critique**, qui ne sont pas des renvois de
cadrage mais des dettes constatées :

| Ce qui a été vu | Issue |
| :-------------- | :---- |
| trois appelants rassemblent les débuts avant de déduire | #223 |
| la liste des huit fréquences est écrite deux fois | #224 |
| les seuils ne sont pas éprouvés à leur frontière | #225 |
| la porte ne voit pas les fichiers que les tests laissent derrière eux | #226 |

## Critères de fin

- [ ] Les neuf issues sont fermées
- [ ] Les exigences du registre sont `implémentées` et citées par un test
- [ ] Les sections de manuel existent et décrivent ce qui existe
- [ ] Les benchmarks sont rejoués, et le budget de D9 est tenu ou renégocié
- [ ] La relecture de fin de phase est faite
