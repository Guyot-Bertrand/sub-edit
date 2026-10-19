# `shift`

<!-- machine-translated -->
> Esta páxina é unha tradución automática e aínda non a revisou ningún lector da lingua.

```
subedit-cli shift (--by <time> | --to-grid)
                  (--output FILE | --output-dir DIR | --in-place)
                  [--dry-run]
                  [--recursive]
                  <file>...
```

Despraza **todas** as posicións do ficheiro na mesma cantidade, nun sentido ou
no outro. Ningún subtítulo queda máis nin menos tempo na pantalla:
iso é o que fai que un desprazamento sexa un desprazamento.

<!-- example: subedit-cli shift --help -->
```console
$ subedit-cli shift --help
Move every position of a file by a fixed amount
Usage: subedit-cli shift [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to shift

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --by TEXT                   Amount to move by: 2.999, -7.001, or 00:00:07.001
  --to-grid                   Move by the amount that puts the positions back on their frame grid
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
| `--by` | **unha das dúas** | unha duración con signo, véxase máis abaixo | — |
| `--to-grid` | **unha das dúas** | indicador: a cantidade mídese, véxase máis abaixo | — |
| `--output` / `--output-dir` / `--in-place` | **unha das tres**, agás con `--dry-run` | véxase [Invocación](invocation.md#o-destino) | — |
| `--dry-run` | non | un indicador: calcula e di, non escribe nada — véxase [Ver antes de escribir](invocation.md#ver-antes-de-escribir) | desactivado |

O formato do ficheiro lido **consérvase**: cambiar o formato é tarefa de
[`convert`](convert.md).

## `--to-grid`: a cantidade medida en vez de dada

Un ficheiro cuxas posicións están nunha grade de fotogramas **salvo
unha constante** foi desprazado, e esa constante pódese medir nas propias posicións.
A opción `--to-grid` mídea e aplícaa, sen que ninguén teña que coñecela.

A cantidade é **propia de cada ficheiro**: dous ficheiros desprazados fóra da mesma
grade en dúas cantidades distintas volven cada un pola súa. Escríbese
na narración, xunto coa taxa de fotogramas atopada.

**O camiño é o máis curto.** Pasado medio fotograma, a liña da grade máis próxima
é a seguinte: a corrección avanza en vez de retroceder case
un fotograma enteiro.

<!-- example: k=0; while [ $k -lt 12 ]; do s=$((1000 + k*47000 + (k%3)*40 + 7)); e=$((s+2000)); k=$((k+1)); printf '%d\n%02d:%02d:%02d,%03d --> %02d:%02d:%02d,%03d\nRéplique %d.\n\n' $k $((s/3600000)) $((s/60000%60)) $((s/1000%60)) $((s%1000)) $((e/3600000)) $((e/60000%60)) $((e/1000%60)) $((e%1000)) $k; done > decale.srt; subedit-cli shift --to-grid --output recale.srt decale.srt -->
```console
$ k=0; while [ $k -lt 12 ]; do s=$((1000 + k*47000 + (k%3)*40 + 7)); e=$((s+2000)); k=$((k+1)); printf '%d\n%02d:%02d:%02d,%03d --> %02d:%02d:%02d,%03d\nRéplique %d.\n\n' $k $((s/3600000)) $((s/60000%60)) $((s/1000%60)) $((s%1000)) $((e/3600000)) $((e/60000%60)) $((e/1000%60)) $((e%1000)) $k; done > decale.srt; subedit-cli shift --to-grid --output recale.srt decale.srt
decale.srt: 12 subtitles shifted by -0.007 s onto their 25 fps grid -> recale.srt
```

**Un ficheiro que non está en ningunha grade coñecida rexéitase, e dise por que.** Non
hai nada ao que volver, e unha fase medida sobre ruído movería o ficheiro
ao chou. Véxase [`inspect`](inspect.md#o-que-indica-frame-rate-grid) para
saber o que dá un ficheiro antes de intentar a operación.

As opcións `--by` e `--to-grid` din ambas canto desprazar: dalas xuntas
é un erro de uso, e tamén o é non dar ningunha.

**A opción `--to-grid` non é [`snap`](snap.md).** Despraza todo o ficheiro
na mesma cantidade e conserva a temporización relativa **exactamente**; `snap`
move cada posición de forma independente e absorbe de paso as que
se corrixiran a man. Nun ficheiro limpo os dous dan o mesmo
resultado; nun ficheiro parcial, non.

## Escribir unha duración

Dúas formas, e só dúas:

| Forma | Exemplos |
| :---- | :------- |
| segundos | `30`, `2.999`, `-7.001`, `+1.5` |
| marca de tempo | `00:00:07.001`, `01:39:37.040`, `-00:00:07.001` |

As dúas din o mesmo: `--by 7.001` e `--by 00:00:07.001` producen
o mesmo ficheiro.

**O separador decimal é o punto**, porque a liña de ordes fala inglés.
Unha coma rexéitase en vez de interpretarse: medio mundo escribe `7,001`
para sete mil un, e lelo como sete segundos sería unha adiviña,
errada para esa metade.

**A resolución é o milisegundo.** O que sexa máis fino rexéitase en vez de redondearse
en silencio: un redondeo tácito daría un resultado que non coincide con nada do que
se pediu.

<!-- example: printf '1\n00:00:10,000 --> 00:00:12,000\nBonjour.\n\n' > a.srt; subedit-cli shift --by -7.001 --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:10,000 --> 00:00:12,000\nBonjour.\n\n' > a.srt; subedit-cli shift --by -7.001 --output b.srt a.srt; cat b.srt
a.srt: 1 subtitle shifted by -7.001 s -> b.srt
1
00:00:02,999 --> 00:00:04,999
Bonjour.
```

