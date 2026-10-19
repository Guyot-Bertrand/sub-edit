# Temporizar desde o vídeo

<!-- machine-translated -->
> Esta páxina é unha tradución automática e aínda non a revisou ningún lector da lingua.

*Suponse que a película está aberta e que se coñece o reprodutor: véxase [O reprodutor](player.md).*

Mírase a película, detense no fotograma onde debe comezar unha liña, e **esa posición pasa a ser
o inicio do subtítulo**. Cinco xestos, no menú **Vídeo**, cos nomes que lles dá Gaupol. Apóianse
no que xa existe: fixar unha posición é **a mesma orde que escribir o valor na
cela**, e inserir é **o mesmo que `Editar ▸ Inserir subtítulos…`**.

| Orde | Atallo | O que fai |
| :------ | :------- | :----------- |
| Vídeo ▸ Fixar o inicio desde a posición do vídeo | `Alt+U` | fixa o **inicio** do subtítulo seleccionado na posición da película |
| Vídeo ▸ Fixar o final desde a posición do vídeo | `Alt+K` | fixa o seu **final** na posición da película |
| Vídeo ▸ Inserir un subtítulo na posición do vídeo | `Alt+J` | insire un subtítulo que **comeza** na posición, e selecciónao |
| Vídeo ▸ Seleccionar o anterior desde a posición do vídeo | `Alt+Up` | selecciona o último subtítulo que **comeza antes** da posición |
| Vídeo ▸ Seleccionar o seguinte desde a posición do vídeo | `Alt+Down` | selecciona o primeiro que **comeza despois** da posición |

**Todos están desactivados sen ningún vídeo aberto**, como os xestos do reprodutor. `Set Start…` e `Set End…`
tamén están desactivados mentres non haxa nada seleccionado: actúan sobre a **primeira liña** da
selección.

**Os atallos son os de Gaupol, con `Alt`**: alí son `U`, `K` e `J` — letras soas, que
a xanela collería antes que a cela en edición, de modo que escribir un `k` nun texto
fixaría un final. `Select Previous…` e `Select Next…` son `Ctrl+Y` e `Ctrl+U` en Gaupol; `Ctrl+Y`
é un *refacer* nalgúns sistemas, e as frechas con `Alt` din *antes* e *despois* mellor ca
dúas letras. Unha cela en edición **valídase** antes de `Set Start…`, `Set End…` ou
`Insert Subtitle…`, como antes de calquera modificación.

## Fixar un bordo

