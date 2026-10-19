# `transform`

<!-- machine-translated -->
> Esta páxina é unha tradución automática e aínda non a revisou ningún lector da lingua.

```
subedit-cli transform --first <index>=<time> --last <index>=<time>
                      (--output FILE | --output-dir DIR | --in-place)
                      [--dry-run]
                      [--recursive]
                      <file>...
```

Corrixe **todas** as posicións do ficheiro a partir de dous puntos cuxo valor correcto
se coñece. É a operación que require unha subtitulación que se desvía
(correcta ao comezo, errada ao final), onde [`shift`](shift.md), que move todo na
mesma cantidade, non pode facer nada.

O formato do ficheiro lido **consérvase**: cambiar o formato é tarefa de
[`convert`](convert.md).

<!-- example: subedit-cli transform --help -->
```console
$ subedit-cli transform --help
Correct every position from two points known to be right
Usage: subedit-cli transform [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to transform

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --first TEXT REQUIRED       Earlier reference, as <index>=<time>: 1=00:00:01.000
  --last TEXT REQUIRED        Later reference, as <index>=<time>: 3=00:00:10.000
  --output TEXT               File to write, for a single input
  --output-dir TEXT           Directory to write into
  --in-place                  Write back over the inputs
  --dry-run                   Work out and say what would be written, and write nothing
```

## Argumentos e opcións

