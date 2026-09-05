# Détection d'encodage

Ce que la détection du projet répond, sur les deux corpus qui peuvent lui être
soumis, et **où elle cesse de savoir**.

**Un taux, et non « la détection marche ».** Une détection est un classifieur :
elle a un taux de succès, pas un résultat juste ou faux — c'est ce que l'issue
#290 a inscrit dans `score-encoding-detection.py`, avant qu'il y ait une
détection à nous. Il y en a une depuis l'issue #296, et voici ses chiffres.

## Comment le rejouer

```console
$ make score
```

C'est une étape de `check-local` depuis l'issue #311 : le score se rejoue à
chaque pull request, et **une divergence d'avec le relevé ci-dessous se voit**.
Un score qui baisse échoue, un score qui monte invite à `make score-record`, un
score inchangé se tait. Ce n'est pas un seuil : le nombre comparé est celui de
la dernière mesure, jamais une barre qu'on aurait posée.

Le reste se lance à la main, et le détecteur est un argument :

```console
$ cmake --build build/dev --target subedit_detect_encoding
$ ./src/scripts/score-encoding-detection.py \
      --detector './build/dev/bin/subedit_detect_encoding {}' --par-longueur --prive
```

`--prive` ajoute le corpus privé, absent de toute machine qui ne l'a pas — le
relevé se lit sans lui, en moins complet. Aucune porte ne le lit.

## Corpus étiqueté

`src/test/data/encodages/`, fabriqué par `encoding-fixtures.py` : la table qui
les écrit dit dans quel encodage chacune l'a été, donc l'étiquette vient avec le
fichier.

<!-- relevé engendré : ne pas modifier à la main -->

    corpus étiqueté : 12/13

Relevé sur la version 0.9.6, le 2026-09-05.

| Fixture | Écrite en | Lue comme |
| :------ | :-------- | :-------- |
| `cp1250-court.srt` | `cp1250` | `cp1252` |

<!-- fin du relevé -->

**Quatre fixtures sont entrées avec l'issue #310**, et le 9/9 est devenu 12/13
sans que le corpus soit devenu plus difficile qu'il n'aurait dû l'être : il
mesurait le cas facile. Neuf fixtures monolingues de cent cinquante à quatre
cents octets ne disaient rien d'une réplique isolée, ni d'un fichier bilingue.

**Trois courtes** — `latin1-court`, `cp1250-court`, `koi8-r-court` — d'une seule
réplique. **Une bilingue**, `koi8-r-rare`, de vingt répliques d'anglais et une
de russe : c'est celle qui a coûté le plus, et la section suivante lui est
consacrée.

**Deux des treize n'ont pas de bonne réponse démontrable.** `latin1.srt`
d'abord — sans octet dans la plage `0x80–0x9f`, Latin-1 et CP1252 sont le même
encodage, et la compter juste est une convention. `cp1250-court.srt` ensuite,
qu'aucun des détecteurs essayés ne lit juste : dix-huit octets de tchèque ne
suffisent pas à séparer l'Europe centrale de l'Europe de l'Ouest. Elle est
comptée fausse, et elle reste — c'est ce qui donne au relevé quelque chose à
mesurer.

## Où la détection cesse de savoir

Le taux ci-dessus est **un** chiffre sur treize fichiers. Il ne dit pas *où* le
cas devient difficile, et l'issue #310 est née de là. `--par-longueur` fabrique
un corpus gradué — graine fixe, soixante tirages par ligne, rien de déposé dans
le dépôt — et le passe au même détecteur.

Trois taux y sont comptés, et ils ne disent pas la même chose. **Exact** : la
réponse nomme l'encodage d'écriture. **Texte** : le fichier relu sous la réponse
est le fichier écrit — ce qui pardonne une confusion sans conséquence.
**Écriture** : le texte relu garde l'alphabet du texte écrit — ce qui ne
pardonne que le mojibake à l'intérieur d'une même famille.

Les deux tables ci-dessous sont relevées sur la version 0.9.6, **après**
l'[ADR 0028](../adr/0028-peser-les-lignes-qui-discriminent.md) ; les chiffres
d'avant sont dans l'ADR, puisque c'est elle qu'ils ont décidée.

