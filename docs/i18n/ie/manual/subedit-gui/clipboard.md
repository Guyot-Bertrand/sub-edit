# Decuper, copiar e collar textus

<!-- machine-translated -->
> Ca págine es un traduction machinal e it ancor ne ha esset revidet de un lector del lingue.

Tri entratas del menu `Edit`, sub `Undo` e `Redo`, e supra
`Find and Replace…`.

| Entrata | Via rapid | Quo it fa |
| :---- | :------- | :----------- |
| Talliar textus | `Ctrl+X` | copia li textus del selection, poy vacua les |
| Copiar textus | `Ctrl+C` | copia li textus del selection |
| Collar textus | `Ctrl+V` | scri li textus copiat comensante ex li prim linea selectet |

**It es textus quel viaja, e nullcos plu**: null comense, null fin, null linea
integral. Collar dunc move null position — li temporisation del linea
quel recive un textu resta su propri. To es quo Gaupol fa.

## Sur quo les actua

**Con un traduction, li textu del columne actual**: li traduction si li
cellule actual es in li columne `Translation`, li textu principal altrubi —
vide [Li tabelle](table.md#li-textu-quel-li-operationes-ciela). `Cut Texts` dunc vacua
li traduction e lassa li textu principal, e `Paste Texts` scri in li
columne u on es. **Li porta-papere porta null document**: on
posse copiar un cellule del textu principal e collar it in li traduction, e
li contrari.

**Li selection, e nequande li tot file.** Li tri entratas es desactivat tam long
quam nullcos es selectet. Altrubi in li fenestre, «nullcos selectet» significa
«li tot file»; ci, un malfortunat `Ctrl+X` vell vacuar omni li textus
del document, e un collation have null linea u comensar.

Durant que un cellule es aperit, `Ctrl+X`, `Ctrl+C` e `Ctrl+V` apartene
al campe de input: les decupa, copia e colla characteres, just quam `Del`
efface ta un character e ne un subtitul. Lansat ex li menu, li entratas
validar prim li input: `Copy Texts` copia alor li textu quam it just ha esset
tipat. Vide [Redacter un cellule](editing.md).

## Copiar

`Copy Texts` copia li textu de chascun linea selectet, del prim al
ultim. **Un selection discontinu conserva su forme**: un linea omisset
del selection deveni un buse, e li collation ne va superscrir it.

**Un linea selectet quel textu es vacui deveni un buse anc**:
collat, it lassa li linea de cielo quam it es. Copiar un sol textu vacui dunc
cola nullcos.

Li porta-papere del sistema recive ci textus **quam textu simplic, separat per un
linea vacui** — un buse es scrit ta quam un textu vacui, e pro to un
textu vacui ne posse esser altricos quam un buse: li collation dona li sam
resultate, che li copie vene de ca fenestre o de un altri programma. Quo on copia ci
posse dunc esser collat in cadun programma, e quo on copia altrubi posse esser collat ci.

Copiar ne modifica li document, e ne intra in li historie.

## Decuper

`Cut Texts` copia, poy **vacua li textus** del lineas selectet. Li lineas
resta, con lor positiones: decuper ne es remover — por remover
lineas, it es [`Remove Subtitles`](lines.md#remover).

Decuper textus quel es ja omni vacui fa nullcos, e ne intra in li
historie.

## Collar

`Paste Texts` scri li textus del porta-papere **comensante ex li prim
linea selectet**, un linea per textu, in infra.

| Quo li porta-papere contene | Quo evenit |
| :-------------------------- | :----------- |
| un textu | it substitue ti del linea |
| un buse, o un textu vacui copiat | li linea conserva su textu |
| plu textus quam il resta lineas | lineas es adjuntet al fin por recivir les |

Li lineas adjuntet nasce quam lineas inseret pos li ultim: tri
secondes chascun, li un pos li altri. **Li fenestre dice it**, in un casse
de information:

```text
inserted 2 subtitles to fit the clipboard
```

Pos li collation, **li lineas scrit es selectet**.

Un collation quel changea nullcos — li sam textus sur li sam lineas — ne intra in
li historie.

### Li etiquettes de un altri formate es traductet

Un copie fat in ca fenestre **memora li formate de su document**, e ta
memorie supervive al apertion de un altri file. Collar in un document de
un altri formate dunc traduce li etiquettes, quam `Save As…` fa:
`{\i1}Bonjour{\i0}` copiat de un Advanced SSA es collat quam `<i>Bonjour</i>` in un
SubRip.

**Quo li formate de cielo ne posse scrir es perdit, e it es dit**, in
li paroles de `Save As…`:

```text
pasting SubRip texts into LRC: 1 tag dropped
```

Contrarimen a `Save As…`, nullcos es demandat ante: li collation es anullat per
`Ctrl+Z`, quel redona li textus quam les esset.

**Un textu copiat foris `subedit` have null formate**, e es collat quam it es:
il ne ha nullcos a traducter ex null loc. To es anc li cas de un copie ex
ca fenestre quel li porta-papere del sistema ha remplazzat ex tande — it es li
contenete del porta-papere del sistema quel importa.

Quande ambi missages es debit, les intra in un sol casse, separat per
un punctu-comma.

## Quo li action de anullar dice pri it

Chascun geste es **un** entrata del historie, quant que sia li númere de
lineas queles it scri o adjunte.

| Operation | Quo `Undo` lee |
| :-------- | :---------------- |
| decuper | `Undo: cutting texts` |
| collar | `Undo: pasting texts` |

Vide [Anullar e refar](undo.md).
