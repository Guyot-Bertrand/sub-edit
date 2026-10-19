# O vídeo asociado

<!-- machine-translated -->
> Esta páxina é unha tradución automática e aínda non a revisou ningún lector da lingua.

O menú **Vídeo** leva primeiro dúas ordes, e despois os xestos que manexan a película
— véxase [O reprodutor](player.md#moverse-pola-película).

| Orde | Atallo | O que fai |
| :------ | :------- | :----------- |
| Seleccionar vídeo… | ningún | escolle o ficheiro de vídeo ao que corresponde o documento |
| Reproducir / Pausa | `Ctrl+P` | reproduce ou detén o vídeo — véxase [O reprodutor](player.md#reproducir-e-deter) |

O vídeo asociado é **unha propiedade do documento aberto**, non un axuste da
aplicación: abrir outro ficheiro de subtítulos comeza desde cero. Non
se escribe en ningún ficheiro e desaparece coa xanela.

## A pista de son

Unha película pode levar varias pistas de son — a versión orixinal, unha dobraxe, un comentario. O
submenú **`Vídeo ▸ Son ▸ Lingua`** enuméraas, **unha entrada por pista**, e **marca a que se reproduce**:

| O que dá o ficheiro | O que di a entrada |
| :------------------ | :------------------ |
| unha lingua e un título | `1: fra — Original` |
| só unha lingua | `2: eng` |
| só un título | `3: Commentary` |
| ningún dos dous | `1` |

O número é **o lugar da pista na película**, unha a unha, na orde do ficheiro: dúas pistas
poden dicir `eng`, e un ficheiro que non di nada das súas etiquetas segue tendo unha primeira e unha segunda.

**Escoller unha entrada reproduce esa pista.** A marca é a resposta do reprodutor, non o clic: indica a
pista que se está a reproducir.

- **O submenú está desactivado** sen vídeo ou sen pista de son. Cunha soa pista, segue activado e
  móstraa, marcada: indica a lingua da película. Os dous xestos do volume, que están ao seu lado, seguen
  activados en calquera caso: véxase [O volume](player.md#o-volume).
- **A pista non se lembra** dun vídeo ao seguinte, nin dunha sesión á seguinte: un número de pista é
  o do seu ficheiro, e levalo á película seguinte escollería calquera cousa que leve alí ese número.
  Volver a unha lapela abre de novo a súa película na pista que o reprodutor escolle por si mesmo.
- Ningún atallo: cámbiase de lingua poucas veces, e o menú abonda.

## Escoller un vídeo

O diálogo ábrese no directorio do ficheiro de subtítulos e filtra polas
extensións de vídeo recoñecidas:

    .avi .flv .m2ts .mkv .mov .mp4 .ogg .ogv .vob .webm

Tamén ofrece mostrar todos os ficheiros. Cancelar o diálogo non cambia
nada.

O ficheiro escollido **ábrese de inmediato**: a imaxe aparece na xanela,
en pausa no seu primeiro instante. Véxase [O reprodutor](player.md).

**Soltar un ficheiro de vídeo na xanela fai o mesmo** — véxase
[Soltar ficheiros na xanela](files.md#soltar-ficheiros-na-xanela).

**O vídeo é o do proxecto da lapela actual.** Cada lapela conserva
o seu; cambiar de lapela cambia a película. **Volver a unha lapela retoma a súa
película onde se deixou**, en pausa, como calquera apertura. Outra película
escollida para ese proxecto mentres tanto comeza, pola súa parte, desde o principio.

## A proposta automática

Cando se abre un ficheiro de subtítulos, **se non se escolleu ningún vídeo**,
a xanela busca un no mesmo directorio: un ficheiro de vídeo cuxo nome
sen extensión é un prefixo do noso, ata un punto.

| Ficheiro de subtítulos | Vídeo atopado | Por que |
| :------------ | :---------- | :-- |
| `film.fr.srt` | `film.mkv` | `film` é un prefixo de `film.fr` |
| `film.en.forced.srt` | `film.mkv` | igualmente, sexa cal sexa o número de segmentos |
| `film.fr.srt` | `film.fr.mkv` en lugar de `film.mkv` | gaña o nome máis longo |
| `film.fr.srt` | ningún, se están `film.mkv` **e** `film.mp4` | non se poden distinguir dúas candidatas |
| `film.fr.srt` | ningún, se só está `fil.mkv` | `fil` non se detén nun punto |

**Unha proposta non é unha elección.** Déixase substituír por outra
proposta — abrir outro ficheiro, gardar con outro nome. Unha elección
explícita, en cambio, nunca se sobrescribe: unha vez usado `Select Video…`, a
proposta automática xa non volve detrás dela.

Iso é o que fai aceptable a suposición: pódese corrixir, e a súa
corrección mantense.

## O que mostra a barra de estado

Á dereita da barra de estado, unha liña di a que corresponde o documento. É
a última de tres: primeiro vén a [codificación](files.md#a-codificación-na-barra-de-estado)
do ficheiro lido, e despois a [grade de fotogramas](frame-grid.md) deducida das
posicións.

| Situación | O que se escribe |
| :-------- | :-------------- |
| hai un vídeo asociado | `Video: film.mkv` |
| … e `ffprobe` dá a súa taxa de fotogramas | `Video: film.mkv, 24000/1001 fps` |
| ningún | `No video` |

**A taxa de fotogramas vai coa película en lugar de ao seu lado.** Son un só feito — aquilo a que
corresponde este documento — e unha terceira mención poñeríaa no mesmo nivel que
a [grade deducida das posicións](frame-grid.md), que é outro.

## Cando non hai película

A xanela mostra un botón **`Select Video…`** onde iría a imaxe, en lugar de
nada en absoluto. É a mesma orde que a do menú `Video`, ao
alcance do rato: unha ausencia sobre a que non se pode actuar é difícil de distinguir dunha
avaría.

O botón desaparece en canto se abre unha película, e a imaxe ocupa **todo** o seu
lugar: a altura que lle deixa a táboa por riba — a que
[`window.table-share`](preferences.md) lembrou dunha sesión á seguinte —, e
nunca menos que a altura mínima da imaxe, que a táboa cede se fai falta.
Cando a película se vai, o botón recupera o mesmo lugar.

**O nome do ficheiro, nunca a súa ruta.** Unha ruta de douscentos caracteres
botaría todo o demais fóra da barra; a ruta completa é a que mostrou o
diálogo.

## `ffmpeg` non é obrigatorio

`subedit-gui` consulta `ffprobe` — distribuído con `ffmpeg` — para unha cousa e só
unha: **a taxa de fotogramas que declara o contedor**. Lese unha vez
por película, cando se asocia.

| Con `ffprobe` | Sen el |
| :------------- | :------ |
| a barra de estado escribe a taxa de fotogramas ao lado do nome da película | só escribe o nome |
| o diálogo de conversión propón a taxa de fotogramas da película no campo inferior | non propón nada, e ábrese como antes |
| o diálogo de aliñamento ábrese nela | ábrese na taxa de fotogramas do proxecto |

**Nada máis depende del, e ningunha operación se nega.** O reprodutor, pola súa parte,
non necesita `ffprobe` en absoluto: apóiase en `libmpv`, que é unha
dependencia de compilación. A duración da película, a que se usa para indicar o que
pasa do final, vén do reprodutor e non de `ffprobe`.

Non é polo tanto un modo degradado senón **o funcionamento normal** dunha máquina
que nunca instalou `ffmpeg`: o dato vén do usuario, e `ffprobe`
propóno cando está presente.

Unha taxa de fotogramas lida é un **racional exacto** — `24000/1001`, non `23.976`.
Esa é a razón pola que se conserva `ffprobe` para esa resposta: o
reprodutor sabe o mesmo como número en coma flotante, e unha taxa de fotogramas aproximada
recalcularía os tempos de todo o ficheiro lixeiramente desviados.
