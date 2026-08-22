# Fixtures vidéo

Deux conteneurs minuscules, dont on ne lit jamais une image : ce qu'on y lit,
c'est **une fréquence d'image et une durée**, les deux métadonnées dont la
phase 6 a besoin et qu'un fichier de sous-titres ne déclare pas.

| Fichier | Fréquence | Durée | Poids |
| :------ | :-------- | ----: | ----: |
| `cadence-25.mp4` | `25/1` — PAL, entière | 2,000 s | 1 504 o |
| `cadence-23-976.mp4` | `24000/1001` — NTSC, fractionnaire | 2,002 s | 1 519 o |

La seconde est celle qui dit quelque chose. Une fixture à fréquence entière
seule laisserait croire à une lecture juste là où le noyau manipule des
rationnels exacts depuis la phase 1.

**Ne pas les éditer à la main, et ne pas les croire sur parole.**
[`src/scripts/video-fixtures.sh`](../../../scripts/video-fixtures.sh) porte la
commande qui les fabrique et la table de ce qu'on en attend :

```console
$ ./src/scripts/video-fixtures.sh --check      # ce que ffprobe en dit, contre la table
$ ./src/scripts/video-fixtures.sh --generate   # les refabriquer
```

`--check` tourne dans `make check-local`. C'est là qu'est la garantie : un
conteneur est illisible dans un diff, et personne ne relira ces 3 Ko.
