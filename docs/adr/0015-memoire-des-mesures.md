# 0015 — Compter les lignes non couvertes, et garder mémoire des performances

**Date :** 2026-08-11
**Statut :** acceptée

## Contexte

Deux mesures, un même défaut : elles ne survivent pas à leur exécution.

**La couverture.** `COVERAGE_MIN` valait 80 quand le taux réel était de 99,4 % :
une régression de dix-neuf points franchissait la porte sans un mot. L'issue #49
l'a relevé à 99,2, mais ce nombre n'a pas été choisi — il a été **subi**. gcovr
arrondit le taux à une décimale *avant* de le comparer au seuil, si bien qu'un
seuil entier de 99 ne peut jamais être franchi par un taux à 98,95 % ou plus. Il
a fallu se glisser entre le 99,4 mesuré et le 99,0 qu'affiche le défaut injecté
par `verify-gates.sh`. Un seuil calé à quatre dixièmes près est un seuil dont
personne ne peut plus raisonner : on ne sait plus s'il protège ou s'il est sur
le point de crier au loup.

**Les performances.** « Mesurer, pas supposer » est un principe du projet, et
aucune mesure ne survit à son exécution. Le benchmark de conversion de fréquence
a mesuré la mauvaise opération pendant une phase entière ; rien n'aurait pu le
montrer, faute d'historique.

Ce qu'on sait au moment de décider :

- `gcovr --json-summary` expose les **compteurs bruts**, `line_covered` et
  `line_total`, et pas seulement le pourcentage arrondi. Un script qui compare
  lui-même échappe donc entièrement au piège d'arrondi ;
- le **rapporteur XML de Catch2** sort les benchmarks en entier — moyenne,
  bornes de confiance, écart-type, points aberrants. Son rapporteur JSON, lui,
  a un `benchmarkEnded` au corps vide : il ne produit rien.

Ce qu'on ignore : la **variance naturelle** des mesures. Le même tri a varié de
230 à 870 µs entre deux exécutions sur la machine de développement.

## Décision

### La couverture se compte en lignes non couvertes

Le cliquet porte sur le **nombre de lignes de `src/lib/` que les tests
n'exercent pas** — 8 aujourd'hui, sur 1353 — et non sur un pourcentage. Il vit
dans `docs/mesures/couverture.md`, avec son mode d'emploi.

`--fail-under-line` disparaît : le script lit le `--json-summary`, calcule
`line_total - line_covered`, et confronte au fichier.

| Situation | Réaction |
| :-------- | :------- |
| plus de lignes non couvertes qu'au relevé | échec, en nommant le fichier qui en a gagné |
| autant | passe, en silence |
| moins | passe, et invite : `make ratchet` enregistre la nouvelle valeur |

**La porte échoue à la baisse, elle n'écrit jamais d'elle-même.** Un fichier
versionné ne bouge que si quelqu'un le demande ; mais personne n'a à calculer
le chiffre.

### Les performances tiennent un journal

`docs/mesures/performances.md`, alimenté par `make bench` depuis la sortie XML
de Catch2.

- **Une section par version**, remplacée si l'on rejoue au même numéro. Le patch
  bougeant une fois par pull request, l'historique devient une mesure par
  version livrée — la granularité qui rend deux chiffres comparables.
- **Moyenne et écart-type** par mesure. L'écart-type n'est pas décoratif : c'est
  lui qui portera le seuil d'alerte, quand la variance sera connue.
- **Le mode de compilation** est enregistré, bien qu'il vaille toujours
  `Release` aujourd'hui.
- **Élagage à un mois**, doublé d'une **table d'extrêmes permanente** : par
  mesure, le minimum et le maximum jamais atteints, avec leur version et leur
  date.
- **Aucun seuil d'alerte**, et le fichier dit pourquoi.

## Alternatives écartées

- **Un seuil en pourcentage**, fût-il exact plutôt qu'arrondi. Écarté pour une
  raison qui n'a rien à voir avec l'arrondi : le pourcentage porte un **effet
  pervers**. Ajouter du code bien testé le fait monter, donc le cliquet monte,
  donc le travail suivant est contraint plus dur sans que personne l'ait décidé.
  Le compte de lignes non couvertes ne bouge pas quand on ajoute du code
  couvert, et ne réagit qu'à ce qu'on veut réellement surveiller.

- **Le couple `couvertes / total`.** Plus riche en information, mais il faut
  définir ce qui constitue une régression quand les deux nombres bougent
  ensemble — une règle de plus à écrire, à justifier et à défendre.

