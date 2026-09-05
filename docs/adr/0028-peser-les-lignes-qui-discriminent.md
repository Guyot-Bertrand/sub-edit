# 0028 — La détection ne pèse que les lignes qui discriminent

Statut : acceptée — 2026-09-05
Décidée en mesurant la détection, issue #310.

## Contexte

L'[ADR 0027](0027-icu-pour-les-encodages.md) a choisi ICU pour détecter et pour
convertir, sur un banc d'essai où elle rendait 9/9. Ce banc ne portait que des
fichiers **monolingues** : chaque fixture est écrite d'un bout à l'autre dans un
seul alphabet.

Un fichier de sous-titres réel ne l'est pas toujours. Un film russe sous-titré
porte des cartons anglais ; un fansub porte une note de traducteur ; un
sous-titrage anglais porte trois apostrophes typographiques déposées par un
traitement de texte. Dans tous ces cas, **la quasi-totalité du fichier est de
l'ASCII**, et le peu qui ne l'est pas est ce qui porte toute la question.

L'issue #310 demandait de mesurer où la détection cesse d'être fiable. La mesure
— `score-encoding-detection.py --par-longueur`, et le relevé de
[`docs/mesures/detection-d-encodage.md`](../mesures/detection-d-encodage.md) —
a répondu autre chose que la question posée.

## Ce qui a été mesuré

Sur des fichiers SubRip engendrés, soixante tirages par ligne :

| Écrit en | Répliques | Accentuées | Octets | Exact | Écriture conservée |
| :------- | --------: | ---------: | -----: | ----: | -----------------: |
| KOI8-R | 200 | 3 | 13 195 | **0 %** | **0 %** |
| KOI8-R | 600 | 10 | 39 724 | **0 %** | **0 %** |
| CP1250 | 600 | 1 | 39 815 | 0 % | 63 % |

Soixante fois sur soixante, un fichier de quarante kilooctets dont dix
répliques sont cyrilliques revenait en `ISO-8859-1` : le russe en latin
accentué, en entier, et le diagnostic « an encoding nothing declared » annonçant
`ISO-8859-1` avec l'aplomb qu'il aurait eu à annoncer le bon encodage.

**Ce n'était pas une limite de l'information disponible**, et c'est ce qui
transforme une fatalité en défaut. Sur le même fichier :

| Détecteur | Réponse |
| :-------- | :------ |
| la nôtre, ICU | `ISO-8859-1` |
| `uchardet`, témoin | `KOI8-R` |

Le classement d'ICU explique le reste. Elle place `ISO-8859-1` à une confiance
de 66, étiquetée « en », et `KOI8-R` à 2, dernière des six candidates. Son
détecteur note des statistiques de lettres sur l'ensemble des octets : treize
mille octets d'anglais y pèsent, et les cent quatre-vingt-dix octets de russe
n'y pèsent rien. La règle « le premier candidat qui décode réellement » n'y peut
rien — un mono-octet décode presque tout, donc le premier du classement gagne
toujours.

## Décision

**ICU ne reçoit que les lignes qui portent un octet hors ASCII.** Le reste du
fichier lui est retiré avant qu'elle le pèse.

Le raisonnement tient en une phrase : **une ligne d'ASCII pur se lit de la même
façon sous toutes les candidates, donc elle ne peut pas les départager.** Ce
qu'elle peut faire, et qu'elle faisait, c'est peser dans une statistique où elle
n'a rien à dire. Ce n'est pas une heuristique de plus ; c'est le retrait de ce
qui n'est pas de l'information.

Le même fichier, ses lignes d'ASCII retirées, place `KOI8-R` à 54 et ne propose
plus `ISO-8859-1` du tout.

**Ce qui est pesé n'est pas ce qui est décodé.** Le classement se fait sur les
lignes retenues ; chaque candidate est ensuite éprouvée **contre le fichier
entier**, parce qu'un encodage qui n'en lit pas la totalité n'est pas une
réponse. Cette règle-là ne bouge pas.

**Deux sorties rendent les octets intacts**, et chacune ferme une façon de nuire
plutôt que d'aider :

