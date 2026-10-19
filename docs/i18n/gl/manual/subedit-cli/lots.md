# Procesar unha árbore

<!-- machine-translated -->
> Esta páxina é unha tradución automática e aínda non a revisou ningún lector da lingua.

```
subedit-cli <subcommand> --recursive [options] <directory>...
```

Cada suborde que le un lote de ficheiros — `inspect` incluída, e non
[`append`](append.md), [`split-file`](split-file.md) nin [`pair`](pair.md), cuxa aridade é a dun par ou
a dunha película — acepta un **directorio** en vez dun ficheiro, con `--recursive` (`-r`).
Esta páxina di o que toma o percorrido, en que orde, e onde escribe.

**Nada do que segue cambia o que se lle fai a un ficheiro**: cada un lese,
procésase e escríbese de forma independente, o fallo dun non detén os outros, e os
catro códigos de saída son os da [invocación](invocation.md#códigos-de-saída).
O percorrido produce unha lista de rutas, que o resto do lote consome como se
se escribisen a man.

## Sen `--recursive`, un directorio rexéitase

Erro de uso, código `1`, ningún ficheiro procesado nin escrito:

<!-- example: mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; subedit-cli inspect films; echo "code=$?" -->
```console
$ mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; subedit-cli inspect films; echo "code=$?"
films: is a directory: use --recursive
code=1
```

## O que toma o percorrido

O percorrido é **recursivo, determinista e sen sorpresas**:

- **a orde** — as entradas de cada directorio tómanse na orde dos seus
  nomes, **byte a byte** (as maiúsculas antes das minúsculas, sexa cal sexa a
  lingua da máquina), e un subdirectorio percórrese no seu lugar, en
  profundidade. Dúas execucións dan os mesmos bytes, en calquera
  máquina;
- **as ligazóns simbólicas non se seguen** — nin cara a un ficheiro, nin cara a un
  directorio: unha árbore que contén unha ligazón cara a un dos seus pais non tería
  fin;
- **os ficheiros e directorios ocultos ignóranse** — aqueles cuxo nome comeza por
  un punto;
- **só se conservan as extensións dos formatos coñecidos**: `.srt`, `.vtt`,
  `.ssa`, `.ass`, `.lrc`, `.sub`, sen distinguir maiúsculas e minúsculas. **`.txt` queda excluído**:
  nomea dous formatos, pero sobre todo todos os `LISEZMOI.txt` do mundo, e un lote que
  fallase en cada un deles sería inutilizable.

**Un ficheiro nomeado na liña de ordes nunca se filtra.** Nomealo é
querelo: `subedit-cli shift --by 1 --in-place notes.txt` procésao, sexa cal sexa
a súa extensión, e só se queixa se o seu contido non está en ningún formato.

**O que se deixou de lado cóntase**, e dise no nivel `-vv`:

<!-- example: mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; printf 'note\n' > films/LISEZMOI.txt; subedit-cli -vv inspect --recursive films 2>&1 | head -1 -->
```console
$ mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; printf 'note\n' > films/LISEZMOI.txt; subedit-cli -vv inspect --recursive films 2>&1 | head -1
films: 1 file left aside, with no extension of a known format
```

Un directorio que non contén nada que percorrer dio no nivel predeterminado
(`<directory>: holds no file in a format this tool walks for`) e non é un erro.

## Onde escribe o percorrido

**A árbore consérvase baixo `--output-dir`**: a ruta dun ficheiro,
**relativa ao directorio dado**, reprodúcese baixo o directorio de saída. O nome
do directorio dado non forma parte dela — é a semántica de `rsync` cunha
barra final, non a de `cp -r`: nómease o destino, e engádeselle o
nivel que se queira (`--output-dir sortie/films`).

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

O directorio de saída e os da árbore **créanse** se faltan. Con
`--in-place`, cada ficheiro conservado escríbese de novo no seu lugar. Unha extensión que
cambia `convert` cambia tamén na árbore: `films/a/x.srt` convertido
a WebVTT escríbese `sortie/a/x.vtt`.

**O directorio de saída, se está dentro da árbore percorrida, exclúese do percorrido**:
unha segunda execución non le de novo o que escribiu a primeira.

## Dous directorios, e colisións

A regra do [destino](invocation.md#o-destino) resólveo: dous ficheiros
que acabarían na mesma ruta rexéitanse **antes de escribir nada**, código `1`, coas
dúas entradas e o destino nomeados.

<!-- example: mkdir -p un/a deux/a; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > un/a/x.srt; cp un/a/x.srt deux/a/x.srt; subedit-cli shift --by 1 -r --output-dir sortie un deux; echo "code=$?" -->
```console
$ mkdir -p un/a deux/a; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > un/a/x.srt; cp un/a/x.srt deux/a/x.srt; subedit-cli shift --by 1 -r --output-dir sortie un deux; echo "code=$?"
sortie/a/x.srt: would be written by both un/a/x.srt and deux/a/x.srt
code=1
```

## O que custa

O lote segue sendo secuencial: a súa duración é a suma das dos seus ficheiros. O
banco de probas de rendemento `make bench` mide o percorrido e o mesmo traballo nun só ficheiro; as cifras
están no [rexistro de rendemento](../../../../mesures/performances.md).