- **Relever le cliquet automatiquement** dès que la couverture monte. Écarté :
  un fichier versionné changerait sans qu'on l'ait demandé, et il faudrait
  ensuite le commiter — le geste revient donc, déguisé en surprise plutôt qu'en
  instruction.

- **Tout laisser à la main**, la porte se taisant à la hausse. Écarté : le
  cliquet décrocherait du réel, et un seuil qui traîne loin sous la vérité est
  exactement le défaut que ce ticket corrige.

- **Garder `--fail-under-line` en filet grossier**, sous le cliquet fin. Écarté :
  deux seuils que personne ne sait réconcilier, dont un qu'on sait émoussé par
  l'arrondi.

- **Enregistrer chaque exécution de `make bench`.** Séduisant parce que la
  variance apparaîtrait gratuitement. Écarté : trois essais dans la journée
  feraient trois relevés versionnés, dont deux que personne n'a choisi de
  garder. La table d'extrêmes donne l'enveloppe sans le bruit.

- **Écrire hors du dépôt et verser sur demande.** Cohérent avec le choix fait
  pour le cliquet, mais le ticket demande expressément que `make bench` conserve
  ses chiffres, et un geste de plus est un geste qu'on oublie.

- **Épingler les sections anciennes qui détiennent un record**, plutôt qu'une
  table d'extrêmes séparée. Écarté par arithmétique : seize mesures ayant
  chacune deux extrêmes, jusqu'à trente-deux sections seraient épinglées, et
  l'élagage cesserait de vouloir dire quelque chose.

- **Analyser la sortie console des benchmarks.** Écarté pour la raison qui a
  déjà écarté le `grep` sur `TEST_CASE` en [0014](0014-registre-d-exigences.md) :
  Catch2 replie ses lignes à 70 colonnes et concatène les orthographes d'un même
  tag. On demande à l'outil ce qu'il sait, on ne devine pas sa mise en page.

- **Le rapporteur JSON de Catch2**, qui serait le support le plus commode. Il
  n'émet rien pour les benchmarks : `benchmarkEnded` a un corps vide. Ce n'est
  pas un choix, c'est un fait vérifié dans ses sources.

## Conséquences

**La preuve de la porte de couverture devient robuste.** Aujourd'hui elle ne se
referme que parce que 99,2 a été calé à quatre dixièmes du taux réel ; avec un
compte de lignes, la fonction non couverte injectée par `verify-gates.sh` fait
échouer la porte quelle que soit la taille du projet.

**`make check` change**, et il faut le dire : son étape de couverture n'utilise
plus `--fail-under-line`. Ce n'est pas une étape de plus — l'étape existe depuis
la phase 0, seule la façon dont elle décide change. La CI n'exécute rien de
nouveau.

**`make check-local` enchaîne déjà `bench`**, donc le lancer avant d'ouvrir une
pull request vérifie et enregistre d'un même geste. Le fichier modifié se
commite avec le reste, ce qui correspond à la règle du projet : les benchmarks
se rejouent à chaque issue.

**Une condition rend « une mesure par version livrée » vraie plutôt qu'assumée :**
ce `make check-local` doit suivre le bump du patch, pas le précéder.
`CMakeLists.txt` est ce que `record-bench.sh` lit pour nommer la section
qu'il écrit ; rejoué avant le bump, il nomme sa section d'après la version
*précédente*, et la section suivante — la vraie mesure de cette version-là —
n'est jamais écrite : la version courante reste sans relevé pendant qu'une
version déjà livrée en gagne un second, faux.

**La table d'extrêmes est ce que lira le futur seuil d'alerte.** C'est la
seconde étape que le ticket annonce sans la traiter, et elle a besoin de
connaître la variance avant de pouvoir poser un chiffre.

**Le coût, écrit plutôt que tu :** un compte de lignes ne dit pas *où*. Le
script doit nommer le fichier qui a gagné des lignes non couvertes, sinon le
message d'échec envoie chercher une aiguille dans mille trois cents lignes.

Le déclencheur d'un réexamen est identifiable : le jour où une ligne
légitimement inatteignable — une branche défensive, un `default:` impossible —
devra être tolérée durablement, le cliquet obligera à le relever, ce qui
autorise silencieusement une ligne non couverte de plus ailleurs. Si le cas se
répète, un cliquet par fichier ou des marqueurs d'exclusion deviendront le
sujet.

Défaire cette décision coûte peu : rétablir `--fail-under-line` et supprimer
les deux fichiers.
