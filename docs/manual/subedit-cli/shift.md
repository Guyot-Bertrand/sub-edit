# `shift`

```
subedit-cli shift --by <temps>
                  (--output FICHIER | --output-dir DOSSIER | --in-place)
                  <fichier>...
```

Décale **toutes** les positions du fichier de la même durée, dans un sens ou
dans l'autre. Aucun sous-titre ne reste à l'écran plus ou moins longtemps :
c'est ce qui fait d'un décalage un décalage.

<!-- exemple: subedit-cli shift --help -->
```console
$ subedit-cli shift --help
Move every position of a file by a fixed amount
Usage: subedit-cli shift [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to shift

Options:
  -h,--help                   Print this help message and exit
  --by TEXT REQUIRED          Amount to move by: 2.999, -7.001, or 00:00:07.001
  --output TEXT               File to write, for a single input
  --output-dir TEXT           Directory to write into
  --in-place                  Write back over the inputs
```

## Arguments et options

| Option | Requis | Valeurs | Défaut |
| :----- | :----- | :------ | :----- |
| `<fichier>...` | oui | un ou plusieurs chemins | — |
| `--by` | **oui** | une durée signée, voir ci-dessous | — |
| `--output` / `--output-dir` / `--in-place` | **l'une des trois** | voir [Invocation](invocation.md#la-destination) | — |

Le format du fichier lu est **conservé** : changer de format est le travail de
[`convert`](convert.md).

## Écrire une durée

Deux formes, et deux seulement :

| Forme | Exemples |
| :---- | :------- |
| secondes | `30`, `2.999`, `-7.001`, `+1.5` |
| horodatage | `00:00:07.001`, `01:39:37.040`, `-00:00:07.001` |

Les deux disent la même chose : `--by 7.001` et `--by 00:00:07.001` produisent
le même fichier.

**Le séparateur décimal est le point**, la ligne de commande parlant anglais.
Une virgule est refusée plutôt qu'interprétée : la moitié du monde écrit `7,001`
pour sept mille un, et lire cela comme sept secondes serait une supposition —
fausse pour cette moitié-là.

**La résolution est la milliseconde.** Plus fin est refusé plutôt qu'arrondi en
silence : un arrondi tacite donnerait un résultat qui ne correspond à rien de ce
qui a été demandé.

<!-- exemple: printf '1\n00:00:10,000 --> 00:00:12,000\nBonjour.\n\n' > a.srt; subedit-cli shift --by -7.001 --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:10,000 --> 00:00:12,000\nBonjour.\n\n' > a.srt; subedit-cli shift --by -7.001 --output b.srt a.srt; cat b.srt
a.srt: 1 subtitle shifted by -7.001 s -> b.srt
1
00:00:02,999 --> 00:00:04,999
Bonjour.
```

## Un décalage avant l'origine est refusé

Reculer au-delà de zéro donne le code `2` et **n'écrit rien**. Le message nomme
le sous-titre qui ne pouvait pas encaisser le décalage, ce qui dit du même coup
jusqu'où l'on peut aller.

La raison n'est pas un principe : une position avant l'origine s'écrit
`-00:00:01,000`, forme qu'aucun des deux formats ne définit et qu'aucun lecteur
n'accepte. Le noyau, lui, autorise ces positions — les refuser ferait d'une
opération d'édition un cas particulier — mais un fichier est autre chose qu'un
modèle en mémoire.

<!-- exemple: printf '1\n00:00:01,000 --> 00:00:03,000\nBonjour.\n\n' > a.srt; subedit-cli shift --by -7.001 --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nBonjour.\n\n' > a.srt; subedit-cli shift --by -7.001 --output b.srt a.srt; echo "code=$?"
a.srt: subtitle 1 would start before the origin, which no subtitle file can hold
code=2
```

## Sortie

**Sortie standard** — rien : le résultat est le fichier écrit.

| Niveau | Ce qui s'ajoute sur la sortie d'erreur |
| :----- | :------------------------------------- |
| 1 | `<chemin>: N subtitles shifted by 2.999 s -> <destination>` |
| 2 | `<chemin>: SubRip, UTF-8, no BOM, LF line endings kept` |
| 3 | `<chemin>: N bytes read, M written`, puis **chaque diagnostic de lecture** — voir [Invocation](invocation.md#les-diagnostics-de-lecture) |

## Codes de retour

Ceux de l'outil : `0` si tous les fichiers ont été écrits, `2` si aucun, `3` si
certains seulement, `1` sur une erreur d'usage.

## Erreurs

| Ce qui la déclenche | Message |
| :------------------ | :------ |
| virgule décimale | `"…" is not a time: use a decimal point and not a comma, the command line being English` |
| plus fin que la milliseconde | `"…" is not a time: positions are held to the millisecond, no finer` |
| texte qui n'est pas une durée | `"…" is not a time: expected seconds like 2.999, or a timestamp like 00:00:07.001` |
| décalage avant l'origine | `<chemin>: subtitle N would start before the origin, which no subtitle file can hold` |

**Rien n'est écrit sans l'une des trois options de destination**, et ses
erreurs sont communes aux quatre sous-commandes qui écrivent :
voir [Invocation](invocation.md#la-destination).