- **Unha entrada do historial por xesto**: `Editar ▸ Desfacer` devolve a posición anterior, e nada máis.
- **As mesmas regras que a cela.** Un inicio fixado despois do final do subtítulo, ou un final antes do seu
  inicio, **non se rexeita**: fíxase, e a táboa marca o subtítulo como marca aquel
  cuxo valor se escribiu a man — véxase [As anomalías](table.md#as-anomalías). Nada se corrixe en silencio.
- **Unha posición que xa é a do bordo non fai nada**, e non engade nada ao historial.
- **A posición é sempre o comezo dun fotograma**: o reprodutor nunca indica unha posición
  dentro dun fotograma. Fixar un bordo e despois volver a el — con `Seek Selection Start`, por exemplo —
  **mostra o mesmo fotograma**.

## Inserir na posición

O subtítulo novo **comeza na posición**, e dura **tres segundos**, ou ata o inicio do seguinte
cando este chega antes. O seu texto está baleiro. Ocupa o seu lugar na orde do ficheiro: despois
de todos os que comezan nesa posición ou antes dela. A inserción é **unha entrada do historial**,
e o subtítulo inserido queda seleccionado: pódese escribir de inmediato.

## Seleccionar desde a posición

Os dous seguen a orde do ficheiro, non a selección actual. **Sen ningún veciño por ese lado**, a
selección vai ao último subtítulo do ficheiro para `Select Next…`, ao primeiro para
`Select Previous…` — o comportamento de Gaupol. Estes dous xestos non moven a película.

## Fotograma a fotograma

Avanzar e retroceder **un fotograma** — ou N, véxase máis abaixo —, para fixar un bordo **no fotograma exacto**.
Dous xestos, e dous botóns na barra de reprodución, a cada lado do botón de reprodución:

| Orde | Atallo | O que fai |
| :------ | :------- | :----------- |
| Vídeo ▸ Retroceder un fotograma | `Alt+Left` | leva a película cara atrás o paso en fotogramas |
| Vídeo ▸ Avanzar un fotograma | `Alt+Right` | lévaa cara adiante o paso en fotogramas |

- **O paso é un número de fotogramas, como mínimo un**: é o axuste `Frame step` das
  [preferencias](preferences.md#o-reprodutor). Un paso de N fotogramas mostra o fotograma **N máis adiante**, tanto a 25 fotogramas por
  segundo como a 23,976; **nunca se expresa en milisegundos**.
- **A película detense nos seus límites**: retroceder desde o primeiro fotograma déixaa alí, avanzar desde o último
  tamén. A reprodución queda detida despois dun paso.
- **Os botóns da barra** — dúas iconas, a cada lado de reproducir/pausa — **desencadean as mesmas accións que o menú**: o mesmo paso, desactivados
  sen vídeo como as entradas do menú, e a mesma axuda emerxente, que dá o atallo. **Un dedo que queda sobre un
  botón, ou sobre a tecla, encadea os pasos.**
- **Un paso cada vez.** Unha tecla mantida repítese máis rápido do que se fai un paso: a primeira repetición sae
  de inmediato, as que chegan mentres se fai o paso **non se acumulan** — comparten un só lugar, o
  último, que sae despois. Soltar a tecla detén polo tanto a imaxe onde se está, e non deixa que a película
  siga levada pola súa inercia.

## Mover un bordo un fotograma

Para axustar un bordo co teclado: **o inicio ou o final do subtítulo seleccionado**, un paso, antes ou despois.

| Orde | Atallo | O que fai |
| :------ | :------- | :----------- |
| Vídeo ▸ Adiantar o inicio | `Alt+Q` | o subtítulo **comeza un paso antes** |
| Vídeo ▸ Atrasar o inicio | `Alt+W` | **comeza un paso despois** |
| Vídeo ▸ Adiantar o final | `Alt+Shift+Q` | **remata un paso antes** |
| Vídeo ▸ Atrasar o final | `Alt+Shift+W` | **remata un paso despois** |

`Q` e `W` son os de Gaupol para *antes* e *despois*, con `Alt` polo motivo indicado máis arriba; o final
leva `Alt+Shift`, porque o `E` de Gaupol é aquí a letra de `Edit`.

- **Unha entrada do historial por movemento**, que se pode desfacer. O subtítulo veciño e o solapamento indícanse
  **como ao escribir**: o bordo fíxase, e a táboa marca o que xa non cadra — véxase
  [As anomalías](table.md#as-anomalías).
- **O mesmo paso que o reprodutor**, polo tanto un fotograma de forma predeterminada. **O que vale un fotograma** lese, nesta orde,
  na **taxa de fotogramas que declara o vídeo**, na do **documento cando conta en fotogramas** (MicroDVD), e despois
  na **grade que debuxan as posicións**. **Sen ningunha das tres, o xesto négase e dio**: escoller unha
  taxa de fotogramas ao chou desprazaría calquera bordo nunha cantidade que non é un fotograma.
- **Non require vídeo**, só unha selección: úsase tamén nun ficheiro contado en fotogramas, sen
  película. Cando hai unha película aberta, **a reprodución vai á nova posición**, para mostrar o fotograma que o bordo
  acaba de tomar.
- Un bordo non vai antes da orixe: retroceder un paso desde menos dun fotograma pono en cero.
