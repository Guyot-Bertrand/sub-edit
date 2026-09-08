# Détection de format

Ce que le projet répond quand on lui demande de quel format est un fichier, et
**où il cesse de savoir**.

**Un taux, et non « la détection marche ».** C'est l'énoncé de #290 pour
l'encodage, transposé par #340 à l'autre inconnue de la phase 9 : une détection
est un classifieur, elle a un taux de succès, pas un résultat juste ou faux.

**Avec deux formats, la question était fermée d'avance.** Un fichier porte la
signature `WEBVTT` ou une flèche à virgule, et rien d'autre ne s'en approche.
Avec neuf, c'est une classification, et les collisions sont réelles :

| Ce qui se ressemble | Ce que ça vaut |
| :------------------ | :------------- |
| `.sub` | MicroDVD **et** SubViewer 2 |
| `.txt` | TMPlayer, MPL2, et tout le reste du monde |
| `{0}{25}` et `[0][25]` | la même grammaire, à la forme des crochets près |
| SSA et ASS | le même en-tête, à une version et un nom de section près |

## Comment le rejouer

```console
$ make score-format
```

C'est une étape de `check-local` : la mesure se rejoue à chaque pull request, et
une divergence d'avec le relevé se voit. Un taux qui baisse échoue, un taux qui
monte invite à `make score-format-record`, un taux inchangé se tait. Ce n'est
pas un seuil : le nombre comparé est celui de la dernière mesure.

**Le détecteur est un argument**, comme pour l'encodage — le cadrage (#337) aura
des voies à départager, et le choix entre elles est un choix de taux. Le mesurer
après avoir choisi, c'est choisir sans mesurer.

```console
$ cmake --build build/dev --target subedit_detect_format
$ ./src/scripts/score-format-detection.py \
      --detector './build/dev/bin/subedit_detect_format {}' --prive
```

`--prive` ajoute le corpus privé, absent de toute machine qui ne l'a pas. Aucune
porte ne le lit.

## Trois issues, et deux d'entre elles ne se valent pas

Une réponse **juste**. Une réponse **fausse**, qui fait ouvrir un fichier comme
ce qu'il n'est pas. Un **refus**, qui dit ne pas savoir.

**Le refus est la règle du projet appliquée** — rien plutôt qu'une supposition.
**La confusion est ce qu'on ne veut à aucun prix**, et elle échoue à elle seule,
quel que soit le taux : un taux qui monte pendant qu'une confusion apparaît ne
doit pas la racheter.

## Le nom du fichier ne fait pas partie de la question

Les fixtures s'appellent `scene.microdvd.sub` : leur nom **porte la réponse**,
puisqu'il faut bien étiqueter un corpus. Il est retiré avant que le détecteur
les voie — chaque fichier lui est passé sous un nom neutre, **en gardant son
extension réelle**, si bien que `.sub` reste aussi ambigu qu'il l'est dans la
nature.

Le jour où le cadrage décidera que l'extension est une preuve recevable, c'est
là qu'il faudra le dire, et délibérément.

## Corpus étiqueté

`src/test/data/formats/`, la même scène dans les neuf formats — l'étiquette vient
avec le fichier, par construction — et `src/test/data/valides/`, dix fichiers que
leur extension suffit à nommer.

<!-- relevé engendré : ne pas modifier à la main -->

    formats reconnus : 13/19

Relevé sur la version 0.9.16, le 2026-09-07.

| | Fichiers |
| :--- | -------: |
| reconnus | 13 |
| refusés, faute d un lecteur | 6 |
| **confondus** | **0** |

Refusés :

- `formats/scene.ass` — Advanced SSA
- `formats/scene.lrc` — LRC
- `formats/scene.microdvd.sub` — MicroDVD
- `formats/scene.mpl2.txt` — MPL2
- `formats/scene.ssa` — Sub Station Alpha
- `formats/scene.tmplayer.txt` — TMPlayer

**Aucune confusion** : la détection refuse, elle ne se trompe pas.

<!-- fin du relevé -->

**Des refus, et pas une confusion.** C'est exactement l'état de la phase : la
détection ne connaît pas encore les neuf formats, elle le dit, et elle n'invente
rien. Le chiffre monte d'un rendu à chaque format livré, et il est le seul du
dépôt qui mesure l'avancement de la phase 9 plutôt qu'une propriété du code.

## Corpus privé — sans étiquettes

`src/data/`, des fichiers réels. **Il ne donne aucun taux** : il n'y a pas de
vérité à lui confronter, et s'en inventer une serait pire que de n'en pas avoir.
Ce qu'il donne est la distribution des réponses.

| Réponse | Fichiers |
| :------ | -------: |
| WebVTT | 54 |
| SubRip | 17 |
| refusé | 2 |

**Les deux refus sont les deux formats de la phase**, un MicroDVD et un Advanced
SSA, tous deux réels. C'est la confirmation de ce que l'initialisation de la
phase (#336) avait fini par trouver après s'être trompée : le corpus privé porte
autre chose que du `.srt` et du `.vtt`, et le relevé qui disait le contraire
filtrait son parcours sur ces deux extensions.

## Ce que la première version de la sonde a appris

**Elle passait les octets bruts à `detectFormat`**, et le corpus privé a répondu
neuf refus au lieu de deux : sept fichiers WebVTT parfaitement valides étaient
comptés comme des échecs de détection.

La cause n'était pas la détection mais l'ordre. **`detectFormat` travaille sur du
texte décodé**, après que l'encodage a été tranché et la marque retirée ; un
fichier en UTF-16 ne commence pas par les octets de `WEBVTT`. La sonde ouvre
donc le fichier comme le produit l'ouvre, plutôt que d'appeler le classifieur à
côté du chemin qui le nourrit.

C'est le défaut de #268 sous une autre forme — **vérifier avec l'outil qui ne
compte pas la même chose** — et c'est le corpus privé qui l'a attrapé, sur des
fichiers qu'aucune fixture ne remplaçait.
