# Invocación

<!-- machine-translated -->
> Esta páxina é unha tradución automática e aínda non a revisou ningún lector da lingua.

```
subedit-gui [--version] [<file>]
```

| Argumento | Obrigatorio | Valor | Predeterminado |
| :------- | :------- | :---- | :------ |
| `<file>` | non | a ruta dun ficheiro de subtítulos que abrir | ningún — a xanela ábrese baleira |
| `--version` | non | — | — |

**Un só ficheiro.** Os argumentos seguintes ignóranse: a xanela só contén
un proxecto de cada vez.

## `--version`

Escribe na saída estándar o bloque que escribe a opción `--version` da liña de ordes, e
remata de inmediato, sen abrir ningunha xanela. Código de saída `0`.

**`subedit <version>` só na primeira liña**, despois unha liña por cada biblioteca coa que
funciona a xanela, coa versión atopada na execución e, entre parénteses, a
versión coa que se compilou; despois as linguas da interface instaladas, polo seu código. Nunha
máquina con Qt 6.4.2:

```console
$ subedit-gui --version
subedit <version>
  ICU      74.2 (built with 74.2)
  Enchant  2.3.3 (built with 2.3.3)
  Qt       6.4.2 (built with 6.4.2)
  libmpv   2.2 (built with 2.2)
  Languages: en, fr
```

