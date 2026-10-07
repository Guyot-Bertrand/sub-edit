# Registre des exigences

Ce que `subedit` promet, ligne par ligne, avec l'état de ce qui le prouve.

Une exigence est **ce que le binaire montre** : sortie standard, sortie
d'erreur, code de retour, fichier produit. Les garanties internes du noyau n'en
sont pas — leurs tests unitaires et la couverture de lignes tiennent ce rôle.
La décision, et les sept options écartées, sont dans
[l'ADR 0014](adr/0014-registre-d-exigences.md).

Ce registre ne remplace pas le [manuel](manual/), qui reste la description
exhaustive de ce que l'utilisateur voit. Il dit autre chose : **quelle promesse
est démontrée par un test**, ce qu'aucun taux de couverture ne sait dire.

## Identifiants

`SURFACE-SUJET-NN`, en capitales, numéro sur deux chiffres — `CLI-VERSION-01`.
`CLI` pour la ligne de commande, `GUI` pour la fenêtre.

Cette forme est ce qui permet de reconnaître un identifiant parmi les tags d'un
test sans tenir de liste à jour : un tag ordinaire du projet — `[e2e]`,
`[format]`, `[framerate]` — n'a ni capitales ni segment numérique final.

**Un identifiant n'est jamais réutilisé.** Une exigence qui disparaît laisse sa
ligne, barrée, la raison dans la colonne d'état. Un identifiant retiré sans
trace laisse un trou que personne ne sait interpréter six mois plus tard.

## États

| État | Ce que la ligne affirme | Ce que le contrôle exige |
| :--- | :---------------------- | :----------------------- |
| `prévue` | l'exigence est décidée, le code ne l'honore pas encore | aucun test ne la cite |
| `implémentée` | le code l'honore | au moins un test la cite |
| `abandonnée` | elle ne sera pas honorée ; la raison suit dans la colonne | aucun test ne la cite |
| `remplacée` | une autre exigence prend sa place ; son identifiant suit | aucun test ne la cite |

Un test cite une exigence par un **tag Catch2** :

```cpp
TEST_CASE("invoking with no argument writes the version", "[e2e][CLI-VERSION-01]")
```

`src/scripts/check-requirements.sh` confronte ces affirmations aux tags que le
binaire de test déclare, et `make check-local` l'exécute. Une exigence
`implémentée` sans test ne franchit pas la porte ; un tag qui ne désigne
aucune exigence non plus.

**Le registre s'alimente en début d'issue**, avant le code. Une exigence écrite
après coup décrit ce qui a été fait ; écrite avant, elle décide ce qui sera
fait.

## Registre

**Rien ne suit cette table.** `src/scripts/verify-gates.sh` prouve que la porte
se referme en ajoutant une ligne en fin de fichier ; du texte après la table
lui ferait injecter son défaut au mauvais endroit, et la preuve ne prouverait
plus rien.

| ID | Exigence | Phase | État |
| :- | :------- | :---- | :--- |
| ~~`CLI-VERSION-01`~~ | l'invocation sans argument écrit `subedit <version>` sur la sortie standard | 3 | remplacée par `CLI-USAGE-01` |
| ~~`CLI-VERSION-02`~~ | rien n'est écrit sur la sortie d'erreur | 3 | remplacée par `CLI-VERSION-05` |
| ~~`CLI-VERSION-03`~~ | tout argument est ignoré, code de retour 0 | 3 | remplacée par `CLI-USAGE-02` |
| `CLI-VERSION-04` | `--version` écrit `subedit <version>` sur la sortie standard, code 0 | 3 | implémentée |
| `CLI-VERSION-05` | une invocation qui réussit n'écrit rien sur la sortie d'erreur | 3 | implémentée |
| `CLI-USAGE-01` | sans argument, l'aide est écrite sur la sortie standard, code 0 | 3 | implémentée |
| `CLI-USAGE-02` | une option inconnue ou une valeur invalide donne le code 1 | 3 | implémentée |
| `CLI-USAGE-03` | une erreur d'usage est détectée avant tout traitement | 3 | implémentée |
| `CLI-USAGE-04` | `--quiet` et `-v` ensemble donnent une erreur d'usage | 3 | implémentée |
| `CLI-OUTPUT-01` | seul le résultat va sur la sortie standard, la narration sur la sortie d'erreur | 3 | implémentée |
| `CLI-OUTPUT-02` | `--quiet` supprime toute narration et laisse passer les erreurs | 3 | implémentée |
| `CLI-OUTPUT-03` | chaque niveau de verbosité contient le précédent, ligne pour ligne | 3 | implémentée |
| `CLI-OUTPUT-04` | le niveau par défaut écrit une ligne par fichier traité | 3 | implémentée |
| `CLI-OUTPUT-05` | le bilan n'apparaît qu'à partir de deux fichiers d'entrée | 3 | implémentée |
| `CLI-OUTPUT-06` | le niveau 3 nomme chaque diagnostic de lecture, sur toutes les sous-commandes | 3 | implémentée |
| `CLI-BATCH-01` | plusieurs fichiers sont traités indépendamment, les échecs nommés | 3 | implémentée |
| `CLI-BATCH-02` | tous en échec donne le code 2, un échec partiel le code 3 | 3 | implémentée |
| `CLI-INSPECT-01` | format, encodage, BOM, fins de ligne, nombre et étendue sont rapportés | 3 | implémentée |
| `CLI-INSPECT-02` | des fins de ligne mélangées sont signalées avec leur ligne | 3 | implémentée |
| ~~`CLI-INSPECT-03`~~ | `--order-report` rend les deux lectures du désordre | 3 | abandonnée : le corpus n'a pas départagé les deux lectures, l'option disparaît et le rapport nomme ce qui rompt l'ordre |
| `CLI-CONVERT-01` | `--to` produit le format demandé | 3 | implémentée |
| `CLI-CONVERT-02` | fins de ligne et BOM sont conservés par défaut, réglables sinon | 3 | implémentée |
| `CLI-CONVERT-03` | sans destination explicite, rien n'est écrit et le code est 1 | 3 | implémentée |
| `CLI-SHIFT-01` | `--by` décale toutes les positions, dans les deux sens | 3 | implémentée |
| `CLI-SHIFT-02` | un décalage rendant une position négative est refusé en nommant le sous-titre | 3 | implémentée |
| `CLI-TRANSFORM-01` | deux repères transforment les positions, les autres suivent | 3 | implémentée |
| `CLI-TRANSFORM-02` | deux indices confondus sont refusés | 3 | implémentée |
| `CLI-FRAMERATE-01` | `--from`/`--to` convertit les positions | 3 | implémentée |
| `CLI-FRAMERATE-02` | une fréquence nulle ou négative est refusée | 3 | implémentée |
| `CLI-HEARING-01` | `hearing-impaired` retire les mentions entre crochets et entre parenthèses | 4 | implémentée |
| `CLI-HEARING-02` | un sous-titre que le retrait vide est supprimé du fichier écrit | 4 | implémentée |
| `CLI-HEARING-03` | une référence purement numérique est laissée telle quelle | 4 | implémentée |
| `CLI-HEARING-04` | le rapport nomme le nombre de sous-titres changés et supprimés | 4 | implémentée |
| `CLI-HEARING-05` | un fichier sans aucune mention est écrit inchangé, code 0 | 4 | implémentée |
| `CLI-HEARING-06` | sans destination explicite, rien n'est écrit et le code est 1 | 4 | implémentée |
| `GUI-VERSION-01` | `subedit-gui --version` écrit `subedit <version>` sur la sortie standard, code 0 | 5 | implémentée |
| `GUI-OPEN-01` | `subedit-gui <fichier>` ouvre le fichier et affiche ses sous-titres | 5 | implémentée |
| `GUI-OPEN-02` | un fichier illisible donne un message et laisse la fenêtre vide | 5 | implémentée |
| `GUI-OPEN-03` | les diagnostics de lecture sont montrés, avec leur ligne | 5 | implémentée |
| `GUI-TABLE-01` | les cinq colonnes affichent numéro, début, fin, durée et texte | 5 | implémentée |
| `GUI-TABLE-02` | les sous-titres en anomalie sont marqués et nommés | 5 | implémentée |
| `GUI-TABLE-03` | les colonnes, sauf le texte, se masquent ; toutes se déplacent ; la table s'en souvient | 11 | implémentée |
| `GUI-EDIT-01` | éditer une cellule de texte modifie le sous-titre et rien d'autre | 5 | implémentée |
| `GUI-EDIT-02` | éditer un début ou une fin lit un horodatage permissif | 5 | implémentée |
| `GUI-EDIT-03` | une validation qui ne change rien n'entre pas dans l'historique | 5 | implémentée |
| `GUI-EDIT-04` | chaque ligne d'une cellule de texte et de son éditeur montre sa longueur, balises non comptées, en ems ou en caractères selon le réglage, et les réglages la retirent | 12 | implémentée |
| `GUI-UNDO-01` | annuler rétablit l'état précédent, l'action nomme l'opération | 5 | implémentée |
| `GUI-UNDO-02` | rétablir refait ce qui vient d'être annulé | 5 | implémentée |
| `GUI-SAVE-01` | enregistrer réécrit le fichier dans sa forme d'origine | 5 | implémentée |
| `GUI-SAVE-02` | enregistrer sous choisit chemin et format | 5 | implémentée |
| `GUI-SAVE-03` | fermer avec des modifications non enregistrées demande confirmation | 5 | implémentée |
| `GUI-SHIFT-01` | le dialogue de décalage décale la cible | 5 | implémentée |
| `GUI-TRANSFORM-01` | le dialogue de transformation corrige par deux repères | 5 | implémentée |
| `GUI-FRAMERATE-01` | le dialogue de conversion re-cale la cible | 5 | implémentée |
| `GUI-HEARING-01` | le retrait des mentions s'applique à la sélection ou au fichier | 5 | implémentée |
| `GUI-HEARING-02` | un retrait qui ne change rien le dit et n'entre pas dans l'historique | 5 | implémentée |
| `CLI-INSPECT-04` | les anomalies d'un document sont rapportées par numéro de sous-titre | 5 | implémentée |
| `CLI-INSPECT-05` | `inspect` écrit la grille déduite, sa concentration et le verdict | 16 | implémentée |
| `CLI-INSPECT-06` | un fichier sans grille le dit, et ne nomme aucune fréquence | 16 | implémentée |
| `CLI-INSPECT-07` | une ambiguïté harmonique est nommée, et la plus basse est retenue | 16 | implémentée |
| `CLI-SNAP-01` | `snap --rate R` porte chaque position sur l'image la plus proche de R | 16 | implémentée |
| `CLI-SNAP-02` | `snap` écrit combien de positions ont bougé, et de combien au plus | 16 | implémentée |
| `CLI-SHIFT-03` | `shift --to-grid` décale du montant mesuré, et l'écrit | 16 | implémentée |
| `CLI-SHIFT-04` | `shift --to-grid` sur un fichier sans grille refuse et dit pourquoi | 16 | implémentée |
| `GUI-GRID-01` | la barre d'état porte le verdict de la déduction | 16 | implémentée |
| `GUI-GRID-02` | la modale d'analyse montre les huit candidates, l'étendue et les écarts | 16 | implémentée |
| `GUI-FRAMERATE-03` | le dialogue de conversion pré-remplit l'entrée avec la mesure, et le dit | 16 | implémentée |
| `GUI-FRAMERATE-04` | un désaccord entre la mesure et ce que la vidéo déclare est montré sans être arbitré | 16 | implémentée |
| `GUI-SNAP-01` | la fenêtre aligne le document sur une cadence, et l'opération s'annule | 16 | implémentée |
| `GUI-GRID-03` | ramener sur la grille affiche son montant, et s'éteint sans grille | 16 | implémentée |
| `GUI-VIDEO-01` | choisir une vidéo l'associe au document, et la fenêtre la nomme | 6 | implémentée |
| `GUI-VIDEO-02` | ouvrir un fichier de sous-titres propose la vidéo voisine de même nom | 6 | implémentée |
| `GUI-PLAYER-01` | la vidéo s'ouvre dans la fenêtre, et se joue | 6 | implémentée |
| `GUI-PLAYER-02` | sélectionner un sous-titre place la lecture à son début | 6 | implémentée |
| `GUI-PLAYER-03` | une vidéo qui ne s'ouvre pas le dit, et laisse la fenêtre utilisable | 6 | implémentée |
| `GUI-FRAMERATE-02` | la fréquence lue dans la vidéo est proposée, et sa provenance est dite | 6 | implémentée |
| `GUI-BOUNDS-01` | une opération qui dépasse la fin du film le signale sans l'empêcher | 6 | implémentée |
| `GUI-CONFIG-01` | la fenêtre retrouve sa géométrie et ses colonnes d'une session à l'autre | 7 | implémentée |
| `GUI-CONFIG-02` | une valeur illisible laisse le défaut en place, et le dit | 7 | implémentée |
| `GUI-CONFIG-03` | une option restée à son défaut est réécrite commentée, donc un défaut changé prend effet | 7 | implémentée |
| `GUI-THEME-01` | le thème se choisit entre trois valeurs, se retient, et « système » ne pose rien | 7 | implémentée |
| `GUI-THEME-02` | « clair » et « sombre » posent une palette explicite | 7 | implémentée |
| `GUI-INSERT-01` | insérer place les sous-titres après le dernier sélectionné, ou avant au choix | 7 | implémentée |
| `GUI-INSERT-02` | insérer dans un document vide ne demande aucune sélection | 7 | implémentée |
| `GUI-REMOVE-01` | supprimer retire la sélection, et l'opération s'annule | 7 | implémentée |
| `GUI-MANUAL-01` | `Help ▸ Manual` ouvre le manuel installé | 7 | implémentée |
| `CLI-ENC-01` | un fichier non-UTF-8 s'ouvre, et l'encodage retenu est dit | 8 | implémentée |
| `CLI-ENC-02` | `--encoding` impose l'encodage de lecture et l'emporte sur la détection | 8 | implémentée |
| `CLI-ENC-03` | un BOM l'emporte sur `--encoding`, et l'écart est dit | 8 | implémentée |
| `CLI-ENC-04` | `--to-encoding` choisit l'encodage produit à l'écriture | 8 | implémentée |
| `CLI-ENC-05` | un fichier réécrit sans consigne rend les mêmes octets, encodage compris | 8 | implémentée |
| `CLI-ENC-06` | un fichier qu'aucun encodage ne décode est refusé, et la raison le dit | 8 | implémentée |
| `CLI-ENC-07` | `inspect` rapporte l'encodage lu et s'il a été deviné | 8 | implémentée |
| `GUI-ENC-01` | la fenêtre ouvre un fichier non-UTF-8 et affiche l'encodage retenu | 8 | implémentée |
| `GUI-ENC-02` | `Save As…` choisit l'encodage, la fin de ligne et le BOM | 8 | implémentée |
| `GUI-ENC-03` | les réglages retiennent le dernier encodage d'écriture choisi | 8 | implémentée |
| `CLI-FORMAT-01` | chacun des neuf formats s'ouvre, et se réécrit octet pour octet | 9 | implémentée |
| `CLI-FORMAT-02` | un fichier qu'aucun des neuf ne revendique est refusé, sans supposition | 9 | implémentée |
| `CLI-FORMAT-03` | `inspect` nomme le format lu parmi les neuf | 9 | implémentée |
| `CLI-FORMAT-04` | un format sans fin s'ouvre, ses fins sont déduites, et la lecture le dit | 9 | implémentée |
| `CLI-FRAMES-01` | un fichier en images s'ouvre à une fréquence déclarée, que la lecture nomme | 9 | implémentée |
| `CLI-FRAMES-02` | `--frame-rate` impose la fréquence, à la lecture comme à l'écriture | 9 | implémentée |
| `CLI-CONVERT-04` | `--to` accepte les neuf formats | 9 | implémentée |
| `CLI-CONVERT-05` | convertir vers un format en images sans fréquence prend la grille déduite, et le dit | 9 | implémentée |
| `CLI-CONVERT-06` | sans fréquence ni grille, une conversion vers un format en images est refusée | 9 | implémentée |
| `CLI-CONVERT-07` | une conversion qui perd quelque chose le dit, poste par poste | 9 | implémentée |
| `GUI-FORMAT-01` | la fenêtre ouvre les neuf formats | 9 | implémentée |
| `GUI-FORMAT-02` | `Save As…` propose les neuf formats | 9 | implémentée |
| `GUI-FORMAT-03` | enregistrer dans un format à perte annonce ce qui sera perdu avant d'écrire | 9 | implémentée |
| `GUI-FRAMES-01` | ouvrir un fichier en images annonce la fréquence retenue, et laisse la changer | 9 | implémentée |
| `GUI-ITALIC-01` | une entrée met la sélection en italique et l'en retire, dans les balises du format ouvert | 10 | implémentée |
| `GUI-ITALIC-02` | un format qui ne porte aucun style éteint l'entrée sans la cacher | 10 | implémentée |
| `GUI-CASE-01` | les quatre casses s'appliquent à la cible sans toucher aux balises | 10 | implémentée |
| `GUI-DASH-01` | les tirets de dialogue se posent et se retirent d'un même geste | 10 | implémentée |
| `GUI-SPLIT-01` | fusionner et scinder, et l'annulation rend le document tel qu'il était | 10 | implémentée |
| `GUI-DURATION-01` | la colonne `Duration` se saisit, et déplace la fin | 10 | implémentée |
| `GUI-ADJUST-01` | l'ajustement applique les quatre contraintes dans l'ordre spécifié | 10 | implémentée |
| `GUI-ADJUST-02` | ce qu'aucune fin ne peut satisfaire est dit, et compté | 10 | implémentée |
| `GUI-SEARCH-01` | chercher dans le texte visible, remplacer dans le source, sans casser les balises | 10 | implémentée |
| `GUI-SEARCH-02` | la recherche porte sur la sélection, ou sur tout le document | 10 | implémentée |
| `GUI-CLIP-01` | copier, couper, coller des textes, et des lignes naissent s'il en manque | 10 | implémentée |
| `GUI-CLIP-02` | coller depuis un document d'un autre format traduit les balises et dit la perte | 10 | implémentée |
| `GUI-TRANS-01` | ouvrir un fichier de traduction l'aligne sur le principal, par position ou par numéro | 11 | implémentée |
| `GUI-TRANS-02` | l'ouverture dit ce qui s'est passé : lignes rattachées, sous-titres nés, sous-titres sans traduction, lignes hors d'ordre | 11 | implémentée |
| `GUI-TRANS-03` | ouvrir une traduction est une seule entrée d'historique, et l'annulation rend le document tel qu'il était | 11 | implémentée |
| `GUI-TRANS-04` | la colonne de traduction apparaît avec la traduction, et se saisit | 11 | implémentée |
| `GUI-TRANS-05` | une opération de texte vise le document de la colonne courante, et la barre d'état le dit | 11 | implémentée |
| `GUI-TRANS-06` | la traduction s'enregistre à part du principal, chacun avec son état modifié | 11 | implémentée |
| `GUI-CLOSE-01` | fermer avec plusieurs documents modifiés pose une seule question, une case par document | 11 | implémentée |
| `GUI-SEARCH-03` | la recherche porte sur le document visé, et remplacer écrit dans son texte source | 11 | implémentée |
| `GUI-PLAYER-04` | la réplique dessinée sur l'image est celle du document visé | 11 | implémentée |
| `GUI-APPEND-01` | ajouter un fichier décale de la fin du dernier sous-titre, dit ce que la conversion perd, et s'annule d'un coup | 11 | implémentée |
| `GUI-TABS-01` | plusieurs projets s'ouvrent en onglets, chacun avec son historique, sa sélection et sa vidéo | 11 | implémentée |
| `GUI-TABS-02` | fermer tout, ou quitter, avec une seule question pour tous les projets | 11 | implémentée |
| `GUI-TABS-03` | l'étiquette d'un onglet dit que son projet est modifié | 11 | implémentée |
| `GUI-TABS-04` | glisser des fichiers sur la fenêtre les ouvre : un onglet par sous-titre, une vidéo à l'onglet courant | 11 | implémentée |
| `GUI-SAVE-04` | enregistrer tout écrit chaque document modifié, onglet par onglet, et s'arrête à un abandon en disant ce qui a été écrit | 11 | implémentée |
| `GUI-PSPLIT-01` | scinder un projet en deux, et l'ajout de la suite rend le projet de départ | 11 | implémentée |
| `GUI-SEARCH-04` | la recherche porte, au choix, sur tous les projets ouverts | 11 | implémentée |
| `GUI-CORRECT-01` | l'assistant applique les tâches cochées à la cible choisie — sélection, projet, tous les projets — et au document choisi | 12 | implémentée |
| `GUI-CORRECT-02` | chaque texte changé se montre avec son original avant d'être appliqué, et s'accepte, se refuse ou se retouche | 12 | implémentée |
| `GUI-CORRECT-03` | appliquer fait une entrée d'historique par projet, et la barre d'état dit combien de sous-titres ont été modifiés et supprimés | 12 | implémentée |
| `GUI-CORRECT-04` | les motifs se cochent par nom, pour une écriture, une langue et un pays, et le choix est retenu | 12 | implémentée |
| `GUI-CORRECT-05` | décocher Humain ou OCR retire ces motifs de l'application | 12 | implémentée |
| `GUI-CORRECT-06` | un motif qui ne se lit pas, ne se traduit pas ou ne termine pas est nommé, et les autres s'appliquent | 12 | implémentée |
| `GUI-CORRECT-07` | les motifs déposés par l'utilisateur s'ajoutent aux motifs livrés | 12 | implémentée |
| `GUI-HEARING-03` | l'assistant retire les paroles entre dièses et le nom du locuteur, en plus des crochets et des parenthèses | 12 | implémentée |
| `GUI-BREAK-01` | le découpage tient la longueur et le nombre de lignes demandés, en caractères ou en ems, et saute les sous-titres qui les tiennent déjà | 12 | implémentée |
| `GUI-SPELL-01` | la vérification parcourt les mots inconnus : ignorer, tout ignorer, ajouter au dictionnaire, remplacer, tout remplacer, joindre au précédent ou au suivant | 12 | implémentée |
| `GUI-SPELL-02` | sans dictionnaire pour la langue choisie, les fonctions du correcteur sont grisées et disent pourquoi | 12 | implémentée |
| `GUI-SPELL-03` | l'assistant joint et scinde des mots selon le correcteur | 12 | implémentée |
| `GUI-SPELL-04` | un mot inconnu est souligné pendant qu'on le tape dans l'éditeur d'une cellule de texte, quand le réglage est vrai et qu'un dictionnaire existe ; sans dictionnaire, rien n'est souligné et rien n'échoue | 12 | implémentée |
| `CLI-JSON-01` | `--format json` écrit sur la sortie standard un objet JSON par ligne ; `text` est le défaut et ne change pas | 13 | implémentée |
| `CLI-JSON-02` | chaque entrée donne **exactement un** objet, échec compris, dans l'ordre des entrées | 13 | implémentée |
| `CLI-JSON-03` | chaque objet porte `schema`, `command`, `file` et `ok` ; un échec porte `error.kind` et `error.message` | 13 | implémentée |
| `CLI-JSON-04` | la verbosité n'agit pas sur la sortie standard en `json` ; la narration reste du texte sur la sortie d'erreur | 13 | implémentée |
| `CLI-JSON-05` | les diagnostics de lecture sont dans `warnings`, à tous les niveaux | 13 | implémentée |
| `CLI-JSON-06` | aucun nombre à virgule : positions en millisecondes entières, cadences en chaînes | 13 | implémentée |
| `CLI-JSON-07` | une erreur d'usage n'écrit rien sur la sortie standard, en `json` comme en `text` | 13 | implémentée |
| `CLI-JSON-08` | `inspect` décrit le fichier : format, encodage, marque, fins de ligne, nombre, étendue, grille ou cadence, anomalies | 13 | implémentée |
| `CLI-JSON-09` | chaque sous-commande qui écrit dit sa destination et ses comptes | 13 | implémentée |
| `CLI-JSON-10` | mêmes entrées et mêmes arguments donnent les octets des attendus versionnés | 13 | implémentée |
| `CLI-DRYRUN-01` | `--dry-run` n'écrit aucun fichier et ne crée aucun dossier, et le code est celui d'un vrai lancement | 13 | implémentée |
| `CLI-DRYRUN-02` | `--dry-run` n'exige aucune destination | 13 | implémentée |
| `CLI-DRYRUN-03` | une destination donnée avec `--dry-run` est vérifiée comme sans lui | 13 | implémentée |
| `CLI-DRYRUN-04` | sur une sous-commande de texte, la sortie standard porte, par sous-titre changé, son numéro, le texte d'avant et le texte d'après | 13 | implémentée |
| `CLI-DRYRUN-05` | en `json`, `changes` porte les mêmes changements, `after` valant `null` pour une suppression | 13 | implémentée |
| `CLI-DRYRUN-06` | la narration d'un `--dry-run` dit que rien n'a été écrit | 13 | implémentée |
| `CLI-BATCH-03` | deux entrées de même destination sont refusées avant tout écrit, code `1`, les deux nommées | 13 | implémentée |
| `CLI-BATCH-04` | une destination qui est une entrée est refusée sans `--in-place` | 13 | implémentée |
| `CLI-BATCH-05` | le dossier de sortie absent est créé | 13 | implémentée |
| `CLI-BATCH-06` | une écriture qui échoue dit « written », jamais « read » | 13 | implémentée |
| `CLI-BATCH-07` | une destination existante est écrasée par écriture atomique | 13 | implémentée |
| `CLI-BATCH-08` | un répertoire en entrée sans `--recursive` est refusé, code `1` | 13 | implémentée |
| `CLI-BATCH-09` | `--recursive` parcourt dans l'ordre des noms, sans suivre de lien, les extensions des formats connus, `.txt` exclu | 13 | implémentée |
| `CLI-BATCH-10` | l'arborescence relative au répertoire donné est conservée sous `--output-dir` | 13 | implémentée |
| `CLI-BATCH-11` | le dossier de sortie compris dans l'arbre parcouru est exclu du parcours | 13 | implémentée |
| `CLI-BATCH-12` | un fichier nommé sur la ligne de commande n'est jamais filtré par son extension | 13 | implémentée |
| `CLI-RANGE-01` | `--range N-M` limite l'opération aux sous-titres N à M, bornes comprises ; `N-` va jusqu'à la fin | 13 | implémentée |
| `CLI-RANGE-02` | une plage hors bornes est refusée en nommant la borne, avant tout traitement | 13 | implémentée |
| `CLI-ADJUST-01` | `adjust` sans option applique les quatre contraintes de Gaupol, dans leur ordre | 13 | implémentée |
| `CLI-ADJUST-02` | chaque contrainte se règle et s'éteint (`off`) ; `--maximum` l'allume | 13 | implémentée |
| `CLI-ADJUST-03` | sans aucune contrainte active, `adjust` est refusé | 13 | implémentée |
| `CLI-ADJUST-04` | le compte rendu dit ce qui a été ajusté et ce qui a été sacrifié, contrainte par contrainte | 13 | implémentée |
| `CLI-ADJUST-05` | en `json`, `counts` porte `adjusted` et `sacrificed.{speed,minimum,gap}`, et `constraints` les contraintes employées | 13 | implémentée |
| `CLI-ADJUST-06` | les comptes sacrifiés s'accordent avec ceux que le script de mesure prédit sur les fixtures versionnées | 13 | implémentée |
| `CLI-CORRECT-01` | `--tasks` est requis et nomme les tâches ; aucune n'est cochée d'avance | 13 | implémentée |
| `CLI-CORRECT-02` | `--code` est requis dès qu'une tâche lit des motifs | 13 | implémentée |
| `CLI-CORRECT-03` | le compte rendu dit les textes changés et supprimés, jamais les correspondances | 13 | implémentée |
| `CLI-CORRECT-04` | `--classes` retire de l'application les motifs de la classe décochée | 13 | implémentée |
| `CLI-CORRECT-05` | `--enable` / `--disable` règlent un motif par son nom ; un nom inconnu ou ambigu est refusé | 13 | implémentée |
| `CLI-CORRECT-06` | une tâche de motifs sans aucun motif actif est refusée | 13 | implémentée |
| `CLI-CORRECT-07` | les motifs de l'utilisateur s'ajoutent aux motifs livrés | 13 | implémentée |
| `CLI-CORRECT-08` | un motif qui ne se lit pas, ne se traduit pas ou ne termine pas est nommé avec le sous-titre, et les autres s'appliquent | 13 | implémentée |
| `CLI-CORRECT-09` | `line-break` mesure en caractères ; `--max-length` est requis ; les bornes du saut suivent, et s'éteignent | 13 | implémentée |
| `CLI-CORRECT-10` | `join-words` et `split-words` joignent et scindent selon le dictionnaire de `--language` | 13 | implémentée |
| `CLI-CORRECT-11` | sans dictionnaire pour `--language`, `correct` est refusé avec la phrase de la fenêtre, avant tout traitement | 13 | implémentée |
| `CLI-CORRECT-12` | les sous-titres vidés sont retirés, sauf `--keep-blank-subtitles` | 13 | implémentée |
| `CLI-CORRECT-13` | `correct` ne lit aucun réglage de l'utilisateur ni sa liste de remplacements | 13 | implémentée |
| `CLI-REPLACE-01` | `replace` cherche dans le texte visible et remplace dans le texte source, sans casser une balise | 13 | implémentée |
| `CLI-REPLACE-02` | `--regex` lit une expression ; une expression illisible est refusée avant tout traitement, avec la raison | 13 | implémentée |
| `CLI-REPLACE-03` | les majuscules sont ignorées par défaut, `--case-sensitive` les distingue | 13 | implémentée |
| `CLI-REPLACE-04` | le compte rendu dit les textes changés, ou que rien n'a été trouvé | 13 | implémentée |
| `CLI-CASE-01` | `case` applique les quatre casses sans toucher aux balises | 13 | implémentée |
| `CLI-ITALIC-01` | `italics --on` / `--off` pose et retire l'italique dans les balises du format du fichier | 13 | implémentée |
| `CLI-ITALIC-02` | un format qui ne porte aucun style refuse, fichier par fichier, avec la raison | 13 | implémentée |
| `CLI-DASH-01` | `dialogue-dashes --add` / `--remove` pose et retire les tirets de dialogue | 13 | implémentée |
| `CLI-SORT-01` | `sort` met les sous-titres dans l'ordre de leur début, de façon stable | 13 | implémentée |
| `CLI-SORT-02` | `sort` dit combien de sous-titres ont bougé, et écrit le fichier même s'il est déjà en ordre | 13 | implémentée |
| `CLI-TRANS-01` | `inspect -t` rapporte lignes rattachées, sous-titres nés, sans traduction et hors d'ordre | 13 | implémentée |
| `CLI-TRANS-02` | `--document translation` exige `-t`, et `-t` exige `--document translation` sur une sous-commande de texte | 13 | implémentée |
| `CLI-TRANS-03` | `-t` n'a de sens que pour une seule entrée ; avec un lot, il est refusé | 13 | implémentée |
| `CLI-TRANS-04` | seul le document visé est écrit, à son propre chemin et dans son propre format | 13 | implémentée |
| `CLI-TRANS-05` | `--align-method` choisit entre position (défaut) et numéro | 13 | implémentée |
| `CLI-LISTENC-01` | `--list-encodings` écrit les encodages qu'ICU sait convertir, un par ligne, et s'arrête | 13 | implémentée |
| `CLI-APPEND-01` | `append` ajoute les fichiers à la suite du premier, décalés de la fin du dernier sous-titre, dans une seule sortie | 13 | implémentée |
| `CLI-PSPLIT-01` | `split-file --at N` écrit les deux moitiés, la seconde ramenée à l'origine, ou refuse en nommant le sous-titre | 13 | implémentée |
| `CLI-PAIR-01` | `pair` écrit la traduction recalée sur les positions du principal, et dit l'alignement | 13 | implémentée |
| `GUI-SURFACE-01` | l'image de la vidéo est dessinée dans la fenêtre, redimensionnable, son rapport d'aspect conservé | 14 | implémentée |
| `GUI-SURFACE-02` | après un saut, l'image affichée est celle de la position demandée, la plus proche | 14 | implémentée |
| `GUI-SURFACE-03` | la fenêtre n'adopte aucune fenêtre native et n'exige pas X11 | 14 | implémentée |
| `GUI-STEP-01` | avancer d'une image affiche l'image suivante | 14 | implémentée |
| `GUI-STEP-02` | reculer d'une image affiche l'image précédente | 14 | implémentée |
| `GUI-STEP-03` | la durée d'une image vient de la vidéo, à défaut du document, à défaut de la grille ; sans aucune, le geste refuse en le disant | 14 | implémentée |
| `GUI-STEP-04` | le pas se règle en nombre d'images, au minimum une ; une valeur absurde ne casse pas l'ouverture | 14 | implémentée |
| `GUI-NUDGE-01` | décaler le début ou la fin d'un sous-titre d'une image est annulable en une entrée | 14 | implémentée |
| `GUI-SEEK-01` | le curseur de position se lit et se déplace | 14 | implémentée |
| `GUI-SEEK-02` | reculer et avancer d'un pas réglable | 14 | implémentée |
| `GUI-SEEK-03` | le sous-titre précédent et le suivant placent la lecture à leur début | 14 | implémentée |
| `GUI-SEEK-04` | le début et la fin de la sélection placent la lecture, avec l'avance réglée | 14 | implémentée |
| `GUI-SEEK-05` | jouer la sélection s'arrête à sa fin | 14 | implémentée |
| `GUI-VOLUME-01` | le volume se règle et se retient d'une session à l'autre | 14 | implémentée |
| `GUI-TIMECODE-01` | le timecode est incrusté sur l'image | 14 | implémentée |
| `GUI-AUDIO-01` | la piste audio se choisit parmi celles de la vidéo | 14 | implémentée |
| `GUI-MARK-01` | poser le début d'un sous-titre depuis la position de la vidéo, en une entrée d'historique | 14 | implémentée |
| `GUI-MARK-02` | poser sa fin depuis la position de la vidéo | 14 | implémentée |
| `GUI-MARK-03` | insérer un sous-titre à la position de la vidéo | 14 | implémentée |
| `GUI-MARK-04` | sélectionner le sous-titre précédent ou suivant depuis la position de la vidéo | 14 | implémentée |
| `GUI-FOLLOW-01` | la table centre le sous-titre courant pendant la lecture | 14 | implémentée |
| `GUI-FOLLOW-02` | un défilement à la main suspend le suivi | 14 | implémentée |
| `GUI-FOLLOW-03` | le suivi reprend sur un geste du lecteur ou à la demande | 14 | implémentée |
| `GUI-REPLICA-01` | la réplique dessinée sur l'image n'a plus de balises brutes | 14 | implémentée |
| `GUI-FRAMES-02` | les positions s'affichent et se saisissent en numéros d'image, la fréquence dite | 14 | prévue |
| `GUI-DRIFT-01` | l'ouverture d'une traduction dit qu'un décalage constant la rattacherait mieux, sans le dire d'une dérive | 14 | prévue |
| `GUI-DRIFT-02` | rouvrir décalée rattache selon le décalage proposé, en une entrée d'historique | 14 | prévue |
| `GUI-REPAIR-01` | la modale d'analyse propose la conversion de fréquence qui remet le fichier sur une grille, ou rien quand deux se valent | 14 | prévue |
| `GUI-REPAIR-02` | `Convert Frame Rate…` s'ouvre préremplie par cette proposition | 14 | prévue |
