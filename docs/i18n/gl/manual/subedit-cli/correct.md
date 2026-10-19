# `correct`

<!-- machine-translated -->
> Esta páxina é unha tradución automática e aínda non a revisou ningún lector da lingua.

```
subedit-cli correct --tasks LIST --code CODE
                    [--classes human|ocr|human,ocr]
                    [--enable NAME]... [--disable NAME]...
                    [--language CODE]
                    [--max-length N] [--max-lines N]
                    [--skip-length N|off] [--skip-lines N|off]
                    [--keep-blank-subtitles | --remove-blank-subtitles]
                    [--range N-M|N-]
                    [--document main|translation] [-t FILE] [--align-method position|number]
                    (--output FILE | --output-dir DIR | --in-place)
                    [--dry-run]
                    [--recursive]
                    <file>...
```

Corrixe os textos cos **padróns de Gaupol**, como o asistente
[`Correct Texts…`](../subedit-gui/correct-texts.md) da xanela — os mesmos padróns, o mesmo
motor, os mesmos recontos. Aquí existen seis tarefas: as **mencións para persoas xordas**, a
**unión** e a **división de palabras**, os **erros comúns**, as **maiúsculas** e o **salto de
liña**.

<!-- example: subedit-cli correct --help -->
```console
$ subedit-cli correct --help
Correct the texts with the patterns of the correction assistant
Usage: subedit-cli correct [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to correct

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --tasks LIST                The tasks to run, in a list: none is chosen beforehand
  --code CODE                 The cascade of patterns the tasks read: Zyyy, Latn, Latn-en, Latn-en-US
  --classes human|ocr|human,ocr
                              The classes of common errors to apply: both by default
  --enable NAME               Switch a pattern on, by its English name or type:name; repeatable
  --disable NAME              Switch a pattern off, by its English name or type:name; repeatable
  --language CODE             The dictionary join-words and split-words read: fr_FR, en_US, en...
  --max-length N              Longest line of line-break, in characters; no default, Gaupol's is in ems
  --max-lines N               Most lines of line-break: 3, as Gaupol's
  --skip-length N|off         Leave alone a subtitle whose longest line is within this, or off: --max-length
  --skip-lines N|off          Leave alone a subtitle whose line count is within this, or off: --max-lines
  --keep-blank-subtitles      Leave the subtitles the correction empties, empty, instead of removing them
  --remove-blank-subtitles    Remove the subtitles the correction empties, whatever the settings file says
  --range N-M|N-              Act only on subtitles N to M, or N to the end
  --document main|translation The document to change: the main one, or the translation given by -t
  -t,--translation-file FILE  Translation file to lay over the subtitle file, for a single input
  --align-method position|number
                              How the lines of the translation find their subtitles
  --output TEXT               File to write, for a single input
  --output-dir TEXT           Directory to write into
  --in-place                  Write back over the inputs
  --dry-run                   Work out and say what would be written, and write nothing
```

## O que se escolle, e o que non

**Non hai nada marcado de antemán.** A xanela abre as súas caixas tal como as deixou o usuario;
unha liña de ordes toma as súas tarefas, o seu código, a súa lonxitude de liña e a súa lingua da
súa propia liña, nunca do ficheiro de configuración: non se len nin as tarefas marcadas un día na
xanela, nin as caixas de padróns activadas alí (`correction.activations`), nin a lista de
substitucións do corrector ortográfico, e non se escribe nada por ese lado.

