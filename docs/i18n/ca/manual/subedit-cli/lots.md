# Processar un arbre

<!-- machine-translated -->
> Aquesta pàgina és una traducció automàtica i encara no l'ha revisada cap lector de la llengua.

```
subedit-cli <subcommand> --recursive [options] <directory>...
```

Cada subordre que llegeix un lot de fitxers — `inspect` inclosa, i no
[`append`](append.md), [`split-file`](split-file.md) ni [`pair`](pair.md), l'aritat dels quals és la d'un parell o
la d'una pel·lícula — accepta un **directori** en lloc d'un fitxer, amb `--recursive` (`-r`).
Aquesta pàgina diu què agafa el recorregut, en quin ordre, i on escriu.

**Res del que segueix no canvia el que es fa a un fitxer**: cadascun es llegeix,
es processa i s'escriu de manera independent, la fallada d'un no atura els altres, i els
quatre codis de sortida són els de [la invocació](invocation.md#codis-de-sortida).
El recorregut produeix una llista de camins, que la resta del lot consumeix com si
s'haguessin escrit a mà.

## Sense `--recursive`, un directori es refusa

Error d'ús, codi `1`, cap fitxer processat ni escrit:

<!-- example: mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; subedit-cli inspect films; echo "code=$?" -->
```console
$ mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; subedit-cli inspect films; echo "code=$?"
films: is a directory: use --recursive
code=1
```

## Què agafa el recorregut

El recorregut és **recursiu, determinista i sense sorpreses**:

- **l'ordre** — les entrades de cada directori es prenen en l'ordre dels seus
  noms, **byte a byte** (majúscules abans que minúscules, independentment de la
  llengua de la màquina), i un subdirectori es recorre al seu lloc, primer en
  profunditat. Dues execucions donen els mateixos bytes, a qualsevol
  màquina;
- **els enllaços simbòlics no se segueixen** — ni cap a un fitxer, ni cap a un
  directori: un arbre que contingués un enllaç a un dels seus pares no tindria
  fi;
- **els fitxers i directoris ocults s'ignoren** — aquells el nom dels quals comença amb
  un punt;
- **només es conserven les extensions dels formats coneguts**: `.srt`, `.vtt`,
  `.ssa`, `.ass`, `.lrc`, `.sub`, independentment de les majúscules. **`.txt` s'exclou**:
  anomena dos formats, però sobretot tots els `LISEZMOI.txt` del món, i un lot que
  fallés amb cadascun seria inservible.

**Un fitxer anomenat a la línia d'ordres no es filtra mai.** Anomenar-lo és
voler-lo: `subedit-cli shift --by 1 --in-place notes.txt` el processa, sigui quina sigui
la seva extensió, i només es queixa si el seu contingut no és en cap format.

**El que s'ha deixat de banda es compta**, i es diu al nivell `-vv`:

<!-- example: mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; printf 'note\n' > films/LISEZMOI.txt; subedit-cli -vv inspect --recursive films 2>&1 | head -1 -->
```console
$ mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; printf 'note\n' > films/LISEZMOI.txt; subedit-cli -vv inspect --recursive films 2>&1 | head -1
films: 1 file left aside, with no extension of a known format
```

Un directori que no conté res per recórrer ho diu al nivell per defecte
(`<directory>: holds no file in a format this tool walks for`) i no és un error.

## On escriu el recorregut

**L'arbre es conserva sota `--output-dir`**: el camí d'un fitxer,
**relatiu al directori indicat**, es reprodueix sota el directori de sortida. El nom
del directori indicat no en forma part — és la semàntica de `rsync` amb una
barra final, no la de `cp -r`: s'anomena la destinació, i s'hi afegeix el
nivell que es vulgui (`--output-dir sortie/films`).

<!-- example: mkdir -p films/a/b; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/x.srt; cp films/x.srt films/a/x.srt; cp films/x.srt films/a/b/y.srt; printf 'note\n' > films/LISEZMOI.txt; subedit-cli shift --by 1 --recursive --output-dir sortie films; find sortie -type f | sort -->
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

El directori de sortida i els de l'arbre **es creen** si falten. Amb
`--in-place`, cada fitxer conservat es torna a escriure al seu lloc. Una extensió que
`convert` canvia també canvia a l'arbre: `films/a/x.srt` convertit
a WebVTT s'escriu `sortie/a/x.vtt`.

**El directori de sortida, si és dins de l'arbre recorregut, s'exclou del recorregut**:
una segona execució no torna a llegir el que ha escrit la primera.

## Dos directoris, i les col·lisions

La regla de [la destinació](invocation.md#la-destinació) ho resol: dos fitxers
que anirien a parar al mateix camí es refusen **abans d'escriure res**, codi `1`, amb les
dues entrades i la destinació anomenades.

<!-- example: mkdir -p un/a deux/a; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > un/a/x.srt; cp un/a/x.srt deux/a/x.srt; subedit-cli shift --by 1 -r --output-dir sortie un deux; echo "code=$?" -->
```console
$ mkdir -p un/a deux/a; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > un/a/x.srt; cp un/a/x.srt deux/a/x.srt; subedit-cli shift --by 1 -r --output-dir sortie un deux; echo "code=$?"
sortie/a/x.srt: would be written by both un/a/x.srt and deux/a/x.srt
code=1
```

## Què costa

El lot continua sent seqüencial: la seva durada és la suma de les dels seus fitxers. El
banc de proves `make bench` mesura el recorregut i la mateixa feina en un sol fitxer; les xifres
són al [registre de rendiment](../../../../mesures/performances.md).
