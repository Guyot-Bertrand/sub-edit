# Cortar, copiar e pegar textos

<!-- machine-translated -->
> Esta páxina é unha tradución automática e aínda non a revisou ningún lector da lingua.

Tres entradas do menú `Edit`, baixo `Undo` e `Redo`, e por riba de
`Find and Replace…`.

| Entrada | Atallo | O que fai |
| :---- | :------- | :----------- |
| Cortar os textos | `Ctrl+X` | copia os textos da selección e despois baléiraos |
| Copiar os textos | `Ctrl+C` | copia os textos da selección |
| Pegar os textos | `Ctrl+V` | escribe os textos copiados a partir da primeira liña seleccionada |

**O que viaxa son textos, e nada máis**: nin inicio, nin final, nin liña
enteira. Pegar non move polo tanto ningunha posición — a temporización da liña que
recibe un texto segue sendo a súa. É o que fai Gaupol.

## Sobre que actúan

**Cunha tradución, sobre o texto da columna actual**: a tradución se a
cela actual está na columna `Translation`, o texto principal noutro caso —
véxase [A táboa](table.md#o-texto-ao-que-apuntan-as-operacións). `Cut Texts` baleira polo tanto
a tradución e deixa o texto principal, e `Paste Texts` escribe na
columna na que se está. **O portapapeis non leva ningún documento**: pódese
copiar unha cela do texto principal e pegala na tradución, e
ao revés.

**A selección, e nunca o ficheiro enteiro.** As tres entradas están desactivadas mentres
non haxa nada seleccionado. Noutras partes da xanela, «nada seleccionado» significa
«o ficheiro enteiro»; aquí, un `Ctrl+X` desafortunado baleiraría todos os textos
do documento, e un pegado non tería ningunha liña pola que comezar.

Mentres hai unha cela aberta, `Ctrl+X`, `Ctrl+C` e `Ctrl+V` pertencen
ao campo de entrada: cortan, copian e pegan caracteres, igual que `Del`
borra alí un carácter e non un subtítulo. Lanzadas desde o menú, as entradas validan primeiro
a entrada: `Copy Texts` copia entón o texto tal como se acaba de
escribir. Véxase [Editar unha cela](editing.md).

## Copiar

`Copy Texts` copia o texto de cada liña seleccionada, da primeira á
última. **Unha selección descontinua conserva a súa forma**: unha liña que queda fóra
da selección convértese nun oco, e o pegado non a sobrescribirá.

**Unha liña seleccionada cuxo texto está baleiro tamén se converte nun oco**:
pegada, deixa a liña de destino tal como está. Copiar un só texto baleiro non
pega polo tanto nada.

O portapapeis do sistema recibe estes textos **como texto simple, separados por unha
liña baleira** — un oco escríbese alí como un texto baleiro, e por iso un texto
baleiro non pode ser outra cousa ca un oco: o pegado dá o mesmo resultado
tanto se a copia vén desta xanela como doutro programa. O que se copia aquí pódese
polo tanto pegar en calquera programa, e o que se copia noutro lugar pódese pegar aquí.

Copiar non modifica o documento, e non entra no historial.

## Cortar

`Cut Texts` copia e despois **baleira os textos** das liñas seleccionadas. As liñas
quedan, coas súas posicións: cortar non é eliminar — para eliminar
liñas, é [`Remove Subtitles`](lines.md#eliminar).

Cortar textos que xa están todos baleiros non fai nada, e non entra no
historial.

## Pegar

`Paste Texts` escribe os textos do portapapeis **a partir da primeira liña
seleccionada**, unha liña por texto, cara abaixo.

| O que contén o portapapeis | O que pasa |
| :-------------------------- | :----------- |
| un texto | substitúe o da liña |
| un oco, ou un texto baleiro copiado | a liña conserva o seu texto |
| máis textos que liñas restantes | engádense liñas ao final para recibilos |

As liñas engadidas nacen como liñas inseridas despois da última: tres
segundos cada unha, unha tras outra. **A xanela dio**, nunha caixa de información:

```text
inserted 2 subtitles to fit the clipboard
```

Despois do pegado, **as liñas escritas quedan seleccionadas**.

Un pegado que non cambia nada — os mesmos textos nas mesmas liñas — non entra
no historial.

### As etiquetas doutro formato tradúcense

Unha copia feita nesta xanela **lembra o formato do seu documento**, e
esa memoria sobrevive á apertura doutro ficheiro. Pegar nun documento doutro
formato traduce polo tanto as etiquetas, como fai `Save As…`:
`{\i1}Bonjour{\i0}` copiado dun Advanced SSA pégase como `<i>Bonjour</i>` nun
SubRip.

**O que o formato de destino non pode escribir pérdese, e dise**, coas
palabras de `Save As…`:

```text
pasting SubRip texts into LRC: 1 tag dropped
```

A diferenza de `Save As…`, non se pregunta nada antes: o pegado desfaise con
`Ctrl+Z`, que devolve os textos tal como estaban.

**Un texto copiado fóra de `subedit` non ten formato**, e pégase tal cal:
non hai nada que traducir desde ningures. Tamén é o caso dunha copia desta
xanela que o portapapeis do sistema substituíu despois — o que conta é o
contido do portapapeis do sistema.

Cando corresponden as dúas mensaxes, caben nunha soa caixa, separadas por
un punto e coma.

## O que di diso a acción de desfacer

Cada xesto é **unha** entrada do historial, sexa cal sexa o número de
liñas que escribe ou engade.

| Operación | O que se le en `Undo` |
| :-------- | :---------------- |
| cortar | `Undo: cutting texts` |
| pegar | `Undo: pasting texts` |

Véxase [Desfacer e refacer](undo.md).