| Opción | Obrigatorio | Valores | Predeterminado |
| :----- | :----- | :------ | :----- |
| `<file>...` | si | unha ou máis rutas | — |
| `--recursive`, `-r` | non | un indicador | desactivado — véxase [Procesar unha árbore](lots.md) |
| `--first` | **si** | unha referencia, `<index>=<time>` | — |
| `--last` | **si** | unha referencia, `<index>=<time>` | — |
| `--output` / `--output-dir` / `--in-place` | **unha das tres**, agás con `--dry-run` | véxase [Invocación](invocation.md#o-destino) | — |
| `--dry-run` | non | un indicador: calcula e di, non escribe nada — véxase [Ver antes de escribir](invocation.md#ver-antes-de-escribir) | desactivado |

Non hai forma curta: estas dúas opcións só se escriben
enteiras.

## Escribir unha referencia

Unha referencia nomea un subtítulo e di onde debe caer agora o seu **inicio**:

```
<index>=<time>
```

**O índice cóntase desde 1**, como se mostra e como o numera un ficheiro
SubRip. O `0` rexéitase, e tamén un índice posterior ao último
subtítulo.

**O tempo escríbese como o de [`shift`](shift.md)**: segundos ou
marca de tempo, o separador decimal é un punto, a resolución é o milisegundo:

| Forma | Exemplos |
| :---- | :------- |
| segundos | `10`, `2.999` |
| marca de tempo | `00:00:10.000`, `01:39:37.040` |

`--first 3=10` e `--first 3=00:00:10.000` din o mesmo.

As opcións `--first` e `--last` nomean dous puntos, non un intervalo. Os seus nomes din
o uso esperado (unha referencia ao comezo do ficheiro, a outra ao final) e non unha
restrición: intercambialas dá o mesmo resultado, e **todo o que está fóra
das dúas referencias segue do mesmo xeito que o que está entre elas**.

## En que se converten as outras posicións

As dúas referencias fixan un factor de escala, e todas as posicións o seguen:

```
t′ = y₁ + (t − x₁) × r      with r = (y₂ − y₁) / (x₂ − x₁)
```

`x₁`, `x₂` son os inicios actuais dos dous subtítulos nomeados, `y₁`, `y₂` os
tempos pedidos. O factor gárdase como un racional exacto e **o redondeo
prodúcese unha soa vez**, ao milisegundo; iso é o que fai que as dúas
referencias caian **exactamente** nos tempos pedidos e non cunha marxe dunha milésima
(véxase [ADR 0013](../../../../adr/0013-mise-a-l-echelle-exacte-des-positions.md)).

Cada subtítulo queda na pantalla **en proporción** ao tempo que quedaba:
o final segue a mesma escala que o inicio. Unha transformación non é un
desprazamento, e non ten a súa propiedade.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nUn.\n\n2\n00:00:05,001 --> 00:00:07,000\nDeux.\n\n3\n00:00:09,000 --> 00:00:11,000\nTrois.\n\n' > a.srt; subedit-cli transform --first 1=00:00:01.000 --last 3=00:00:10.000 --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nUn.\n\n2\n00:00:05,001 --> 00:00:07,000\nDeux.\n\n3\n00:00:09,000 --> 00:00:11,000\nTrois.\n\n' > a.srt; subedit-cli transform --first 1=00:00:01.000 --last 3=00:00:10.000 --output b.srt a.srt; cat b.srt
a.srt: 3 subtitles transformed onto 1=00:00:01.000 and 3=00:00:10.000 -> b.srt
1
00:00:01,000 --> 00:00:03,250
Un.

2
00:00:05,501 --> 00:00:07,750
Deux.

3
00:00:10,000 --> 00:00:12,250
Trois.
```

A primeira referencia non se move, a terceira pasa de 9 s a 10 s: o factor é
`9/8`, e o segundo subtítulo, que ninguén nomeou, pasa de
`5,001` a `5,501`, é dicir, `1000 + 4001 × 9/8` redondeado unha vez.

## Dúas referencias que non o son

| Situación | Cando se ve | Código |
| :-------- | :------------- | :--- |
| `--first` e `--last` nomean o mesmo índice | antes de calquera lectura | `1` |
| un índice está máis alá do último subtítulo | ao abrir o ficheiro | `2` ou `3` |
| os dous subtítulos nomeados comezan no mesmo momento | ao abrir o ficheiro | `2` ou `3` |

Os tres rexéitanse polo mesmo motivo: `x₂ − x₁` é cero, e dous
puntos coincidentes non definen ningunha recta. Devolver unha división por cero disfrazada de
resultado sería peor que rexeitala.

O primeiro caso é un **erro de uso**: as dúas opcións son as culpables sexan cales sexan
os ficheiros, polo que corresponde unha soa mensaxe, non unha por ficheiro. Os
outros dous dependen do ficheiro aberto e, polo tanto, contan como un fallo de
procesamento.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nUn.\n\n' > a.srt; subedit-cli transform --first 2=1.000 --last 2=4.000 --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nUn.\n\n' > a.srt; subedit-cli transform --first 2=1.000 --last 2=4.000 --output b.srt a.srt; echo "code=$?"
--first and --last both name subtitle 2, and two references on one subtitle define no transform
code=1
```

Un índice fóra dos límites rexéitase **nomeando o límite**, o que di ao
mesmo tempo o que se podería ter escrito:

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nUn.\n\n2\n00:00:05,000 --> 00:00:07,000\nDeux.\n\n' > a.srt; subedit-cli transform --first 1=1.000 --last 9=10.000 --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nUn.\n\n2\n00:00:05,000 --> 00:00:07,000\nDeux.\n\n' > a.srt; subedit-cli transform --first 1=1.000 --last 9=10.000 --output b.srt a.srt; echo "code=$?"
a.srt: subtitle 9 is past the end: the file holds 2 subtitles
code=2
```

## Rexéitase unha posición antes da orixe

Como en [`shift`](shift.md), e polo mesmo motivo: unha posición negativa
escríbese `-00:00:01,000`, unha forma que ningún dos nove formatos define e que ningún
lector acepta. O caso dáse aquí cando un subtítulo está **antes
da primeira referencia** e a escala o empurra máis alá do cero. Non se
escribe nada, o código é `2`, e a mensaxe nomea o subtítulo culpable.

## Saída

**Saída estándar** — nada: o resultado é o ficheiro escrito. Con `--dry-run`, tampouco nada: non hai ficheiro, e a liña de narración dio — véxase [Ver antes de escribir](invocation.md#ver-antes-de-escribir).

| Nivel | O que se engade na saída de erro |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles transformed onto 1=00:00:01.000 and 3=00:00:10.000 -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — o formato, a codificación, a marca e os finais de liña do ficheiro lido, repostos tal como estaban |
| 3 | `<path>: N bytes read, M written`, e despois **todos os diagnósticos de lectura** — véxase [Invocación](invocation.md#diagnósticos-de-lectura) |

As referencias vólvense escribir co punto decimal, sexa cal sexa a forma en que se teclearon.

### En JSON

Con [`--format json`](invocation.md#saída-que-pode-ler-un-script), o obxecto de cada ficheiro leva `counts.subtitles` (todos os subtítulos do ficheiro). A táboa completa é a de [Invocación](invocation.md#o-que-pon-cada-suborde-en-counts).

## Códigos de saída

Os da ferramenta: `0` se se escribiron todos os ficheiros, `2` se non se escribiu ningún, `3` se
só se escribiron algúns, `1` nun erro de uso.

## Erros

| O que o provoca | Mensaxe |
| :------------------ | :------ |
| referencia sen `=` | `"…" is not a reference: write it <index>=<time>, as in 3=00:00:10.000` |
| referencia á que lle falta unha metade | `"…" is missing one of its two halves: write it <index>=<time>, as in 3=00:00:10.000` |
| índice `0` | `"0" is not a subtitle number: subtitles are counted from 1, as the file shows them` |
| índice que non é un número enteiro | `"…" is not a subtitle number: expected a whole number, counted from 1 as the file shows them` |
| tempo non válido | os de [`shift`](shift.md) |
| `--first` e `--last` no mesmo índice | `--first and --last both name subtitle N, and two references on one subtitle define no transform` |
| índice fóra dos límites | `<path>: subtitle N is past the end: the file holds M subtitles` |
| dous subtítulos que comezan xuntos | `<path>: subtitles N and M start at the same moment, so they define no transform` |
| posición antes da orixe | `<path>: subtitle N would land before the origin, which no subtitle file can hold` |

**Non se escribe nada sen unha das tres opcións de destino**, e os seus
erros son comúns ás subordes que escriben:
véxase [Invocación](invocation.md#o-destino).