Os números non se copian aquí: son os da máquina, e este manual non ten
ningún bloque xerado que manteña actualizados os do propio programa. As bibliotecas son
as da liña de ordes, ICU e Enchant, e as dúas que só enlaza a xanela — Qt, e
libmpv, o reprodutor, indicado pola versión da súa API cliente, a que anuncia
`mpv.pc`. O resto descríbese no
[manual da liña de ordes](../subedit-cli/invocation.md#--version). As palabras
`built with` e `Languages` seguen a lingua da xanela; nada máis o fai.

`Axuda ▸ Acerca de subedit` mostra as mesmas bibliotecas, da mesma lista, nunha táboa —
o nome, a versión cargada, a versión coa que se compilou, unha fila para cada unha — e despois as
linguas debaixo; os seus encabezados seguen a lingua da xanela.

## Abrir un ficheiro

```console
$ subedit-gui film.srt
```

Os formatos que se len son **os nove** que lista o filtro de apertura — véxase
[Abrir e gardar](files.md#abrir). O formato recoñécese polo contido e
non pola extensión, e **tamén a codificación**: unha BOM
se o ficheiro a leva, se non a codificación que suxiren os seus bytes.

## Cando falla a apertura

Un ficheiro que falta, un ilexible, un cuxos bytes non se descodifican en ningunha
codificación, ou un cuxo contido non recoñece ningún formato: **a xanela ábrese
igualmente, baleira**, e o fallo escríbese na saída de erro.

**A mensaxe indica a causa**, coas mesmas palabras que a liña de ordes:

| Mensaxe | O que pasou |
| :------ | :------------ |
| `does not exist` | a ruta non designa ningún ficheiro |
| `cannot be opened: permission denied` | o sistema négase a abrilo |
| `cannot be read` | o sistema negouse por outro motivo |
| `cannot be decoded in the chosen encoding` | os bytes non se len na codificación escollida |
| `is in no format this tool knows` | ningún formato recoñece o contido |
| `holds nothing recognisable as a subtitle` | o formato recoñécese, pero nada do que contén é un subtítulo |

```console
$ subedit-gui notes.txt
subedit-gui: notes.txt: is in no format this tool knows
```

Abrir baleira en vez de negarse a iniciar: a ferramenta segue servindo para
algo, e o fallo dun argumento non é motivo para non arrancar.

**Un ficheiro mal formado pero lexible ábrese**, e o que a lectura detectou
polo camiño pódese ler nun botón da barra de estado — véxase
[os diagnósticos dunha lectura](files.md#os-diagnósticos-dunha-lectura).

## Códigos de saída

| Código | Cando |
| :--- | :--- |
| `0` | a xanela abriuse e despois pechouse normalmente, ou `--version` respondeu |
| `2` | un erro inesperado antes da apertura — falta Qt, pantalla non dispoñible |

O fallo ao abrir un ficheiro **non cambia o código de saída**: a
xanela abriuse, e iso é o que indica o código.

## A páxina de manual

Unha instalación deposita `subedit-gui(1)` en `share/man/man1`:

```console
$ man subedit-gui
```

Dá a forma de chamada, `--version`, as mensaxes de fallo na apertura, os
códigos de saída, as dúas variables de contorno e o ficheiro de configuración.
**Non describe a xanela** — faino este manual, e a páxina
remite aquí.

## A plataforma Qt

`QT_QPA_PLATFORM` escolle sobre que funciona a xanela. **Se a variable está
definida, `subedit-gui` respéctaa sen discusión.**

Se non o está, deixa que Qt escolla: o reprodutor integrado debuxa a imaxe na
propia xanela, e non lle pide nada á plataforma — véxase
[O reprodutor](player.md).

`QT_QPA_PLATFORM=offscreen` fai funcionar a xanela onde non hai
servidor gráfico — que é o que fan a integración continua e as probas.

## Lingua

A xanela fala a lingua do seu contorno, **escollida unha soa vez, ao iniciarse**: cambiar as
variables dunha xanela en funcionamento non cambia nada ata o seguinte inicio. As variables e as regras
son as da liña de ordes — `LANGUAGE`, `LC_ALL`, `LC_MESSAGES`, `LANG`, `LC_ALL=C` para o
inglés, o inglés **sen dicir nada** cando a lingua pedida non está instalada — descritas no
[manual da liña de ordes](../subedit-cli/invocation.md#lingua). Os catálogos son os mesmos
ficheiros, instalados en `<prefix>/share/subedit/locale`.

**Unha lingua escollida en `Editar ▸ Preferencias…` ten prioridade**, e o contorno só se le
cando vale `System default`, que é o caso mentres non se escolla ningunha: unha lingua instalada escollida alí
impóñese a todas as variables, `LC_ALL=C` incluída. A escolla, tamén, aplícase no seguinte inicio. Véxase
[A lingua da interface](preferences.md#a-lingua-da-interface).

| Que | Segue a lingua |
| :--- | :------------------- |
| os menús e as súas entradas, coas súas letras subliñadas; a barra de ferramentas; as axudas emerxentes | si |
| as columnas da táboa e de `Ver ▸ Columnas`, as lapelas, a liña de estado, a barra de reprodución | si |
| os botóns estándar que debuxa o propio Qt (`OK`, `Cancel`) | si, cando está instalado o catálogo propio de Qt para esa lingua (`qtbase_<language>.qm`); en inglés se non |
| os diálogos, as páxinas de `Correct Texts…`, os títulos e os filtros dos diálogos de ficheiros, as caixas de mensaxe e o que din | si |
| as liñas que a xanela escribe na súa saída de erro, `subedit-gui: …` | si, agás o nome do programa |
| os nomes dos padróns de corrección en `Correct Texts…`, e as súas descricións nas axudas emerxentes | si, para os padróns distribuídos co programa; un padrón propio móstrase tal como o escribe o seu ficheiro |
| os nomes das codificacións, dos finais de liña e das linguas de ortografía (`UTF-8`, `CRLF`, `en_US`) | nunca: son nomes, non palabras |
| os atallos de teclado | nunca: `Ctrl+S` é `Ctrl+S` en todas as linguas |
| unha mensaxe que o catálogo non traduce | móstrase en inglés |

Esta versión instala `ca`, `cs`, `de`, `es`, `fi`, `fr`, `gl`, `he`, `hu`, `ie`, `is`, `it_IT` e `nl` — catalán, checo, alemán, español,
finés, francés, galego, hebreo, húngaro, interlingue, islandés, italiano e neerlandés — traducidos por completo. O francés foi revisado por un lector da
lingua; o catalán, o checo, o alemán, o español, o finés, o galego, o hebreo, o húngaro, o interlingue, o islandés, o italiano e o neerlandés son traducións automáticas aínda non
revisadas por ningún lector da lingua.
<!-- machine-translated: ca -->
<!-- machine-translated: cs -->
<!-- machine-translated: de -->
<!-- machine-translated: es -->
<!-- machine-translated: fi -->
<!-- machine-translated: gl -->
<!-- machine-translated: he -->
<!-- machine-translated: hu -->
<!-- machine-translated: ie -->
<!-- machine-translated: is -->
<!-- machine-translated: it_IT -->
<!-- machine-translated: nl -->
<!-- machine-translated: pl -->
<!-- machine-translated: pt -->
<!-- machine-translated: pt_BR -->
Nun escritorio en francés a barra de menús mostra
`&Fichier`, `É&dition`, `&Affichage`, `&Vidéo`, `&Outils`, `&Projets`, `Aid&e`. As capturas de pantalla deste
manual fanse en galego.
