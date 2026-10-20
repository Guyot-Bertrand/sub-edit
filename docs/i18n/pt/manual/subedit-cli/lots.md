# Processar uma árvore de pastas

<!-- machine-translated -->
> Esta página é uma tradução automática e ainda não foi revista por nenhum leitor do idioma.

```
subedit-cli <subcommand> --recursive [options] <directory>...
```

Qualquer subcomando que leia um lote de ficheiros — `inspect` incluído, e não [`append`](append.md), [`split-file`](split-file.md) nem [`pair`](pair.md), cuja aridade é a de um par ou a de um filme — aceita uma **pasta** no lugar de um ficheiro, com `--recursive` (`-r`).
Esta página diz o que a travessia recolhe, por que ordem, e onde escreve.

**Nada do que se segue altera o que é feito a um ficheiro**: cada um é lido,
processado e escrito de forma independente, a falha de um não interrompe os outros, e os
quatro códigos de saída são os da ferramenta — ver [a invocação](invocation.md#códigos-de-saída).
A travessia produz uma lista de caminhos, que o resto do lote consome como se
tivessem sido escritos à mão.

## Sem `--recursive`, uma pasta é recusada

Erro de utilização, código `1`, nenhum ficheiro processado nem escrito:

<!-- example: mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; subedit-cli inspect films; echo "code=$?" -->
```console
$ mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; subedit-cli inspect films; echo "code=$?"
films: is a directory: use --recursive
code=1
```

## O que a travessia recolhe

A travessia é **recursiva, determinista e sem surpresas**:

- **a ordem** — as entradas de cada pasta são tomadas pela ordem dos seus
  nomes, **byte a byte** (as maiúsculas antes das minúsculas, seja qual for o
  idioma da máquina), e uma subpasta é percorrida no seu lugar, em
  profundidade. Duas execuções dão os mesmos bytes, em todas as
  máquinas;
- **as ligações simbólicas não são seguidas** — nem para um ficheiro, nem para uma
  pasta: uma árvore que contivesse uma ligação para um dos seus ascendentes não teria
  fim;
- **os ficheiros e as pastas ocultos são ignorados** — aqueles cujo nome começa por
  um ponto;
- **só se mantêm as extensões dos formatos conhecidos**: `.srt`, `.vtt`,
  `.ssa`, `.ass`, `.lrc`, `.sub`, sem distinção entre maiúsculas e minúsculas. **`.txt` fica excluído**: designa
  dois formatos, mas sobretudo todos os `LISEZMOI.txt` do mundo, e um lote que
  falhasse em cada um deles seria inutilizável.

**Um ficheiro indicado na linha de comandos nunca é filtrado.** Indicá-lo é
querê-lo: `subedit-cli shift --by 1 --in-place notes.txt` processa-o, seja qual for
a sua extensão, e só se queixa se o conteúdo não estiver em nenhum formato.

**O que foi deixado de lado é contado**, e dito ao nível `-vv`:

<!-- example: mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; printf 'note\n' > films/LISEZMOI.txt; subedit-cli -vv inspect --recursive films 2>&1 | head -1 -->
```console
$ mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; printf 'note\n' > films/LISEZMOI.txt; subedit-cli -vv inspect --recursive films 2>&1 | head -1
films: 1 file left aside, with no extension of a known format
```

Uma pasta que não contém nada para percorrer di-lo ao nível predefinido
(`<directory>: holds no file in a format this tool walks for`) e isso não é um erro.

## Onde a travessia escreve

**A árvore é mantida sob `--output-dir`**: o caminho de um ficheiro,
**relativo à pasta indicada**, é reproduzido sob a pasta de saída. O nome
da pasta indicada não faz parte dele — é a semântica do `rsync` com uma
barra final, e não a do `cp -r`: indica-se o destino e acrescenta-se-lhe o
nível que se quiser (`--output-dir sortie/films`).

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

A pasta de saída e as da árvore são **criadas** se não existirem. Com
`--in-place`, cada ficheiro mantido é escrito de volta no seu lugar. Uma extensão que
`convert` altera também se altera na árvore: `films/a/x.srt` convertido
para WebVTT é escrito `sortie/a/x.vtt`.

**A pasta de saída, se estiver dentro da árvore percorrida, é excluída da travessia**:
uma segunda execução não volta a ler o que a primeira escreveu.

## Duas pastas e colisões

A regra é a mesma — ver [o destino](invocation.md#o-destino): dois ficheiros
que acabariam no mesmo caminho são recusados **antes de se escrever fosse o que fosse**, código `1`, com os
dois ficheiros de entrada e o destino nomeados.

<!-- example: mkdir -p un/a deux/a; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > un/a/x.srt; cp un/a/x.srt deux/a/x.srt; subedit-cli shift --by 1 -r --output-dir sortie un deux; echo "code=$?" -->
```console
$ mkdir -p un/a deux/a; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > un/a/x.srt; cp un/a/x.srt deux/a/x.srt; subedit-cli shift --by 1 -r --output-dir sortie un deux; echo "code=$?"
sortie/a/x.srt: would be written by both un/a/x.srt and deux/a/x.srt
code=1
```

## O que custa

O lote continua sequencial: a sua duração é a soma das dos seus ficheiros. O
teste de desempenho `make bench` mede a travessia e o mesmo trabalho num único ficheiro; os valores
estão no [registo de desempenho](../../../../mesures/performances.md).
