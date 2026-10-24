# subedit — manual

<!-- machine-translated -->
> Esta páxina é unha tradución automática e aínda non a revisou ningún lector da lingua.

subedit le, inspecciona e recalcula os tempos de ficheiros de subtítulos. Preséntase como dous
programas que comparten o mesmo núcleo, e polo tanto as mesmas regras e as mesmas palabras.

| Programa | Para que | O seu manual |
| :-------- | :-------------- | :--------- |
| `subedit-gui` | abrir un ficheiro, velo nunha táboa, editalo, miralo xunto coa película | [o manual da xanela](subedit-gui/index.md) |
| `subedit-cli` | procesar ficheiros por lotes, desde un script, sen interface | [o manual da liña de ordes](subedit-cli/index.md) |

Os dous len e escriben **os nove formatos** — SubRip, WebVTT, SubViewer 2,
Sub Station Alpha, Advanced SSA, MicroDVD, MPL2, TMPlayer, LRC — e
recoñecen o formato polo seu contido e non pola súa extensión. **Tamén a codificación
se recoñece polo contido**: un ficheiro en Latin-1, en CP1252 ou en UTF-16
ábrese, dise como se leu, e vólvese escribir na súa propia codificación — un
ficheiro aberto e despois gardado sen ningún cambio non se move nin un byte.

## Por onde comezar