## Rexéitase un desprazamento antes da orixe

Retroceder máis alá do cero dá o código `2` e **non escribe nada**. A mensaxe nomea
o subtítulo que non podía admitir o desprazamento, o que di ao mesmo tempo
ata onde se pode chegar.

O motivo non é un principio: unha posición anterior á orixe escríbese
`-00:00:01,000`, unha forma que ningún dos nove formatos define e que ningún lector
acepta. O núcleo si admite esas posicións (rexeitalas faría dunha
operación de edición un caso especial), pero un ficheiro é outra cousa que un
modelo en memoria.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nBonjour.\n\n' > a.srt; subedit-cli shift --by -7.001 --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nBonjour.\n\n' > a.srt; subedit-cli shift --by -7.001 --output b.srt a.srt; echo "code=$?"
a.srt: subtitle 1 would start before the origin, which no subtitle file can hold
code=2
```

## Saída

**Saída estándar** — nada: o resultado é o ficheiro escrito. Con `--dry-run`, tampouco nada: non hai ficheiro, e a liña de narración dio — véxase [Ver antes de escribir](invocation.md#ver-antes-de-escribir).

| Nivel | O que se engade na saída de erro |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles shifted by 2.999 s -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — o formato, a codificación, a marca e os finais de liña do ficheiro lido, repostos tal como estaban |
| 3 | `<path>: N bytes read, M written`, e despois **todos os diagnósticos de lectura** — véxase [Invocación](invocation.md#diagnósticos-de-lectura) |

### En JSON

Con [`--format json`](invocation.md#saída-que-pode-ler-un-script), o obxecto de cada ficheiro leva `counts.subtitles` (todos os subtítulos do ficheiro) e `counts.shifted_by_ms` (o desprazamento, en milisegundos enteiros, **con signo**). A táboa completa é a de [Invocación](invocation.md#o-que-pon-cada-suborde-en-counts).

## Códigos de saída

Os da ferramenta: `0` se se escribiron todos os ficheiros, `2` se non se escribiu ningún, `3` se
só se escribiron algúns, `1` nun erro de uso.

## Erros

| O que o provoca | Mensaxe |
| :------------------ | :------ |
| coma decimal | `"…" is not a time: use a decimal point and not a comma, in every language` |
| máis fino que o milisegundo | `"…" is not a time: positions are held to the millisecond, no finer` |
| texto que non é unha duración | `"…" is not a time: expected seconds like 2.999, or a timestamp like 00:00:07.001` |
| desprazamento antes da orixe | `<path>: subtitle N would start before the origin, which no subtitle file can hold` |

**Non se escribe nada sen unha das tres opcións de destino**, e os seus
erros son comúns ás subordes que escriben:
véxase [Invocación](invocation.md#o-destino).