- **un NUL quelque part.** C'est un encodage large — UTF-16 avant tout — où
  `0x0a` est une moitié d'unité de code, donc y couper tranche un caractère et
  non une fin de ligne ; et où une ligne d'ASCII pur est une suite de `X\0X\0`
  qui ne porte aucun octet haut et disparaîtrait tout entière. Mesuré : sans
  cette sortie, un fichier UTF-16LE sans BOM était rendu `UTF-16BE`.
- **rien de retiré.** Un fichier accentué de bout en bout n'a pas de masse
  d'ASCII à retirer, et le reconstruire ne ferait que normaliser ses fins de
  ligne pour rien.

## Ce que la décision coûte, écrit plutôt que tu

**Une passe de plus sur les octets, et une copie.** Elle n'a lieu que dans la
branche où les octets ne sont pas de l'UTF-8 valide, c'est-à-dire jamais pour la
grande majorité des fichiers. Le banc ne la voit pas.

**Les fichiers courts perdent leurs horodatages, qui étaient du volume.** Sur
les fichiers entièrement accentués de moins de six répliques, retirer les lignes
d'index et de temps divise la taille pesée par trois, et le taux d'exactitude du
CP1250 recule de quelques points — 92 % à 85 % sur cinq répliques, dans le bruit
de soixante tirages mais dans le mauvais sens. **L'écriture, elle, reste juste à
100 %** : ce qui recule est le choix entre deux latins, jamais le choix de
l'alphabet. C'est le prix, et il est payé là où il coûte le moins.

**Le corpus étiqueté ne peut pas mesurer seul ce que cette décision change.**
`koi8-r-rare.srt` a été ajouté pour cela — vingt répliques d'anglais et une de
russe, la seule fixture dont la réponse bouge d'avant à après.

## Alternatives écartées

**Remplacer ICU par `uchardet`.** Elle répond juste ici, et l'ADR 0027 avait
déjà noté qu'elle détecte aussi bien. Écartée pour la raison qui l'avait déjà
fait écarter : elle ne convertit pas, donc elle s'ajouterait à ICU au lieu de la
remplacer — une dépendance de plus pour un cas que trente lignes traitent.

**Poser un seuil de longueur, et rendre `nullopt` en deçà.** C'est ce
qu'envisageait #310, et la mesure l'écarte deux fois. Le défaut réel est sur des
fichiers de quarante kilooctets, qu'aucune borne inférieure sur la taille ne
voit. Et la détection n'est consultée que lorsque les octets ne sont **pas** de
l'UTF-8 valide : rendre `nullopt` fait retomber la lecture sur l'UTF-8, qui
échoue par construction, donc le fichier ne s'ouvrirait plus du tout. Une
réponse imparfaite qu'un diagnostic annonce vaut mieux qu'un refus.

**Peser les deux et garder la meilleure confiance.** ICU rend une confiance par
candidate, et on pourrait comparer le classement du fichier entier à celui des
lignes retenues. Mesuré : sur le fichier bilingue, `ISO-8859-1` sur tout le
fichier vaut 66 et `KOI8-R` sur les lignes retenues vaut 54. La confiance monte
avec le volume pesé, donc cette comparaison redonnerait précisément la mauvaise
réponse. Deux chiffres calculés sur deux échantillons différents ne se comparent
pas.

**Retirer l'ASCII caractère par caractère plutôt que ligne par ligne.** Plus
agressif, et faux : une lettre haute isolée de son mot perd le contexte que les
statistiques de bigrammes d'ICU utilisent. La ligne est l'unité qui garde le mot
entier tout en jetant ce qui n'en est pas un.

## Conséquences

**L'ADR 0027 n'est pas remplacée, elle est complétée.** ICU reste le détecteur
et le convertisseur ; ce qui change est ce qu'on lui soumet.

**Le corpus étiqueté porte désormais un cas bilingue**, et le score se rejoue à
chaque pull request — issue #311. Une régression sur ce point ne peut plus
passer inaperçue : elle fait échouer `make score`.
