# `convert`

```
subedit-cli convert --to srt|vtt
                    [--line-endings unix|windows|mac] [--bom | --no-bom]
                    (--output FICHIER | --output-dir DOSSIER | --in-place)
                    <fichier>...
```

Écrit chaque fichier dans le format et la forme demandés. **Ne modifie jamais
l'entrée**, sauf si `--in-place` le demande explicitement.

<!-- exemple: subedit-cli convert --help -->
```console
$ subedit-cli convert --help
Write a subtitle file out in another format or shape
Usage: subedit-cli convert [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to convert

Options:
  -h,--help                   Print this help message and exit
  --to TEXT:{srt,vtt} REQUIRED
                              Format to write
  --line-endings TEXT:{unix,windows,mac}
                              Line endings to write; the source's by default
  --bom                       Write a byte order mark
  --no-bom                    Write no byte order mark
  --output TEXT               File to write, for a single input
  --output-dir TEXT           Directory to write into
  --in-place                  Write back over the inputs
```

## Arguments et options

| Option | Requis | Valeurs | Défaut |
| :----- | :----- | :------ | :----- |
| `<fichier>...` | oui | un ou plusieurs chemins | — |
| `--to` | **oui** | `srt` ou `vtt`, et rien d'autre | — |
| `--line-endings` | non | `unix`, `windows` ou `mac` | celles du fichier lu |
| `--bom` / `--no-bom` | non | drapeaux, exclusifs l'un de l'autre | ce que portait le fichier lu |
| `--output` / `--output-dir` / `--in-place` | l'une des trois | voir [Invocation](invocation.md#la-destination) | — |

`mac` désigne le retour chariot seul (`\r`), la fin de ligne du Mac OS classique.

## La destination

Les trois options et leurs règles sont communes aux six sous-commandes qui
écrivent : voir [Invocation](invocation.md#la-destination). Rien n'est écrit
sans l'une d'elles.

Ce qui est propre à `convert` : **`--output-dir` change l'extension avec le
format.** `convert --to vtt` sur `a.srt` écrit `a.vtt`, jamais `a.srt` contenant
du WebVTT — un fichier dont le nom ment fait trébucher tous les autres outils.

## Convertir sur place est refusé

`--in-place` avec un `--to` qui change le format donne le code `1` et n'écrit
rien : sur place, il n'y a pas de second nom pour porter le nouveau format, et
le fichier resterait sous une extension que son contenu ne justifie plus.

`--in-place` reste utilisable pour changer **la forme sans le format** —
réécrire un `.srt` en `.srt` avec d'autres fins de ligne, par exemple.

Le refus se décide sur **l'extension seule**, avant toute lecture : une erreur
d'usage ne doit jamais laisser un lot à moitié écrit.

## Fins de ligne et marque d'ordre des octets

Par défaut, **le fichier écrit reprend ce que portait le fichier lu**. Le modèle
retient les deux à la lecture ; les imposer par défaut perdrait à chaque
conversion une information conservée exprès.

<!-- exemple: printf '1\n00:00:01,000 --> 00:00:03,000\nBonjour.\n\n' > a.srt; subedit-cli convert --to vtt --output b.vtt a.srt; cat b.vtt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nBonjour.\n\n' > a.srt; subedit-cli convert --to vtt --output b.vtt a.srt; cat b.vtt
a.srt: 1 subtitle written as WebVTT -> b.vtt
WEBVTT

00:01.000 --> 00:03.000
Bonjour.
```

## Sortie

**Sortie standard** — rien : le résultat est le fichier écrit.

Sur la sortie d'erreur, selon le niveau :

| Niveau | Ce qui s'ajoute |
| :----- | :-------------- |
| 1 | `<chemin>: N subtitles written as WebVTT -> <destination>` |
| 2 | `<chemin>: SubRip -> WebVTT, UTF-8, no BOM, LF line endings` — l'encodage nommé est celui qui est écrit |
| 3 | `<chemin>: N bytes read, M written`, puis **chaque diagnostic de lecture** — voir [Invocation](invocation.md#les-diagnostics-de-lecture) |

## Codes de retour

Ceux de l'outil : `0` si tous les fichiers ont été écrits, `2` si aucun, `3` si
certains seulement, `1` sur une erreur d'usage.

## Erreurs

| Ce qui la déclenche | Message |
| :------------------ | :------ |
| `--bom` avec `--no-bom` | `--bom and --no-bom ask for opposite things; give one or the other` |
| `--in-place` qui change le format | `--in-place cannot change the format: the file would keep a name its content no longer matches` |
| `--bom` sur un encodage qui n'en a pas | `<chemin>: <encodage> has no byte order mark to write` |

**Le dernier n'est pas une pédanterie.** Une marque d'ordre des octets existe
pour les encodages Unicode et pour aucun autre : la demander sur un fichier en
Windows-1252, c'est demander quelque chose qui n'existe pas. Le fichier
s'écrirait sans elle, et personne ne saurait que la demande n'a pas été
honorée.

Celles de la destination sont communes aux sous-commandes qui écrivent :
voir [Invocation](invocation.md#la-destination).

Les erreurs de lecture sont celles d'[`inspect`](inspect.md) : elles portent sur
le fichier d'entrée et ont les mêmes messages.