### Par longueur — tout le fichier est accentué

Taux d'exactitude, par nombre de répliques :

| Répliques | 1 | 2 | 3 | 5 | 8 | 13 | 21 |
| :-------- | --: | --: | --: | --: | --: | --: | --: |
| Latin-1 | 100 % | 100 % | 100 % | 100 % | 100 % | 100 % | 100 % |
| CP1250 | **20 %** | 53 % | 70 % | 85 % | 93 % | 95 % | 97 % |
| KOI8-R | 100 % | 100 % | 100 % | 100 % | 100 % | 100 % | 100 % |

**L'écriture est juste sur les vingt et une lignes**, à 100 % — pas une réponse
ne sort de la famille latine pour du latin, ni du cyrillique pour du
cyrillique. La falaise que l'issue avait vue est là, sur CP1250, et elle est
d'une autre nature qu'annoncé : c'est **la confusion des latins qui s'aggrave
quand le texte raccourcit**, pas le basculement vers une autre écriture.

**Ce ne sont pas les octets qui manquent, ce sont les octets hauts.** Une
réplique de CP1250 fait cinquante-six octets dont deux au-dessus de `0x7f` ;
une de KOI8-R en fait cinquante-huit dont dix-neuf, et elle est reconnue. Le
cyrillique écrit chaque lettre haut, le tchèque n'y écrit que ses diacritiques.

**Cette colonne-là a reculé de quelques points avec l'ADR 0028**, et c'est le
prix qu'elle nomme : retirer les lignes d'index et d'horodatage divise par trois
ce qui est pesé sur un fichier de cinq répliques. Le choix entre deux latins y
perd, le choix de l'alphabet n'y perd rien.

### À texte rare — un fichier long, presque tout ASCII

C'est l'autre bout, et c'est celui qui a fait l'ADR 0028. Un fichier de deux
cents ou six cents répliques d'anglais, dont quelques-unes seulement portent un
autre alphabet — un film russe sous-titré avec des cartons anglais, une note de
traducteur dans un fansub.

| Écrit en | Répliques | Accentuées | Octets | ≠ ASCII | Exact | Écriture |
| :------- | --------: | ---------: | -----: | ------: | ----: | -------: |
| Latin-1 | 600 | 1 | 39 826 | 3 | 100 % | 100 % |
| CP1250 | 200 | 3 | 13 181 | 8 | 67 % | 100 % |
| CP1250 | 200 | 10 | 13 112 | 28 | 95 % | 100 % |
| CP1250 | 600 | 1 | 39 815 | 2 | 10 % | 100 % |
| KOI8-R | 200 | 3 | 13 195 | 56 | **100 %** | **100 %** |
| KOI8-R | 600 | 10 | 39 724 | 190 | **100 %** | **100 %** |

**Les deux lignes en gras étaient à 0 % sur les deux colonnes.** Quarante
kilooctets ne sauvaient rien : cent quatre-vingt-dix octets de cyrillique dans
un fichier d'anglais revenaient en latin accentué, soixante fois sur soixante.
Le diagnostic « an encoding nothing declared » était bien émis — la lecture au
mieux d'ADR 0008 tenait — mais il annonçait `ISO-8859-1` avec le même aplomb
qu'il aurait annoncé le bon encodage.

**L'écriture est désormais juste partout**, et l'exactitude ne reste basse que
là où elle ne peut pas monter : deux octets de tchèque dans quarante kilooctets
ne départagent pas CP1250 de CP1252, et aucun détecteur ne le peut.

### Ce que la mesure impute, et à qui

**Ce n'est pas une limite de l'information disponible**, et c'est la découverte
de la mesure. Sur un fichier de deux cents répliques dont dix portent du
cyrillique, les trois détecteurs ne disent pas la même chose :

| Détecteur | Réponse |
| :-------- | :------ |
| la nôtre, ICU | `ISO-8859-1` |
| `uchardet`, témoin | `KOI8-R` |
| `file -b --mime-encoding`, témoin | `iso-8859-1` |

