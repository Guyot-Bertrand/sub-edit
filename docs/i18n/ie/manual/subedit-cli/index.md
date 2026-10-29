# subedit-cli — manual

<!-- machine-translated -->
> Ca págine es un traduction machinal e it ancor ne ha esset revidet de un lector del lingue.

Li instrument de linea de comande de subedit: laborar con files de subtitules sin interfacie
grafic, in lotes o ex un script. Por li altri programma e por saver
u comensar, vide
[li manual](../index.md).

> **Statu actual.** Li dissette subcomandes de `subedit-cli` existe: inspecter,
> converter, displaciar, transformar, retemporisar li frequentie de fotogrammas, alliniar sur un grille de fotogrammas, remover
> li mentiones por surdes, ajustar li durationes, substituer un textu, changear li casu de littere,
> li italic e li strecs de dialog, remetter li subtitules in órdine, corecter li
> textus con li modelles de Gaupol, mettre files un pos li altri, separar un
> file in du, e scrir un traduction a li positiones de su file principal. Ca manual
> descrive quo existe, nequande quo es planat; quo va venir pos es in
> li [plan de via](../../../../feuille-de-route.md).

## Sectiones

| Section | Contenete |
| :------ | :------ |
| [Installation](installation.md) | compilar e installar li instrument |
| [Invocation](invocation.md) | optiones global, outputs, codes de exeunte |
| [Li file de configuration](invocation.md#li-file-de-configuration) | quo `adjust` e `correct` lee del file de parametres del fenestre, `--config`, `--no-config` |
| [Tractar un arbore](lots.md) | plu files, un tot directoria, `--recursive` |
| [`inspect`](inspect.md) | reportar de quo un file es composit |
| [`convert`](convert.md) | scrir un file in un altri formate o un altri forme |
| [`shift`](shift.md) | mover omni positiones del sam quantité |
| [`transform`](transform.md) | corecter omni positiones ex du punctus conosset quam just |
| [`framerate`](framerate.md) | retemporisar un file de un frequentie de fotogrammas a un altri |
| [`snap`](snap.md) | remetter li marcas de tempore sur li fotogrammas de un frequentie de fotogrammas |
| [`hearing-impaired`](hearing-impaired.md) | remover li mentiones por surdes |
| [`adjust`](adjust.md) | ajustar li durationes a un rapidesse de lectura e a limites |
| [`replace`](replace.md) | substituer un textu sin rupter un etiquette |
| [`case`](case.md) | mettre li textus in majusculas de titul, majuscul de frase, majusculas o minusculas |
| [`italics`](italics.md) | mettre li textus in italic, o remover lor italic |
| [`dialogue-dashes`](dialogue-dashes.md) | mettre strecs de dialog, o remover les |
| [`sort`](sort.md) | remetter li subtitules in li órdine de lor comense |
| [`join-repeats`](join-repeats.md) | crear un sol subtitul ex li peces de un textu queles seque se |
| [`disjoint`](disjoint.md) | decuper li subtitules superponent por que null intervalle superpone plu un altri |
| [`repair`](repair.md) | encatenar `join-repeats`, `sort` e `disjoint` sur un file mal segmentat, foris de órdine e con superpositiones |
| [`correct`](correct.md) | corecter li textus con li modelles de Gaupol: mentiones, union e separation de paroles, errores comun, majusculisation, ruptura de linea |
| [`append`](append.md) | mettre files pos li unesim, in un sol file |
| [`split-file`](split-file.md) | separar un file in du, li cauda reportat al origine |
| [`pair`](pair.md) | scrir un traduction a li positiones de su file principal, e dir qualmen su lineas esset alliniat |
| [`transcribe`](transcribe.md) | scrir li parol de un video o de un file audio quam subtitules |
| [`sync`](sync.md) | retemporisar li subtitules de un file sur li audio de un film |
| [Instalar li utensiles de transcription](install-transcription.md) | li motor, li model e `ffmpeg`, quel li programma ne livra |
| [`config`](config.md) | listar, monstrar e changear li parametres de operation partit con li fenestre |
