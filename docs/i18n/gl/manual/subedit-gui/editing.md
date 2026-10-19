# Editar unha cela

<!-- machine-translated -->
> Esta páxina é unha tradución automática e aínda non a revisou ningún lector da lingua.

Catro columnas edítanse no seu sitio: `Start`, `End`, `Duration` e `Text` — cinco
coa columna `Translation`, cando o proxecto ten unha tradución. `#` non se pode
editar — o número é a posición da liña na orde, non un dato do ficheiro.

**Abrir o editor dunha cela:** facer dobre clic nela, ou seleccionala
e premer `F2`.

![O texto do terceiro subtítulo aberto para edición, paleta clara: o campo
é de varias liñas e mostra as dúas liñas tal como se
escribirán.](captures/edition.png)

![A mesma cela aberta coa paleta escura.](captures/edition-sombre.png)

## O texto

**A tradución edítase como o texto**, co mesmo campo de varias liñas e as
mesmas regras; só cambia a cela, e o texto principal queda tal como
estaba. Unha validación que non cambia nada non fai nada, tanto para un como para o
outro — véxase [máis abaixo](#unha-validación-que-non-cambia-nada-non-fai-nada).

O editor é un campo **de varias liñas**, e ábrese coa altura do subtítulo:
un subtítulo de dúas liñas escríbese nel tal como se escribirá, coas súas dúas liñas
lexibles e sen barra de desprazamento. Un `Shift+Enter` que engade unha agranda
o editor; a fila da táboa axústase ao validar.

| Tecla | Efecto |
| :-- | :----- |
| `Enter` | valida e pecha o editor |
| `Shift+Enter` | insire un salto de liña no texto |
| `Esc` | pecha o editor; a cela conserva o que tiña |
| `Tab` | valida e pasa á cela seguinte |

Facer clic noutro lugar tamén valida: perder o foco equivale a un `Enter`.

**Once xestos sen diálogo validan igualmente** antes de calcular o seu obxectivo:
`Italic`, `Maiúsculas ▸ …`, `Dialogue`, `Merge Subtitles`, `Split Subtitle`,
`Cut Texts`, `Paste Texts`, `Copy Texts`, `Remove Subtitles`, `Undo` e `Redo`.
Lanzados desde o menú, a barra de ferramentas ou un atallo da xanela,
toman o texto tal como o editor o acaba de validar, como despois dun clic
noutro lugar. `Ctrl+X`, `Ctrl+C` e `Ctrl+V` seguen, con todo, co editor mentres
estea aberto — véxase [Cortar, copiar e pegar](clipboard.md): é entón o
menú o que lanza `Cut Texts`, `Copy Texts` e `Paste Texts`.

`Undo` e `Redo` len o historial e non o documento: véxase
[Desfacer e refacer](undo.md). Un xesto que abre un diálogo non
necesita esta regra, xa que o diálogo toma o foco ao abrirse, o que
xa valida a entrada.

### As etiquetas son as do ficheiro

O texto mostrado e escrito é **o do ficheiro, etiquetas incluídas**: un
`<i>…</i>` dun SubRip escríbese así, e un `{\i1}…{\i0}` dun Advanced SSA
tamén. Nada se traduce para mostralo — a táboa de
[`Italic`](operations.md#italic) di o que escribe cada formato.

Para a cursiva, polo tanto, non hai nada que saber: `Ctrl+I` pon a etiqueta correcta
para o documento aberto, e un formato que non leva ningunha — TMPlayer, LRC —
desactiva a entrada en lugar de deixar buscar. A negra e o subliñado
escríbense, como en Gaupol.

**E as etiquetas non seguen as operacións sobre o texto**: poñer un subtítulo
en minúsculas non converte o seu `<I>` en `<i>`, e un trazo de diálogo
ponse diante do texto e non diante dunha etiqueta de apertura. Véxase
[`Case` e `Dialogue`](operations.md#case-e-dialogue).

## A lonxitude das liñas

![O editor dunha cela de dúas liñas, paleta clara: unha marxe á dereita
mostra a lonxitude de cada liña, aliñada con ela.](captures/longueurs-edition.png)

![O mesmo editor coa paleta escura.](captures/longueurs-edition-sombre.png)

**O editor de texto mostra, nunha marxe á dereita, a lonxitude de cada
liña**, aliñada coa primeira fila desa liña. A marxe é tan ancha
como o número máis grande, actualízase **mentres se escribe**, e segue o desprazamento.
Un texto baleiro non mostra nada. As lonxitudes son as da táboa — sen as
etiquetas, redondeadas cara abaixo, na unidade fixada — e retíranse coa
caixa de verificación `Show line lengths in the editor` de `Editar ▸ Preferencias…`. Véxase
[Lonxitudes das liñas](preferences.md#lonxitudes-das-liñas).

## O inicio e o final

O editor é un campo dunha liña, **limitado á forma dunha marca de tempo**:
rexeita mentres se escribe o que nunca podería selo.

As formas aceptadas son as que xa acepta a lectura dun ficheiro:

| Elemento | O que pasa |
| :------ | :---------- |
| campos | `MM:SS` ou `HH:MM:SS`, un ou dous díxitos cada un |
| decimais | de un a tres, despois dunha coma ou dun punto, ou ningún |
| signo | un `-` inicial, para unha posición anterior ao comezo do vídeo |
| límites | minutos e segundos por debaixo de 60 |

`00:01:02,500`, `1:02.5` e `-0:01,000` son polo tanto tres entradas válidas.

**Unha entrada que a lectura rexeita deixa a cela sen cambios.** É o caso
de `00:70:00,000`, cuxa forma é correcta pero cuxos minutos están fóra dos
límites: o editor péchase e nada se moveu. Non se inventa ningunha posición
— errar nunha posición é silencioso, nada na pantalla distingue un
subtítulo mal temporizado dun ben temporizado.

Cambiar un inicio ou un final actualiza de inmediato a columna `Duration`.

## A duración

**Escribir unha duración move o final, e nunca o inicio**: o final pasa a ser
`Start + Duration`. Mover o inicio movería o subtítulo enteiro, que
é a tarefa de [`Shift Positions…`](operations.md), e na columna `Start` xa se
pode escribir.

A duración escríbese **coa forma dunha marca de tempo**, a que mostra a
columna, e aplícanse as formas da táboa anterior — cunha diferenza:

| Elemento | Inicio, final | Duración |
| :------ | :--------- | :------- |
| forma | a dunha marca de tempo | a mesma |
| signo `-` inicial | aceptado | **rexeitado mentres se escribe** |

`0:02.5` e `00:00:02,500` son polo tanto dúas entradas válidas dunha duración de dous
segundos e medio; `-0:01,000` non se pode escribir. Unha duración nula acéptase:
o subtítulo desaparece entón no instante en que aparece.

**Un final que pasa por riba do subtítulo seguinte está permitido.** O
solapamento tínguese na táboa, coa súa axuda emerxente — véxase
[A táboa](table.md#as-anomalías) — un aviso, non un rexeitamento: pode ser o que
se quería. A [lista de diagnósticos](files.md#os-diagnósticos-dunha-lectura)
tamén o leva, e retírao cando se corrixiu.

**Un final escrito máis alá do final da película non se indica**, a diferenza
dunha operación do menú `Tools` que o levaría alí — véxase
[As operacións](operations.md#máis-alá-do-final-da-película). Unha cela xa mostra
o valor que se acaba de escribir.

Unha entrada ilexible deixa a cela sen cambios, como para unha posición, e
unha duración idéntica á mostrada non entra no historial.

## Unha validación que non cambia nada non fai nada

Abra unha cela, non escriba nada, prema `Enter`: non pasa nada.
Non se rexistra ningunha modificación, e o ficheiro segue considerándose idéntico ao
do disco.

A comparación faise sobre a **posición**, non sobre a cadea: escribir `0:01.0`
nunha cela que mostra `00:00:01,000` designa o mesmo instante, e
polo tanto tampouco cambia nada.

## Que pasa cunha cela editada

**Pódese desfacer** — unha cela, unha entrada. Véxase
[Desfacer e refacer](undo.md).

**Gárdase**, e pechar a xanela sen telo feito pide
confirmación. Véxase [Abrir e gardar](files.md).