Le classement d'ICU sur ce fichier place `ISO-8859-1` à une confiance de 66,
étiquetée « en », et `KOI8-R` à 2, dernière des six candidates. La masse d'ASCII
domine la statistique de lettres, et la règle « le premier candidat qui décode
réellement » ne peut rien y faire : un mono-octet décode tout, donc le premier
du classement gagne toujours.

**Le correctif tient en une ligne, et c'est l'ADR 0028.** Ne soumettre à ICU que
les lignes portant un octet haut — les lignes d'ASCII pur sont identiques dans
toutes les candidates, elles ne discriminent rien — retourne le classement du
même fichier : `KOI8-R` à 54, `ISO-8859-1` disparue. Les deux bords qu'il fallait
éprouver y sont écrits, avec ce qu'ils ont coûté.

### Ce qui n'en sort pas : un seuil

L'issue #310 envisageait que la détection rende `nullopt` en deçà d'une certaine
longueur, et **la mesure l'écarte**, pour deux raisons.

**Un seuil de longueur manquerait le cas.** Le défaut réel est sur des fichiers
de quarante kilooctets ; aucune borne inférieure sur la taille ne les voit.

**Et il rendrait illisible ce qui se lit.** La détection n'est consultée que
lorsque les octets ne sont **pas** de l'UTF-8 valide — c'est la première règle
de `detectEncoding`. Rendre `nullopt` fait retomber `readSubtitles` sur l'UTF-8,
qui échoue par construction : le fichier ne s'ouvre plus du tout. Une réponse
imparfaite qu'un diagnostic annonce vaut mieux qu'un refus.

## Corpus privé — 71 fichiers, sans étiquettes

`src/data/`, des fichiers réels. **Il ne donne aucun taux** : il n'y a pas de
vérité à lui confronter, et s'en inventer une serait pire que de n'en pas avoir.
Ce qu'il donne est ce qui se vérifie sans étiquette. Aucune porte ne le lit.

| | notre détection | `uchardet`, témoin |
| :--- | :-------------- | :----------------- |
| fichiers qui se décodent entièrement sous la réponse | **71/71** | 71/71 |
| réponses UTF-8 | 57 | 52 |
| réponses de la famille latine | 14, dont 11 `cp1252` | 14, dont 5 dites `ascii` |

**Les deux se contredisent sur 14 fichiers sur 71**, et c'est le chiffre le plus
instructif de la table. Aucun de ces désaccords ne porte sur autre chose que la
famille latine : lire de l'ASCII comme du Latin-1 ne change aucun caractère du
fichier tant qu'il n'est pas réécrit.

Que 71 réponses sur 71 décodent le fichier de bout en bout **ne prouve à peu
près rien** : un encodage mono-octet décode presque toute suite d'octets, et
c'est précisément d'où vient le mojibake. Le chiffre est nécessaire, il n'est
pas suffisant. La section précédente en est la démonstration : les fichiers
bilingues y décodaient tous, et tous perdaient leur second alphabet.

## Les trois règles que la détection tranche elle-même

Elles sont dans `core/text/encoding.cpp`, et ce sont elles qui séparent notre
réponse de celle d'ICU.

**Le BOM d'abord, toujours.** C'est la seule chose qu'un fichier de sous-titres
déclare de son encodage. Une heuristique qui passerait avant répondrait à une
question déjà tranchée.

**Seules les lignes qui discriminent sont pesées.** Une ligne d'ASCII pur se lit
de la même façon sous toutes les candidates ; la soumettre à ICU ne fait que
peser dans une statistique de lettres où elle n'a rien à dire. C'est l'ADR 0028,
et la section « à texte rare » ci-dessus est la mesure qui l'a décidée.

**Des octets qui se décodent en UTF-8 sont de l'UTF-8, et ICU n'est pas
consultée.** Mesuré, et l'écart est la raison de la règle : sur
`valides/minimal.srt` — deux accents en cent quarante octets — le détecteur
d'ICU classe ISO-8859-1 à 81 et UTF-8 à 80. Un point de statistiques de lettres
contre une propriété structurelle, et le fichier revenait avec `Ã©` là où il
avait ses accents.
