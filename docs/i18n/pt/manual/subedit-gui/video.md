# O vídeo associado

<!-- machine-translated -->
> Esta página é uma tradução automática e ainda não foi revista por nenhum leitor do idioma.

O menu **Vídeo** traz primeiro três comandos, e depois os gestos que conduzem o filme
— ver [O reprodutor](player.md#percorrer-o-filme).

| Comando | Atalho | O que faz |
| :------ | :----- | :-------- |
| Selecionar vídeo… | nenhum | escolhe o ficheiro de vídeo a que o documento está associado |
| Transcrever áudio… | nenhum | transcreve o áudio do filme em legendas que substituem as do projeto — ver [Transcrever o áudio](#transcrever-o-áudio) |
| Reproduzir / Pausa | `Ctrl+P` | reproduz ou pára o vídeo — ver [O reprodutor](player.md#reproduzir-e-parar) |

O vídeo associado é **uma propriedade do documento aberto**, não uma definição
da aplicação: abrir outro ficheiro de legendas recomeça do zero. Não
é escrito em nenhum ficheiro e desaparece com a janela.

## A faixa de áudio

Um filme pode levar várias faixas de som — a versão original, uma dobragem, um comentário. O
submenu **`Vídeo ▸ Áudio ▸ Idioma`** enumera-as, **uma entrada por faixa**, e **marca a que está a ser reproduzida**:

| O que o ficheiro dá | O que a entrada diz |
| :------------------ | :------------------ |
| um idioma e um título | `1: fra — Original` |
| um idioma sozinho | `2: eng` |
| um título sozinho | `3: Commentary` |
| nenhum dos dois | `1` |

O número é **o lugar da faixa no filme**, uma a uma, pela ordem do ficheiro: duas faixas
podem dizer `eng`, e um ficheiro mudo quanto às suas etiquetas tem na mesma uma primeira e uma segunda.

**Escolher uma entrada reproduz essa faixa.** A marca é a resposta do reprodutor, não o clique: indica a
faixa que está a ser reproduzida.

- **O submenu fica desativado** sem vídeo ou sem faixa de áudio. Com uma única faixa, continua ativado e
  mostra-a, assinalada: diz o idioma do filme. Os dois gestos de volume, que estão ao lado, ficam
  ativados em qualquer caso: ver [O volume](player.md#o-volume).
- **A faixa não é memorizada** de um vídeo para o seguinte, nem de uma sessão para a seguinte: um número de faixa é
  o do seu ficheiro, e transpô-lo para o filme seguinte escolheria o que tivesse esse número nesse filme.
  Voltar a um separador reabre o seu filme na faixa que o reprodutor escolhe sozinho.
- Sem atalho: muda-se de idioma raramente, e o menu basta.

## Escolher um vídeo

A caixa de diálogo abre-se na pasta do ficheiro de legendas e filtra pelas
extensões de vídeo reconhecidas:

    .avi .flv .m2ts .mkv .mov .mp4 .ogg .ogv .vob .webm

Oferece também a possibilidade de mostrar todos os ficheiros. Cancelar a caixa de diálogo não altera
nada.

O ficheiro escolhido é **aberto de imediato**: a imagem aparece na janela,
em pausa no seu primeiro instante. Ver [O reprodutor](player.md).

**Largar um ficheiro de vídeo na janela faz o mesmo** — ver
[Largar ficheiros na janela](files.md#largar-ficheiros-na-janela).

**O vídeo é o do projeto do separador atual.** Cada separador guarda
o seu; mudar de separador muda o filme. **Voltar a um separador retoma o seu
filme onde o deixou**, em pausa, como em qualquer abertura. Outro filme
escolhido entretanto para esse projeto começa, por seu lado, do princípio.

## A proposta automática

Quando se abre um ficheiro de legendas, **se nenhum vídeo foi escolhido**,
a janela procura um na mesma pasta: um ficheiro de vídeo cujo nome
sem extensão seja um prefixo do nosso, até um ponto.

| Ficheiro de legendas | Vídeo encontrado | Porquê |
| :------------------- | :--------------- | :----- |
| `film.fr.srt` | `film.mkv` | `film` é um prefixo de `film.fr` |
| `film.en.forced.srt` | `film.mkv` | idem, seja qual for o número de segmentos |
| `film.fr.srt` | `film.fr.mkv` em vez de `film.mkv` | o nome mais longo ganha |
| `film.fr.srt` | nenhum, se `film.mkv` **e** `film.mp4` existirem | não se consegue distinguir dois candidatos |
| `film.fr.srt` | nenhum, se só existir `fil.mkv` | `fil` não pára num ponto |

**Uma proposta não é uma escolha.** Deixa-se substituir por outra
proposta — abrir outro ficheiro, guardar com outro nome. Uma escolha
explícita, pelo contrário, nunca é sobrescrita: depois de usado `Select Video…`, a
proposta automática já não volta atrás dela.

É isso que torna aceitável a suposição: pode ser corrigida, e a sua
correção mantém-se.

## O que a barra de estado mostra

À direita da barra de estado, uma linha diz a que o documento está associado. É
a última de três: a [codificação](files.md#a-codificação-na-barra-de-estado)
do ficheiro lido vem primeiro, a [grelha de fotogramas](frame-grid.md) deduzida das
posições a seguir.

| Situação | O que é escrito |
| :------- | :-------------- |
| um vídeo está associado | `Video: film.mkv` |
| … e `ffprobe` dá a sua taxa de fotogramas | `Video: film.mkv, 24000/1001 fps` |
| nenhum | `No video` |

**A taxa de fotogramas acompanha o filme em vez de ficar ao lado.** São um único facto — aquilo a que
este documento está associado — e uma terceira menção colocá-la-ia ao mesmo nível que
a [grelha deduzida das posições](frame-grid.md), que é outra coisa.

## Transcrever o áudio

**`Transcribe Audio…`** escreve a fala do filme como legendas. Fica desativado enquanto não houver um filme
aberto: é o áudio do **filme do projeto**, nunca de um ficheiro que se escolheria — para transcrever um
ficheiro de áudio, use [`transcribe`](../subedit-cli/transcribe.md) na linha de comandos.

A caixa de diálogo pede três coisas:

- **a faixa de áudio**, listada como o menu [`Audio`](#a-faixa-de-áudio) as lista, com o idioma
  e o título que o ficheiro declara;
- **o idioma**, duas letras como `en` ou `fr`. Começa pelo idioma da faixa escolhida
  quando o ficheiro declara um, acompanha a faixa enquanto não tiver digitado um, e
  deixada vazia faz o modelo **detetar** o idioma;
- **o modelo**, um ficheiro, com um botão `Browse…`.

As escolhas são guardadas para a vez seguinte. O próprio motor e os limites das legendas são
[definições](preferences.md) (`transcription.*`).

**Quando o projeto já contém legendas, a janela pergunta antes de as substituir**, e diz
quantas se perdem. Pergunta antes do trabalho, para que recusar não custe nada. Quando o projeto está vazio, não há
nada a perguntar.

O trabalho decorre **em segundo plano**, com uma caixa que mostra até onde o motor chegou e um botão `Cancel`;
a janela continua utilizável à volta. Quando termina, as legendas do projeto são substituídas
pela transcrição **numa só operação**: um `Undo` devolve o que lá estava, incluindo os textos de
tradução. Uma transcrição que falha, ou que cancela, não altera nada — o motivo é
dito numa caixa, exceto num cancelamento, que apenas diz na barra de estado que foi cancelado.

**O que ela necessita não é fornecido**: um motor (whisper.cpp), um modelo e o `ffmpeg` para extrair o áudio.
Quando falta um, a caixa diz qual, e remete para
[Instalar as ferramentas de transcrição](../subedit-cli/install-transcription.md).

## Quando não há filme

A janela mostra um botão **`Select Video…`** onde estaria a imagem, em vez de
nada. É o mesmo comando que o do menu `Video`, ao
alcance do rato: uma ausência sobre a qual não se pode agir distingue-se mal de uma
falha.

O botão desaparece assim que um filme é aberto, e a imagem ocupa **todo** o seu
lugar: a altura que a tabela deixa acima dela — a que
[`window.table-share`](preferences.md) memorizou de uma sessão para a seguinte —, e
nunca menos do que a altura mínima da imagem, que a tabela cede se for preciso.
Quando o filme desaparece, o botão volta a ocupar o mesmo lugar.

**O nome do ficheiro, nunca o seu caminho.** Um caminho de duzentos carateres
empurraria tudo o resto para fora da barra; o caminho completo é o que a
caixa de diálogo mostrou.

## `ffmpeg` não é necessário

O `subedit-gui` consulta o `ffprobe` — fornecido com o `ffmpeg` — para uma única
coisa: **a taxa de fotogramas que o contentor declara**. É lida uma vez
por filme, quando este é associado.

| Com `ffprobe` | Sem |
| :------------ | :-- |
| a barra de estado escreve a taxa de fotogramas ao lado do nome do filme | escreve apenas o nome |
| a caixa de diálogo da conversão propõe a taxa de fotogramas do filme no campo inferior | não propõe nada, e abre-se como dantes |
| a caixa de diálogo do alinhamento abre-se nela | abre-se na taxa de fotogramas do projeto |

**Nada mais depende dele, e nenhuma operação recusa.** O reprodutor, por seu lado,
não precisa de `ffprobe` de todo: apoia-se na `libmpv`, que é uma
dependência de compilação. A duração do filme, a que serve para comunicar o que
passa do fim, vem do reprodutor e não do `ffprobe`.

**A transcrição é a única coisa que precisa do `ffmpeg`**: ele extrai o áudio do filme. Ver
[Instalar as ferramentas de transcrição](../subedit-cli/install-transcription.md).

Não se trata, portanto, de um modo degradado, mas do **funcionamento normal** de uma máquina
que nunca instalou o `ffmpeg`: o dado vem do utilizador, e o `ffprobe`
propõe-no quando lá está.

Uma taxa de fotogramas lida é um **racional exato** — `24000/1001`, não `23.976`.
É essa a razão por que o `ffprobe` é mantido para esta resposta: o
reprodutor conhece a mesma coisa como número de vírgula flutuante, e uma taxa de fotogramas aproximada
resincronizaria o ficheiro inteiro ligeiramente fora do sítio.
