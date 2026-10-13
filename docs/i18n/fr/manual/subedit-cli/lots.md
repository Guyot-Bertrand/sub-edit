# Traiter un arbre

```
subedit-cli <sous-commande> --recursive [options] <dossier>...
```

Toutes les sous-commandes qui lisent un lot de fichiers — `inspect` comprise, et non
[`append`](append.md), [`split-file`](split-file.md) ni [`pair`](pair.md), dont l'arité est celle d'une paire ou
d'un film — acceptent un **répertoire** à la place d'un fichier, avec `--recursive` (`-r`).
Cette page dit ce que le parcours prend, dans quel ordre, et où il écrit.

**Rien de ce qui suit ne change ce qui est fait à un fichier** : chacun est lu,
traité et écrit indépendamment, l'échec de l'un n'arrête pas les autres, et les
quatre codes de retour sont ceux de [l'invocation](invocation.md#codes-de-retour).
Le parcours produit une liste de chemins, que le reste du lot consomme comme s'ils
avaient été tapés.

## Sans `--recursive`, un répertoire est refusé

Erreur d'usage, code `1`, aucun fichier traité ni écrit :

<!-- exemple: mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; subedit-cli inspect films; echo "code=$?" -->
```console
$ mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; subedit-cli inspect films; echo "code=$?"
films: is a directory: use --recursive
code=1
```

## Ce que le parcours prend

Le parcours est **récursif, déterministe et sans surprise** :

- **l'ordre** — les entrées de chaque répertoire sont prises dans l'ordre de leurs
  noms, **octet par octet** (les majuscules avant les minuscules, sans égard à la
  langue de la machine), et un sous-répertoire est parcouru à sa place, en
  profondeur d'abord. Deux exécutions donnent les mêmes octets, sur toutes les
  machines ;
- **les liens symboliques ne sont pas suivis** — ni vers un fichier, ni vers un
  répertoire : un arbre qui contient un lien vers l'un de ses parents n'aurait pas
  de fin ;
- **les fichiers et dossiers cachés sont ignorés** — ceux dont le nom commence par
  un point ;
- **seules les extensions des formats connus sont retenues** : `.srt`, `.vtt`,
  `.ssa`, `.ass`, `.lrc`, `.sub`, sans égard à la casse. **`.txt` est exclu** : il
  désigne deux formats, mais surtout tous les `LISEZMOI.txt` du monde, et un lot qui
  échouerait sur chacun serait inutilisable.

**Un fichier nommé sur la ligne de commande n'est jamais filtré.** Le nommer, c'est
le vouloir : `subedit-cli shift --by 1 --in-place notes.txt` le traite, quelle que
soit son extension, et ne se plaint que si son contenu n'est dans aucun format.

**Ce qui a été laissé de côté est compté**, et dit au niveau `-vv` :

<!-- exemple: mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; printf 'note\n' > films/LISEZMOI.txt; subedit-cli -vv inspect --recursive films 2>&1 | head -1 -->
```console
$ mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; printf 'note\n' > films/LISEZMOI.txt; subedit-cli -vv inspect --recursive films 2>&1 | head -1
films: 1 file left aside, with no extension of a known format
```

Un répertoire qui ne contient rien de parcourable le dit au niveau par défaut
(`<dossier>: holds no file in a format this tool walks for`) et n'est pas une erreur.

## Où le parcours écrit

**L'arborescence est conservée sous `--output-dir`** : le chemin d'un fichier,
**relatif au répertoire donné**, est reproduit sous le dossier de sortie. Le nom
du répertoire donné n'en fait pas partie — c'est la sémantique de `rsync` avec une
barre finale, non celle de `cp -r` : on nomme la destination, et on y ajoute le
niveau qu'on veut (`--output-dir sortie/films`).

<!-- exemple: mkdir -p films/a/b; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/x.srt; cp films/x.srt films/a/x.srt; cp films/x.srt films/a/b/y.srt; printf 'note\n' > films/LISEZMOI.txt; subedit-cli shift --by 1 --recursive --output-dir sortie films; find sortie -type f | sort -->
```console
$ mkdir -p films/a/b; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/x.srt; cp films/x.srt films/a/x.srt; cp films/x.srt films/a/b/y.srt; printf 'note\n' > films/LISEZMOI.txt; subedit-cli shift --by 1 --recursive --output-dir sortie films; find sortie -type f | sort
films/a/b/y.srt: 1 subtitle shifted by 1.000 s -> sortie/a/b/y.srt
films/a/x.srt: 1 subtitle shifted by 1.000 s -> sortie/a/x.srt
films/x.srt: 1 subtitle shifted by 1.000 s -> sortie/x.srt
3 of 3 files shifted
sortie/a/b/y.srt
sortie/a/x.srt
sortie/x.srt
```

Le dossier de sortie et ceux de l'arborescence sont **créés** s'ils manquent. Avec
`--in-place`, chaque fichier retenu est réécrit à sa place. Une extension que
`convert` change se change aussi dans l'arborescence : `films/a/x.srt` converti
en WebVTT s'écrit `sortie/a/x.vtt`.

**Le dossier de sortie, s'il est dans l'arbre parcouru, est exclu du parcours** :
une seconde exécution ne relit pas ce que la première a écrit.

## Deux répertoires, et les collisions

La règle de [la destination](invocation.md#la-destination) tranche : deux fichiers
qui aboutiraient au même chemin sont refusés **avant tout écrit**, code `1`, les
deux entrées et la destination nommées.

<!-- exemple: mkdir -p un/a deux/a; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > un/a/x.srt; cp un/a/x.srt deux/a/x.srt; subedit-cli shift --by 1 -r --output-dir sortie un deux; echo "code=$?" -->
```console
$ mkdir -p un/a deux/a; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > un/a/x.srt; cp un/a/x.srt deux/a/x.srt; subedit-cli shift --by 1 -r --output-dir sortie un deux; echo "code=$?"
sortie/a/x.srt: would be written by both un/a/x.srt and deux/a/x.srt
code=1
```

## Ce que ça coûte

Le lot reste séquentiel : sa durée est la somme de celle de ses fichiers. Le banc
`make bench` mesure le parcours et le même travail en un seul fichier ; les chiffres
sont au [journal des performances](../../mesures/performances.md).