| O que quere | Onde ir |
| :----------------- | :------- |
| instalar a ferramenta | [Instalación](subedit-cli/installation.md) — vale para os dous programas |
| abrir un ficheiro e ver o que contén | [A táboa](subedit-gui/table.md) |
| recalcular os tempos de subtítulos desprazados nunha cantidade constante | [As operacións](subedit-gui/operations.md#shift-positions), ou [`shift`](subedit-cli/shift.md) |
| recalcular os tempos dun ficheiro escrito para outra taxa de fotogramas | [`framerate`](subedit-cli/framerate.md), e [`snap`](subedit-cli/snap.md) se a temporización xa é correcta |
| saber para que taxa de fotogramas se escribiu un ficheiro | [A grade de fotogramas](subedit-gui/frame-grid.md), ou [`inspect`](subedit-cli/inspect.md) |
| eliminar as mencións para persoas xordas | [As operacións](subedit-gui/operations.md#remove-hearing-impaired-mentions), ou [`hearing-impaired`](subedit-cli/hearing-impaired.md) |
| corrixir mencións, erros comúns, maiúsculas e saltos de liña nunha soa pasada | [`Correct Texts…`](subedit-gui/correct-texts.md), ou [`correct`](subedit-cli/correct.md) |
| verificar a ortografía palabra por palabra, cos dicionarios do sistema | [`Check Spelling…`](subedit-gui/check-spelling.md) |
| alongar subtítulos demasiado curtos para poder lelos | [`Adjust Durations…`](subedit-gui/operations.md#adjust-durations), ou [`adjust`](subedit-cli/adjust.md) |
| volver poñer en orde un ficheiro cuxos subtítulos non o están | [`sort`](subedit-cli/sort.md) |
| facer un só subtítulo cos anacos dun mesmo texto que se suceden | [`Join Repeated Subtitles`](subedit-gui/lines.md#unir-os-subtítulos-repetidos), ou [`join-repeats`](subedit-cli/join-repeats.md) |
| endereitar dunha vez un ficheiro mal segmentado, desordenado e con solapamentos | [`repair`](subedit-cli/repair.md) |
| eliminar os solapamentos entre subtítulos | [`Make Ranges Disjoint…`](subedit-gui/lines.md#facer-as-franxas-disxuntas), ou [`disjoint`](subedit-cli/disjoint.md) |
| poñer as dúas partes dunha película unha detrás da outra | [`Append File…`](subedit-gui/operations.md#append-file), ou [`append`](subedit-cli/append.md) |
| cortar unha película en dúas partes | [`Split Project…`](subedit-gui/operations.md#split-project), ou [`split-file`](subedit-cli/split-file.md) |
| cambiar as maiúsculas, a cursiva ou os trazos de diálogo de todo un ficheiro | [As operacións](subedit-gui/operations.md#italic), ou [`case`](subedit-cli/case.md), [`italics`](subedit-cli/italics.md), [`dialogue-dashes`](subedit-cli/dialogue-dashes.md) |
| buscar unha palabra e substituíla, sen romper as etiquetas | [Buscar e substituír](subedit-gui/find-and-replace.md), ou [`replace`](subedit-cli/replace.md) |
| escribir a fala dunha película como subtítulos | [`Transcribe Audio…`](subedit-gui/video.md#transcribir-o-son), ou [`transcribe`](subedit-cli/transcribe.md) — despois de [instalar as ferramentas](subedit-cli/install-transcription.md) |
| traducir xunto ao texto orixinal | [Abrir unha tradución](subedit-gui/files.md#abrir-unha-tradución), ou [`pair`](subedit-cli/pair.md) para recalcular os seus tempos sobre o ficheiro principal |
| procesar cen ficheiros dunha vez | [Procesar unha árbore](subedit-cli/lots.md), despois [Invocación](subedit-cli/invocation.md) |

**Entre `framerate` e `snap`, un erro pasa desapercibido.** Se
dubida, [`snap`](subedit-cli/snap.md#snap-non-é-framerate-e-confundilos-non-se-nota)
comeza pola comparación das dúas.

## O que promete este manual

**Describe o que existe, nunca o que está previsto.** O que virá, e en
que orde, está na [folla de ruta](../../../feuille-de-route.md).

**Este manual está escrito en inglés, e traducido.** As súas traducións ségueno
páxina a páxina, no directorio `docs/i18n/` do repositorio, unha árbore por lingua. Os
programas falan a lingua do seu contorno; os nomes que mostran — subordes,
opcións, etiquetas dos menús — cítanse aquí tal como o programa os escribe en inglés.

**Os exemplos e as imaxes xéranse, o texto non.** Os bloques
`console` prodúcense executando de verdade a orde; as
capturas de pantalla, construíndo a xanela real e fotografándoa. Ningún
dos dous pode, polo tanto, describir unha ferramenta que xa non existe. O que queda sen rede
é o texto que os rodea, cuxa corrección depende da revisión.

**As capturas de pantalla mostran as dúas paletas**, `Light` e `Dark` — as que
ofrecen [as preferencias](subedit-gui/preferences.md#o-tema).

## As dúas superficies din as mesmas palabras

Un motivo para ler calquera dos dous manuais cando se usa o outro programa: as
mensaxes de erro, os nomes dos formatos, os veredictos da grade e as etiquetas
de desfacer escríbense **unha soa vez**, no núcleo, e os dous programas
cítanos. Un ficheiro rexeitado dá a mesma frase nos dous lados.

O que difire é o que pode facer cada superficie:

| | `subedit-gui` | `subedit-cli` |
| :--- | :------------ | :------------ |
| varios ficheiros á vez | si — unha lapela por proxecto | si |
| unha tradución xunto ao ficheiro principal | si | si — `-t` nas subordes de texto, e [`pair`](subedit-cli/pair.md) para recalcular os seus tempos |
| engadir un ficheiro, dividir un proxecto | si | si — [`append`](subedit-cli/append.md), [`split-file`](subedit-cli/split-file.md) |
| desfacer | si, mil entradas | non aplicable — nada se cambia no seu sitio sen pedilo |
| editar a man un texto ou unha posición | si | non |
| inserir, eliminar, fusionar e dividir liñas | si | non |
| buscar e substituír | si | si — [`replace`](subedit-cli/replace.md) |
| cortar, copiar e pegar textos | si | non |
| axustar as duracións, a cursiva, as maiúsculas, os trazos | si | si — [`adjust`](subedit-cli/adjust.md), [`italics`](subedit-cli/italics.md), [`case`](subedit-cli/case.md), [`dialogue-dashes`](subedit-cli/dialogue-dashes.md) |
| corrixir os textos mediante padróns | si — `Correct Texts…` | si — [`correct`](subedit-cli/correct.md) |
| verificar a ortografía palabra por palabra | si — `Check Spelling…` | non |
| ver a película mentres se temporiza | si | non |
| escribir por riba da entrada | `Save` | `--in-place` |
