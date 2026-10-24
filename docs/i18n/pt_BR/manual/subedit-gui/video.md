# O vídeo associado

<!-- machine-translated -->
> Esta página é uma tradução automática e ainda não foi revisada por um leitor do idioma.

O menu **Vídeo** traz primeiro três comandos, e os gestos que conduzem o filme
— veja [O reprodutor](player.md#navegar-pelo-filme).

| Comando | Atalho | O que faz |
| :------ | :----- | :-------- |
| Selecionar vídeo… | nenhum | escolhe o arquivo de vídeo ao qual o documento se associa |
| Transcrever áudio… | nenhum | transcreve o áudio do filme em legendas que substituem as do projeto — veja [Transcrever o áudio](#transcrever-o-áudio) |
| Reproduzir / Pausar | `Ctrl+P` | reproduz ou interrompe o vídeo — veja [O reprodutor](player.md#reproduzir-e-parar) |

O vídeo associado é **uma propriedade do documento aberto**, e não uma configuração
do aplicativo: abrir outro arquivo de legendas recomeça do zero. Ele
não é gravado em nenhum arquivo e desaparece com a janela.

## A faixa de áudio

Um filme pode ter várias faixas de som — a versão original, uma dublagem, um comentário. O
submenu **`Vídeo ▸ Áudio ▸ Idioma`** as lista, **uma entrada por faixa**, e **marca a que está tocando**:

| O que o arquivo fornece | O que a entrada diz |
| :---------------------- | :------------------ |
| um idioma e um título | `1: fra — Original` |
| um idioma apenas | `2: eng` |
| um título apenas | `3: Commentary` |
| nenhum dos dois | `1` |

O número é **o lugar da faixa no filme**, contado uma a uma, na ordem do arquivo: duas faixas
podem dizer `eng`, e um arquivo sem nenhum rótulo ainda tem uma primeira e uma segunda.

**Escolher uma entrada reproduz essa faixa.** A marca é a resposta do reprodutor, não o clique: ela indica a
faixa que está tocando.

- **O submenu fica desativado** sem vídeo ou sem faixa de áudio. Com uma única faixa, ele continua ativado e
  a mostra, marcada: indica o idioma do filme. Os dois gestos de volume, que ficam ao lado, continuam
  ativados em qualquer caso: veja [O volume](player.md#o-volume).
- **A faixa não é lembrada** de um vídeo para o seguinte, nem de uma sessão para a seguinte: um número de faixa é
  o do seu arquivo, e transportá-lo para o filme seguinte escolheria o que quer que tenha esse número ali.
  Voltar a uma aba reabre o filme com a faixa que o reprodutor escolhe por conta própria.
- Sem atalho: o idioma é trocado raramente, e o menu basta.

## Escolher um vídeo

A caixa de diálogo abre no diretório do arquivo de legendas e filtra pelas
extensões de vídeo reconhecidas:

    .avi .flv .m2ts .mkv .mov .mp4 .ogg .ogv .vob .webm

Ela também oferece mostrar todos os arquivos. Cancelar a caixa de diálogo não muda
nada.

O arquivo escolhido é **aberto imediatamente**: a imagem aparece na janela,
em pausa no seu primeiro instante. Veja [O reprodutor](player.md).

**Soltar um arquivo de vídeo na janela faz a mesma coisa** — veja
[Soltar arquivos na janela](files.md#soltar-arquivos-na-janela).

**O vídeo é o do projeto da aba atual.** Cada aba guarda o
seu; trocar de aba troca o filme. **Voltar a uma aba retoma o seu
filme de onde foi deixado**, em pausa, como em qualquer abertura. Outro filme
escolhido para esse projeto nesse meio-tempo começa, por sua vez, do início.

## A proposta automática

Quando um arquivo de legendas é aberto, **se nenhum vídeo foi escolhido**,
a janela procura um no mesmo diretório: um arquivo de vídeo cujo nome
sem a extensão é um prefixo do nosso, até um ponto.

| Arquivo de legendas | Vídeo encontrado | Por quê |
| :------------------ | :--------------- | :------ |
| `film.fr.srt` | `film.mkv` | `film` é um prefixo de `film.fr` |
| `film.en.forced.srt` | `film.mkv` | idem, qualquer que seja o número de segmentos |
| `film.fr.srt` | `film.fr.mkv` em vez de `film.mkv` | o nome mais longo vence |
| `film.fr.srt` | nenhum, se `film.mkv` **e** `film.mp4` estiverem presentes | dois candidatos não podem ser distinguidos |
| `film.fr.srt` | nenhum, se só `fil.mkv` estiver presente | `fil` não termina num ponto |

**Uma proposta não é uma escolha.** Ela se deixa substituir por outra
proposta — abrir outro arquivo, salvar com outro nome. Uma escolha
explícita, por outro lado, nunca é sobrescrita: depois de usar `Select Video…`, a
proposta automática não volta mais atrás dela.

É isso que torna aceitável o palpite: ele pode ser corrigido, e sua
correção se mantém.

## O que a barra de status mostra

À direita da barra de status, uma linha diz a que o documento se associa. Ela
é a última de três: a [codificação](files.md#a-codificação-na-barra-de-status)
do arquivo lido vem primeiro, a [grade de quadros](frame-grid.md) deduzida das
posições vem em seguida.

| Situação | O que está escrito |
| :------- | :----------------- |
| um vídeo está associado | `Video: film.mkv` |
| … e `ffprobe` fornece sua taxa de quadros | `Video: film.mkv, 24000/1001 fps` |
| nenhum | `No video` |

**A taxa de quadros acompanha o filme em vez de ficar ao lado dele.** São um único fato — a que
este documento se associa — e uma terceira menção a colocaria no mesmo nível que
a [grade deduzida das posições](frame-grid.md), que é outra coisa.

## Transcrever o áudio

**`Transcribe Audio…`** escreve a fala do filme como legendas. Fica desativado enquanto não houver um filme
aberto: é o áudio do **filme do projeto**, nunca de um arquivo que se escolheria — para transcrever um
arquivo de áudio, use [`transcribe`](../subedit-cli/transcribe.md) na linha de comando.

A caixa de diálogo pede três coisas, e diz se pode começar:

- **a faixa de áudio**, listada como o menu [`Audio`](#a-faixa-de-áudio) as lista, com o idioma
  e o título que o arquivo declara;
- **o idioma**, duas letras como `en` ou `fr`. Começa pelo idioma da faixa escolhida
  quando o arquivo declara um, acompanha a faixa enquanto você não tiver digitado um, e
  **uma linha sob o campo diz de onde vem**. Quando o arquivo não declara nenhum idioma para a faixa,
  a linha diz isso: o modelo vai então detectá-lo a partir do som, o que só um modelo multilíngue
  faz, e você deve digitá-lo se o conhecer. Com um modelo só para inglês, que não detecta nada,
  a linha diz que o idioma precisa ser digitado;
- **a parte do filme**, de um tempo a outro — veja [Experimentar primeiro uma parte](#experimentar-primeiro-uma-parte). Os dois campos
  vazios querem dizer o filme inteiro.

**Diz também o que falta.** Três linhas dizem se o motor, o `ffmpeg` e o modelo estão presentes, e
onde; quando um não está, `OK` fica desativado, e a caixa diz isso com o link para
[Instalar as ferramentas de transcrição](../subedit-cli/install-transcription.md). Um botão `Preferences…` abre
as [preferências](preferences.md#transcrição), onde se definem o programa e o modelo — são fatos
sobre a máquina, não escolhas a refazer a cada transcrição — e a caixa verifica de novo quando se fecham.

O idioma é guardado para a vez seguinte. O próprio motor e os limites das legendas são
[configurações](preferences.md) (`transcription.*`).

**Quando o projeto já contém legendas, a janela pergunta antes de as substituir**, e diz
quantas se perdem. Pergunta antes do trabalho, para que recusar não custe nada. Quando o projeto está vazio, não há
nada a perguntar.

O trabalho roda **em segundo plano**, com uma caixa e um botão `Cancel`; a janela continua utilizável ao redor. **A caixa
diz em que ponto está o trabalho**, para que você saiba que está vivo e quanto tempo vai demorar:

- **a etapa**: primeiro “Extraindo o áudio…” — a caixa fica então ocupada, sem valor, porque extrair não diz nada sobre o progresso —,
  depois “Transcrevendo o áudio…”, com uma barra;
- **a posição**: “0:12:30 de 0:50:00”, até onde no filme o motor chegou, assim que produziu o seu primeiro texto;
- **o tempo que falta**, “faltam cerca de 38 min”: ausente no início, mostrado quando há trabalho feito que chegue para extrapolar, e
  calculado a partir da velocidade do último minuto; é uma precisão de alguns minutos, não uma promessa;
- **o último texto** obtido, para que você possa ver que o trabalho faz sentido.

Num filme longo com um modelo grande num processador, os primeiros minutos podem ser longos: o motor carrega o modelo e trabalha nas suas primeiras
janelas de trinta segundos. A posição e o último texto se mexem antes da porcentagem própria do motor, que só vem em passos
de 5 % do filme. Quando termina, as legendas do projeto são substituídas
pela transcrição **em uma só operação**: um `Undo` devolve o que lá estava, incluindo os textos de
tradução. Uma transcrição que falha, ou que você cancela, não altera nada — o motivo é
dito numa caixa, exceto em um cancelamento, que apenas diz na barra de status que foi cancelado.

### Experimentar primeiro uma parte

Um intervalo faz da transcrição uma **experiência**, para avaliar um modelo ou um idioma em dois minutos antes de se comprometer com
duas horas. Os campos “Parte do filme” recebem um início e um fim, cada um escrito `2:00`, `1:02:03`, `2:00.5` ou um número de
segundos, `90`; um só é permitido (dali até o fim, ou do início até ali). `OK` fica desativado enquanto um campo não
for um tempo ou o fim não vier depois do início, e uma linha sob os campos diz qual.

- **As legendas mantêm o tempo do filme**: o que é dito aos 2:10 fica em `0:02:10` no resultado.
- **Nada é substituído e nada é perguntado.** O resultado se abre como **um projeto próprio**, numa nova aba, sem
  arquivo; o projeto de onde vem, e as legendas dele, ficam como estavam. Salve-o, ou feche-o.

**O que ela precisa não é fornecido**: um motor (whisper.cpp), um modelo e o `ffmpeg` para extrair o áudio.
Veja [Instalar as ferramentas de transcrição](../subedit-cli/install-transcription.md).

## Quando não há filme

A janela mostra um botão **`Select Video…`** onde ficaria a imagem, em vez de
nada. É o mesmo comando do menu `Video`, ao
alcance do mouse: uma ausência sobre a qual não se pode agir é difícil de distinguir de uma
falha.

O botão desaparece assim que um filme é aberto, e a imagem ocupa **todo** o seu
lugar: a altura que a tabela deixa acima dela — a que
[`window.table-share`](preferences.md) lembrou de uma sessão para a seguinte —, e
nunca menos que a altura mínima da imagem, que a tabela cede se necessário.
Quando o filme sai, o botão retoma o mesmo lugar.

**O nome do arquivo, nunca o seu caminho.** Um caminho de duzentos caracteres
empurraria todo o resto para fora da barra; o caminho completo é o que a caixa de
diálogo mostrou.

## `ffmpeg` não é obrigatório

O `subedit-gui` consulta o `ffprobe` — fornecido com o `ffmpeg` — para uma única
coisa: **a taxa de quadros que o contêiner declara**. Ela é lida uma vez
por filme, quando ele é associado.

| Com `ffprobe` | Sem |
| :------------ | :-- |
| a barra de status escreve a taxa de quadros ao lado do nome do filme | ela escreve apenas o nome |
| a caixa de diálogo de conversão propõe a taxa de quadros do filme no campo inferior | ela não propõe nada, e abre como antes |
| a caixa de diálogo de alinhamento abre nela | ela abre na taxa de quadros do projeto |

**Nada mais depende dele, e nenhuma operação se recusa.** O reprodutor, por sua vez,
não precisa do `ffprobe` de forma alguma: ele se apoia na `libmpv`, que é uma
dependência de compilação. A duração do filme, a que serve para relatar o que
ultrapassa o fim, vem do reprodutor e não do `ffprobe`.

**A transcrição é a única coisa que precisa do `ffmpeg`**: ele extrai o áudio do filme. Veja
[Instalar as ferramentas de transcrição](../subedit-cli/install-transcription.md).

Não é portanto um modo degradado, mas **o funcionamento normal** de uma máquina
que nunca instalou o `ffmpeg`: o dado vem do usuário, e o `ffprobe`
o propõe quando está presente.

Uma taxa de quadros lida é um **racional exato** — `24000/1001`, e não `23.976`.
É essa a razão de o `ffprobe` ser mantido para essa resposta: o
reprodutor conhece a mesma coisa como número de ponto flutuante, e uma taxa de quadros aproximada
ressincronizaria o arquivo inteiro um pouco fora do lugar.