**Catro axustes veñen do ficheiro de configuración, de maneira predeterminada** — os valores que
`Correct Texts…` lembra, ou que escribiu [`config set`](config.md). **Unha opción na liña gaña
sempre**, despois o ficheiro, despois o valor predeterminado integrado; véxase
[O ficheiro de configuración](invocation.md#o-ficheiro-de-configuración) para saber onde está,
`--config` e `--no-config`.

| Clave | O que achega | Opción que a substitúe |
| :-- | :--------------- | :----------------------- |
| `correction.human`, `correction.ocr` | as clases de erros comúns aplicadas | `--classes` |
| `correction.remove-blank` | se se eliminan os subtítulos que a corrección baleira | `--keep-blank-subtitles`, `--remove-blank-subtitles` |
| `correction.line-break.max-lines` | o número máximo de liñas de `line-break` | `--max-lines` |

- **Cada un dos catro ten unha opción que o desfai**, diga o que diga o ficheiro:
  `--classes human,ocr` aplica as dúas clases, `--remove-blank-subtitles` elimina os subtítulos
  baleirados, `--max-lines 3` devolve as liñas de Gaupol. `--keep-blank-subtitles` e
  `--remove-blank-subtitles` xuntas son un erro de uso.
- **As dúas clases desactivadas no ficheiro ignóranse, e dise** no nivel 1 —
  `settings: correction.human and correction.ocr are both false, ignored` —: `--classes` non o podería escribir, e
  deixaría a tarefa common-errors sen nada que facer. Aplícanse as dúas clases no seu lugar.
- **Un número de liñas fóra de 1 a 1000 non se le en absoluto do ficheiro**: o lector do
  ficheiro rexéitao — `settings: correction.line-break.max-lines: cannot read 'N', default kept`,
  no nivel 1 — e úsase o 3 integrado.

Os exemplos desta páxina execútanse con `--no-config`, para que digan o que fan os valores
predeterminados integrados, sexa cal sexa a configuración da máquina que os executa.

| Opción | Obrigatorio | Valores | Predeterminado |
| :----- | :----- | :------ | :----- |
| `<file>...` | si | un ou varios camiños | — |
| `--tasks` | **si** | unha lista separada por comas: `mentions`, `join-words`, `split-words`, `common-errors`, `capitalization`, `line-break` | ningunha tarefa |
| `--code` | **si** cunha tarefa de padróns, e **non aplicable** no caso contrario | `Script[-language[-COUNTRY]]`: `Zyyy`, `Latn`, `Latn-en`, `Latn-en-US` | — |
| `--language` | **si** con `join-words` ou `split-words`, e **non aplicable** no caso contrario | un código de lingua: `fr`, `fr_FR`, `en_US` | — |
| `--classes` | non | `human`, `ocr` ou `human,ocr` | as dúas — ou o que diga o ficheiro de configuración |
| `--enable`, `--disable` | non, repetibles | o nome inglés dun padrón, ou `type:name` | os ficheiros `.conf` distribuídos |
| `--max-length` | **si** con `line-break` | un número de **caracteres** maior que cero | **ningún** — véxase máis abaixo |
| `--max-lines` | non | un número enteiro de liñas, maior que cero | 3 — ou o que diga o ficheiro de configuración |
| `--skip-length`, `--skip-lines` | non | un número, ou `off` | os mesmos límites que `--max-length` e `--max-lines` |
| `--keep-blank-subtitles` | non | unha marca: os subtítulos baleirados quedan, baleiros | os subtítulos baleirados elimínanse — salvo que o ficheiro de configuración diga outra cousa |
| `--remove-blank-subtitles` | non | unha marca: os subtítulos baleirados elimínanse, diga o que diga o ficheiro de configuración; rexeitada con `--keep-blank-subtitles` | desactivada |
| `--range` | non | `N-M`, ou `N-` ata o final — véxase [`adjust`](adjust.md#--range) | o ficheiro enteiro |
| `--document` | non | `main` ou `translation` — véxase [Unha tradución](invocation.md#unha-tradución) | `main` |
| `-t`, `--translation-file` | con `--document translation` | un ficheiro de tradución, para **unha soa** entrada | — |
| `--align-method` | non, e só con `-t` | `position` ou `number` | `position` |
| `--recursive`, `-r` | non | unha marca | desactivada — véxase [Procesar unha árbore](lots.md) |
| `--output` / `--output-dir` / `--in-place` | **unha das tres**, agás con `--dry-run` | véxase [Invocación](invocation.md#o-destino) | — |
| `--dry-run` | non | unha marca: calcula e di, non escribe nada — véxase [Ver antes de escribir](invocation.md#ver-antes-de-escribir) | desactivada |

### As tarefas

**A orde da lista non cambia nada**: as tarefas aplícanse na orde de Gaupol — as mencións,
despois a unión e a división de palabras, despois os erros comúns, despois as maiúsculas, despois o salto de liña —, cada unha sobre o
texto que deixou a anterior. Un subtítulo que as mencións baleiraron non participa no que
segue.

| Tarefa | O que fai |
| :---- | :-------------- |
| `mentions` | elimina os sons entre corchetes, e entre parénteses cando `Sound in parentheses` está activado (o varrido de [`hearing-impaired`](hearing-impaired.md)), e despois os padróns de letras de cancións e de interlocutores |
| `join-words` | volve unir unha palabra cortada en dúas, a partir do dicionario de `--language` |
| `split-words` | separa dúas palabras pegadas, a partir do mesmo dicionario |
| `common-errors` | corrixe os erros comúns: espazos, puntuación, ligaduras, erros de OCR |
| `capitalization` | pon a maiúscula onde a quere a lingua |
| `line-break` | volve cortar as liñas dun subtítulo para que ningunha supere `--max-length`, sopesando os padróns de salto do código |

### O código de padróns

**`--code` nomea a cascada**: `Zyyy` para calquera escritura, despois a escritura, a lingua e o
país — `Latn-en-US` le os padróns de `Zyyy`, de `Latn`, de `Latn-en` e de `Latn-en-US`.
**É obrigatorio**, porque unha liña de ordes non ten lingua ambiente: un valor predeterminado
universal aplicaría *menos* do que se cre, sen dicilo. Vale para todas as tarefas da mesma
execución; unha tarefa que queira un código propio é unha segunda execución. Un código mal
escrito é un erro de uso:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks common-errors --code francais --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks common-errors --code francais --output b.srt a.srt; echo "code=$?"
--code: "francais" is not a pattern code: expected Script[-language[-COUNTRY]], like Zyyy, Latn or Latn-en-US
code=1
```

Os padróns que o usuario pon en `$XDG_DATA_HOME/subedit/patterns` (ou
`~/.local/share/subedit/patterns`) **engádense aos padróns distribuídos**, como na xanela, e un
padrón distribuído que non se pode ler indícase no nivel 1: `patterns: <file>, line N (<reason>)`.

### Unir e dividir palabras, e o dicionario

**Estas dúas tarefas non len ningún padrón**: consultan o **dicionario do sistema, mediante
Enchant** — unha dependencia dos paquetes (`libenchant-2-2` en Debian e Ubuntu, `enchant2` en
Fedora), que `subedit-cli` comparte coa xanela. Non teñen, polo tanto, nin código nin nome que
fixar: `--code` non ten sentido sen unha tarefa de padróns, nin `--language` sen unha destas
dúas — unha indicada sen a outra é un erro de uso, código `1`.

- **`join-words`** une unha palabra que o dicionario non coñece á que a precede ou a segue,
  **cando un só sentido dá unha palabra coñecida**: `hel lo` convértese en `hello`; dous sentidos
  posibles, ou ningún, non deciden nada.
- **`split-words`** separa unha palabra que o dicionario non coñece **cando exactamente unha das
  súas suxestións é esa palabra cun espazo**: `hellothere` convértese en `hello there`. Unha palabra
  cunha maiúscula inicial seguida de minúsculas déixase tal como está: a miúdo é un nome.

**Sen dicionario para a lingua, a execución rexéitase**, código `1`, **antes de ler ningún
ficheiro** e cos termos da xanela, que desactiva a función e dio — aquí a liña de ordes rexeita e
dio, en lugar de calar:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nhel lo\n\n' > a.srt; subedit-cli --no-config correct --tasks join-words --language qq_QQ --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nhel lo\n\n' > a.srt; subedit-cli --no-config correct --tasks join-words --language qq_QQ --output b.srt a.srt; echo "code=$?"
no dictionary for qq_QQ
code=1
```

**A lista de substitucións da xanela non se le nin se escribe.** *Énchese* coas escollas do
usuario na corrección ortográfica; unha liña de ordes non ten esa escolla, e ler a lista faría
que un script dependese do que o seu autor puxo nela algunha vez.

### Salto de liña

**Mídese en caracteres, e esa é a diferenza coa xanela.** A xanela mide en *ems* — un ancho
calibrado sobre o tipo de letra, a 0,55 em por letra —, e o seu 24 predeterminado é un valor en
ems. Unha liña de ordes non ten tipo de letra: conta caracteres, os do texto. **Copiar o 24 como
caracteres cortaría case todo, sen dicilo**; non hai, polo tanto, **ningunha lonxitude
predeterminada**, e a opción `--max-length` é obrigatoria con `line-break`:

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nUn texte\n\n' > a.srt; subedit-cli --no-config correct --tasks line-break --code Latn-en --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nUn texte\n\n' > a.srt; subedit-cli --no-config correct --tasks line-break --code Latn-en --output b.srt a.srt; echo "code=$?"
--max-length is required by the task line-break: Gaupol's 24 is a width in ems, and has no value in characters
code=1
```

**`--max-lines` vale 3**, o valor de Gaupol, que a unidade non cambia; o salto de liña toma unha
máis cando tres non bastan. **A omisión** — un subtítulo que xa está dentro dos límites déixase
tal como está — toma de maneira predeterminada **os mesmos límites** (`--skip-length` =
`--max-length`, `--skip-lines` = `--max-lines`), como fai Gaupol; `--skip-length off` e
`--skip-lines off` desactívana, e o texto vólvese ler entón como un só e cortar de novo:

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nWhen the rain stopped, we walked home under the dark sky of the old town.\n\n' > a.srt; subedit-cli --no-config correct --tasks line-break --code Latn-en --max-length 24 --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nWhen the rain stopped, we walked home under the dark sky of the old town.\n\n' > a.srt; subedit-cli --no-config correct --tasks line-break --code Latn-en --max-length 24 --output b.srt a.srt; cat b.srt
a.srt: Edited 1 and removed 0 subtitles -> b.srt
1
00:00:01,000 --> 00:00:03,000
When the rain
stopped, we walked
home under the dark
sky of the old town.
```

Os padróns de salto son os do código: `Latn` non corta entre un número e a súa unidade nin
despois dun trazo de diálogo, `Latn-en` non corta despois dun artigo, unha preposición, un
tratamento ou un determinante posesivo. **Con `Zyyy`, non existe ningún padrón de salto**: as
liñas só se equilibran. **Esta tarefa nunca se rexeita por falta dun padrón activo**: os seus
padróns sopesan onde cortar, e o corte prodúcese sen eles. As catro opcións só teñen sentido con
`line-break`; indicadas sen ela, son un erro de uso.

### As clases, e a activación por nome

**`--classes` filtra os erros comúns de forma definitiva**: desmarcar `ocr` retira os seus
padróns da aplicación — a ligadura `ﬁ` queda entón `ﬁ`. Os outros tres tipos de padróns non
levan clase.

**`--enable` e `--disable` fixan un padrón polo seu nome inglés**, sendo o nome a clave
(ADR 0037); un nome pode designar varios rexistros, que comparten entón unha soa caixa. É
necesario, e é o que deixa de lado os valores predeterminados da xanela: **os ficheiros `.conf`
distribuídos desactivan os padróns de mencións**, de maneira que `--tasks mentions` só non
aplicaría nada. Os dous padróns «Sound in brackets» e «Sound in parentheses» rexen cada un o seu
delimitador — os corchetes, as parénteses — e fíxanse coas mesmas opcións: activar un só non
elimina o outro.

Tres cousas son **erros de uso**, código `1`, indicados antes de ler ningún ficheiro:

| O que o provoca | Mensaxe |
| :------------------ | :------ |
| un nome que non designa ningún padrón das tarefas indicadas, co código indicado | `--enable: "<name>" names no pattern of the tasks given, under the code <code>` |
| un nome que designa padróns de dous tipos — escríbese entón `type:name`, sendo o tipo `common-error`, `capitalization`, `hearing-impaired` ou `line-break` | `--enable: "<name>" names patterns of several types: write type:name, …` |
| un nome indicado a `--enable` e a `--disable` | `"<name>" is given to --enable and to --disable: choose one` |

**Un erro de escritura faise notar**: a xanela ignora un axuste caducado porque un axuste pode
sobrevivir a un padrón que desapareceu; unha invocación non ten superviventes.

**Unha tarefa sen ningún padrón activo rexéitase**, código `1` — «nada que facer» é unha resposta
que se di, non un éxito silencioso:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\n[Bruit] Bonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks mentions --code Latn --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n[Bruit] Bonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks mentions --code Latn --output b.srt a.srt; echo "code=$?"
mentions: no pattern is active under the code Latn; switch one on with --enable
code=1
```

O mesmo ficheiro, co varrido dos corchetes activado:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\n[Bruit] Bonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks mentions --code Latn --enable "Sound in brackets" --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n[Bruit] Bonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks mentions --code Latn --enable "Sound in brackets" --output b.srt a.srt; cat b.srt
a.srt: Edited 1 and removed 0 subtitles -> b.srt
1
00:00:01,000 --> 00:00:02,000
Bonjour
```

## O que se escribe, e o que se di

O formato, a codificación e os finais de liña do ficheiro lido **consérvanse**. **Os subtítulos
que a corrección baleira elimínanse**, como coa caixa predeterminada da xanela;
`--keep-blank-subtitles` déixaos baleiros, e `--remove-blank-subtitles` elimínaos mesmo cando o
ficheiro de configuración os conserva. **Unha tradución baleirada queda baleirada**, e conserva o
seu subtítulo.

**O reconto é o da xanela** — «Edited N and removed M subtitles»: textos modificados e subtítulos
eliminados, **nunca coincidencias**. Un ficheiro onde non cambia nada escríbese igualmente: un
destino indicado é un destino escrito.

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\neﬀet  de ﬁn\n\n2\n00:00:03,000 --> 00:00:04,000\nPropre.\n\n' > a.srt; subedit-cli --no-config correct --tasks common-errors --code Latn --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\neﬀet  de ﬁn\n\n2\n00:00:03,000 --> 00:00:04,000\nPropre.\n\n' > a.srt; subedit-cli --no-config correct --tasks common-errors --code Latn --output b.srt a.srt; cat b.srt
a.srt: Edited 1 and removed 0 subtitles -> b.srt
1
00:00:01,000 --> 00:00:02,000
effet de fin

2
00:00:03,000 --> 00:00:04,000
Propre.
```

### Un padrón que non se pode aplicar

**Un padrón que non se pode ler, que non se pode traducir ou que non remata noméase, co subtítulo
no que desistiu**, e aquí é onde a liña de ordes di máis ca a xanela: a páxina de confirmación
nomea o padrón unha soa vez, sen o texto.

| Motivo | O que significa |
| :----- | :------------------- |
| `cannot be translated` | a súa expresión usa o que a tradución a ICU rexeita |
| `will not compile` | o motor rexeitou a súa expresión |
| `has an invalid replacement` | a súa substitución non se le como a le Python |
| `timed out` | o motor desistiu con ese texto |
| `never settled` | `Repeat` seguía cambiando o texto despois de cen pasadas |
| `grew the text too long` | o texto superou o que pode conter un subtítulo |

No nivel 1, na saída de erro: 
`<path>: pattern "<name>" (<reason>) was not applied to subtitle N` — sen o final cando o padrón se deixa de lado para todos os textos. **O número é o
do ficheiro**, sexan cales sexan as tarefas que eliminaron subtítulos antes. En JSON, é un
aviso `pattern-failed` do ficheiro, cuxo `detail` leva a frase. **Un padrón abandonado non é un
ficheiro fallido**: o ficheiro escríbese, e o código de saída é `0`.

## Saída

**Saída estándar** — nada, xa que o resultado é o ficheiro escrito. **Con `--dry-run`, son os
cambios propostos**, un bloque por subtítulo: o seu número, o texto antes, o texto despois — e
`(removed)` sen liña `+` para un subtítulo que a corrección eliminaría; con
`--keep-blank-subtitles`, queda, baleiro. Véxase [Ver antes de escribir](invocation.md#ver-antes-de-escribir).

| Nivel | O que se engade na saída de erro |
| :----- | :------------------------------------- |
| 1 | `<path>: Edited N and removed M subtitles -> <destination>`, os padróns que non se puideron aplicar, os ficheiros de padróns que non se poden ler — e, antes, o ficheiro de configuración lido — `settings: read <path>` — e o que se ignorou del |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` |
| 3 | `<path>: N bytes read, M written`, e despois cada diagnóstico de lectura |

En JSON, `counts` leva `corrected` (os textos modificados) e `removed` (os subtítulos
eliminados), e `changes` os subtítulos tocados. Con `-t`, a clave `alignment` di como se colocou
a tradución.
`settings` nomea o ficheiro de configuración que se leu, ou é `null` cando non se leu ningún.

## Códigos de saída

Os da ferramenta: `0` se se escribiron todos os ficheiros, `2` se non se escribiu ningún, `3` se
só se escribiron algúns, `1` nun erro de uso.

## Erros

| O que o provoca | O que se escribe, na saída de erro |
| :------------------ | :--------------------------------------- |
| un ficheiro indicado con `--config` que non existe — o propio ficheiro do usuario, se falta, dá os valores predeterminados | `settings file not found: <FILE>` |
| falta `--tasks` | `--tasks is required`, seguido dunha indicación cara a `--help` |
| unha tarefa descoñecida | `--tasks: "<name>" is not a task: expected mentions, join-words, split-words, common-errors, capitalization or line-break` |
| falta `--code` | `--code is required by the tasks that read patterns: <tasks>` |
| `--code` sen tarefa de padróns | `--code is for the tasks that read patterns, and none was asked for` |
| falta `--language` con `join-words` ou `split-words` | `--language is required by the tasks that check words: <tasks>` |
| un valor de `--language` que non é un código de lingua | `--language: "<value>" is not a language: expected a locale code, like fr, en_US or sr@Latn` |
| `--language` sen ningunha destas dúas tarefas | `--language is for the tasks join-words and split-words, which were not asked for` |
| unha lingua sen dicionario | `no dictionary for <language>` |
| un código mal escrito | `--code: "<code>" is not a pattern code: expected Script[-language[-COUNTRY]], like Zyyy, Latn or Latn-en-US` |
| falta `--max-length` con `line-break` | `--max-length is required by the task line-break: Gaupol's 24 is a width in ems, and has no value in characters` |
| un valor de `--max-length` que non é un número maior que cero | `--max-length: "<value>" is not a length: expected a number of characters greater than zero` |
| valores de `--max-lines`, `--skip-lines` que non son un número enteiro de liñas | `--max-lines: "<value>" is not a number of lines: expected a whole number greater than zero` |
| `--skip-length` ou `--skip-lines` ilexibles | `--skip-length: "<value>" is not a bound: expected a number greater than zero, or off` |
| unha das catro opcións sen `line-break` | `--max-length is for the task line-break, which was not asked for` |
| unha clase descoñecida | `--classes: "<name>" is not a class: expected human, ocr or human,ocr` |
| `--keep-blank-subtitles` con `--remove-blank-subtitles` | `--keep-blank-subtitles and --remove-blank-subtitles say opposite things; give one of them` |
| un nome de padrón erróneo | véxase a táboa de arriba |
| unha tarefa sen padrón activo | `<task>: no pattern is active under the code <code>; switch one on with --enable` |
| `--range`, `-t`, `--document`, o destino | os de [Invocación](invocation.md#erros) e de [Unha tradución](invocation.md#unha-tradución) |
