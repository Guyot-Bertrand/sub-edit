# Temporisar secun li video

<!-- machine-translated -->
> Ca págine es un traduction machinal e it ancor ne ha esset revidet de un lector del lingue.

*On supone que li film es aperit e li reproductor conosset: vide [Li reproductor](player.md).*

On regarda li film, on halta sur li fotogramma u un linea deve comensar, e **ta position deveni
li comense del subtitul**. Cin gestes, in li menu **Video**, sub li nómines queles Gaupol da les. Les
basa se sur quo ja existe: fixar un position es **li sam comande quam tipar li valore in li
cellule**, e inserter es **li sam quam `Redacter ▸ Inserer subtitules…`**.

| Comande | Via rapid | Quo it fa |
| :------ | :------- | :----------- |
| Video ▸ Fixar li comense ex position de video | `Alt+U` | fixa li **comense** del subtitul selectet al position del film |
| Video ▸ Fixar li fin ex position de video | `Alt+K` | fixa su **fin** al position del film |
| Video ▸ Inserer un subtitul a position de video | `Alt+J` | insere un subtitul quel **comensa** a ta position, e selecte it |
| Video ▸ Selecter li precedent ex position de video | `Alt+Up` | selecte li ultim subtitul quel **comensa ante** li position |
| Video ▸ Selecter li sequent ex position de video | `Alt+Down` | selecte li prim quel **comensa pos** li position |

**Omni es desactivat sin video aperit**, quam li gestes del reproductor. `Set Start…` e `Set End…`
es anc desactivat tam long quam nullcos es selectet: les actua sur li **prim linea** del
selection.

**Li vias rapid es ti de Gaupol, con `Alt`**: ta ili es `U`, `K` e `J` — letteres nud, queles
li fenestre vell prender ante li cellule in redaction, talmen que tipar un `k` in un textu
vell fixar un fin. `Select Previous…` e `Select Next…` es `Ctrl+Y` e `Ctrl+U` in Gaupol; `Ctrl+Y`
es un *refar* sur alcun sistemas, e li sagittes con `Alt` dice melior *ante* e *pos* quam
du letteres. Un cellule in redaction es **validat** ante `Set Start…`, `Set End…` o
`Insert Subtitle…`, quam ante cada modification.

## Fixar un extremité

- **Un entrata de historie per geste**: `Redacter ▸ Desfar` redona li position precedent, e nullcos plu.
- **Li sam regules quam li cellule.** Un comense fixat pos li fin del subtitul, o un fin ante su
  comense, **ne es refusat**: it es fixat, e li tabelle marca li subtitul quam it marca ti
  queles valore esset tipat a manu — vide [Li anomalies](table.md#li-anomalies). Nullcos es corecte in silentie.
- **Un position quel ja es ti del extremité fa nullcos**, e adjunte nullcos al historie.
- **Li position es sempre li comense de un fotogramma**: li reproductor nequande reporta un position
  intra un fotogramma. Fixar un extremité, poy retornar a it — con `Seek Selection Start`, por exemple —
  **monstra li sam fotogramma**.

## Inserter al position

Li nov subtitul **comensa al position**, e dura **tri secondes**, o til li comense del sequent
si ti veni plu tost. Su textu es vacui. It prende su loc in li órdine del file: pos
omni ti queles comensa a ta position o ante it. Li insertion es **un entrata de historie**,
e li subtitul inseret es selectet: on posse tipar it strax.

## Selecter ex li position

Ambi seque li órdine del file, ne li selection actual. **Sin vicin de ta latere**, li
selection va al ultim subtitul del file por `Select Next…`, al prim por
`Select Previous…` — li comportament de Gaupol. Ci du gestes ne move li film.

## Fotogramma por fotogramma

Avansar e retroear **per un fotogramma** — o per N, vide infra —, por fixar un extremité **al fotogramma exact**.
Du gestes, e du butones in li barre de reproduction, de ambi lateres del buton de reproduction:

| Comande | Via rapid | Quo it fa |
| :------ | :------- | :----------- |
| Video ▸ Passu retro | `Alt+Left` | mova li film retro per li passu de fotogramma |
| Video ▸ Passu avan | `Alt+Right` | mova it avan per li passu de fotogramma |

- **Li passu es un númere de fotogrammas, un minimum**: it es li parametre `Frame step` del
  [preferenties](preferences.md#li-reproductor). Un passu de N fotogrammas monstra li fotogramma **N plu lontan**, a 25 fotogrammas per
  second quam a 23,976; it es **nequande expresset in millisecondes**.
- **Li film halta a su limites**: retroear ex li prim fotogramma lassa it ta, avansar ex li ultim
  anc. Li reproduction es haltet pos un passu.
- **Li butones del barre** — du icones, de ambi lateres de reproducter/pausa — **declencha li sam actiones quam li menu**: li sam passu, desactivat
  sin video quam li entratas del menu, e li sam information-bulla, quel dona li via rapid. **Un digite quel resta sur un
  buton, o sur li clave, cadena li passus.**
- **Un passu a un vice.** Un clave tenet repete plu rapid quam un passu es fat: li prim repetition ea
  strax, ti queles arriva durant que li passu es in curs **ne se acumula** — les partisse un sol loc, li
  ultim, quel ea pos. Relaxar li clave halta dunc li image u on es, e ne lassa li film
  continuar sub su impulse.

## Mover un extremité per un fotogramma

Por ajustar un extremité con li claviero: **li comense o li fin del subtitul selectet**, per un passu, plu tost o plu tard.

| Comande | Via rapid | Quo it fa |
| :------ | :------- | :----------- |
| Video ▸ Mover li comense plu tost | `Alt+Q` | li subtitul **comensa un passu plu tost** |
| Video ▸ Mover li comense plu tard | `Alt+W` | it **comensa un passu plu tard** |
| Video ▸ Mover li fin plu tost | `Alt+Shift+Q` | it **fini un passu plu tost** |
| Video ▸ Mover li fin plu tard | `Alt+Shift+W` | it **fini un passu plu tard** |

`Q` e `W` es ti de Gaupol por *plu tost* e *plu tard*, con `Alt` por li rason dat supra; li fin
prende `Alt+Shift`, pro que li `E` de Gaupol es ci li littere de `Edit`.

- **Un entrata de historie per movement**, anullabil. Li subtitul vicin e li superposition es reportat
  **quam durant li tipation**: li extremité es fixat, e li tabelle marca quo ne tene plu — vide
  [Li anomalies](table.md#li-anomalies).
- **Li sam passu quam li reproductor**, dunc un fotogramma predefinit. **Quo vale un fotogramma** es leet, in ca órdine,
  ex **li frequentie de fotogrammas quel li video declara**, ti del **document quande it conta in fotogrammas** (MicroDVD), poy
  **li grille quel li positiones tracea**. **Sin null de li tri, li geste refusa e dice it**: eliger un
  frequentie de fotogrammas a hasard vell displaciar cada extremité per alquo quel ne es un fotogramma.
- **It ne besona un video**, solmen un selection: it es anc usat sur un file contat in fotogrammas, sin
  film. Quande un film es aperit, **li reproduction va al nov position**, por monstrar li fotogramma quel li extremité
  ha just prendet.
- Un extremité ne va ante li origine: retroear per un passu ex min quam un fotogramma pone it a zero.
