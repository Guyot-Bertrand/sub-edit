# Anullar e refar

<!-- machine-translated -->
> Ca págine es un traduction machinal e it ancor ne ha esset revidet de un lector del lingue.

Chascun operation intra in un historie, e posse esser anullat. Un cellule redactet conta quam
**un** entrata: li tipation ne es registrat tecla per tecla, solmen li validation importa.

| Action | Via rapid | U |
| :----- | :------- | :---- |
| Desfar | `Ctrl+Z` | menu **Redacter**, barre de instrumentes |
| Refar | `Ctrl+Shift+Z` | menu **Redacter**, barre de instrumentes |

## Li action nómina quo it va anullar

Li menu ne dice «Desfar» ma **«Desfar: redacter un textu»**, secun li operation
quel va esser anullat. Li buton del barre de instrumentes conserva li sol parol — su
largore altrimen sequeva li ultim operation, e it se movet sub li
punctator — ma su information-bulla porta li etiquette complet.

| Operation | Quo li action lee |
| :-------- | :-------------------- |
| un cellule de textu redactet | `editing a text` |
| un comense, un fin | `editing a start`, `editing an end` |
| un duration | `editing a duration` |
| un insertion de lineas | `inserting` |
| un remotion de lineas | `removing` |
| un displaciament, `Shift Whole File onto Grid` includet | `shifting` |
| un transformation | `transforming` |
| un conversion de frequentie de fotogrammas | `converting the frame rate` |
| un alliniation sur un frequentie de fotogrammas | `aligning on the frame rate` |
| un remotion de mentiones | `removing hearing-impaired mentions` |
| metter in italic | `putting in italics` |
| exter li italic | `taking italics out` |
| un change de casu de littere | `changing the case` |
| strecs adjuntet, removet | `adding dialogue dashes`, `removing dialogue dashes` |
| un coalescentie de lineas | `merging` |
| un separation de un linea | `splitting` |
| textus decupet, collat | `cutting texts`, `pasting texts` |
| un ajustament de durationes | `adjusting durations` |
| li apertion de un traduction | `opening a translation` |
| li adjunction de un file | `appending a file` |
| li separation del projecte | `splitting the project` |
| un correction acceptat in `Correct Texts…`, o fat per `Check Spelling…` | `correcting texts` |
| un substitution, omni substitutiones | `replacing`, `replacing all` |

**Vint-sett etiquettes, e to es omnicos quo li fenestre posse produer.** Li nucleo
nómina un vint-ottesim — `sorting` — quel null action del fenestre atinge: li ordination
evenit solmen sub un politica de órdine strict, durant que li fenestre aperte su documentes
sub li politica indulgent, quel reporta li desórdine vice reparar it.

**`Shift Whole File onto Grid` ne es distinget de un displaciament**, e to es
exact: it es un displaciament, queles quantité esset mesurat vice tipat.

It es li sam paroles quam in li linea de comande: les es scrit un vez,
in li nucleo.

Quande null cose es a anullar, li action lee simplicmen «Desfar» **e it es
desactivat**: su via rapid fa nullcos.

## Li marca de modification

Li titul del fenestre porta un marca durant que li file diferte de ti sur li
disco — un asterisco, o quo li platteforma usa vice it. **Talmen anc li etiquette
del lingueta**, `premier.srt*`: li titul parla solmen del projecte quel on regarda,
li etiquette informa pri ti quel on ne regarda.

**Chascun lingueta ha su propri historie.** Anullar desfa solmen quo li projecte del
lingueta actual ha recivet — includet un `Replace All` fat sur omni projectes: it
lassa un entrata in chascun projecte quel it ha tanget.

**It desapare si on anulla til li punctu de comensa.** To ne es un detallia de
monstration: li númere de modificationes es contat, ne registrat quam un yes o un no, ergo
retroear posse reducter it a zero, e li marca desapare.

## Du coses a saver

**Li punctu de referentie es li ultim salvation**, o li apertion del file si null
ha existet. Salvar move it: li marca desapare, e anullar pos to
readjunte it.

Un operation fat pos un anullation **deleta quo on posset refar**: refar
va reproducter un comande quel su statu inicial ne existe plu. To es li
comportament de cada redactor, e it es voluntari.

**Durant que un cellule es aperit**, `Undo` e `Redo` — ex li menu o li barre de instrumentes —
validar prim li input, quam li altri gestes de
[Redacter un cellule](editing.md). Quo just esset tipat deveni alor li ultim
entrata del historie: `Undo` anulla it, ne li operation ante; `Redo` ne ha
plu nullcos a refar, pro que un input deleta quo on posset refar. Un cellule
aperit u nullcos esset tipat lassa null entrata: `Undo` anulla alor li
operation precedent.

Li historie conserva mil entratas e oblida li max old ultra to.
