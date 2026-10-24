# Instalar as ferramentas de transcrição

<!-- machine-translated -->
> Esta página é uma tradução automática e ainda não foi revisada por um leitor do idioma.

Transcrever áudio em legendas — [`transcribe`](transcribe.md) na linha de comando,
[`Transcribe Audio…`](../subedit-gui/video.md#transcrever-o-áudio) na janela — precisa de três
coisas que o **subedit não fornece nem baixa**: um motor, um modelo para esse motor e o
`ffmpeg`. Você os instala e indica ao subedit onde está o modelo.

**Sem elas, nada se estraga.** O resto do programa funciona como antes; só a transcrição
se desativa, com uma mensagem que remete para aqui.

## O motor: whisper.cpp

O [whisper.cpp](https://github.com/ggml-org/whisper.cpp) é o motor que o subedit executa. É um programa,
`whisper-cli`, que roda num processador normal — uma placa gráfica apenas o torna mais rápido. Compile-o
como diz a sua própria documentação (basta um compilador de C++ e o CMake):

```bash
git clone https://github.com/ggml-org/whisper.cpp
cd whisper.cpp
cmake -B build
cmake --build build -j --config Release
```

O programa é então `build/bin/whisper-cli`. Coloque-o onde seu `PATH` o encontre, ou **indique o seu
caminho** ao subedit com a configuração `transcription.executable`. Uma compilação para placa gráfica se faz
com as opções que a sua documentação indica; o subedit executa o que instalou e não escolhe nada
por si.

## Um modelo

O motor lê um **modelo**, um arquivo de várias centenas de megabytes a alguns gigabytes que
baixa uma vez. Os modelos oficiais estão listados em
[huggingface.co/ggerganov/whisper.cpp](https://huggingface.co/ggerganov/whisper.cpp), e o
repositório do motor tem um script que obtém um:

```bash
sh ./models/download-ggml-model.sh small
```

Depois indique o seu caminho com `--model` na linha de comando, ou com a configuração `transcription.model`,
ou na caixa de diálogo da janela, que o memoriza.

### Que modelo para que idioma

O Whisper não muda de modelo depois de encontrar o idioma: **o modelo que indica decide o que ele
pode fazer.**

| Modelo | Idiomas | Deteta o idioma | Tamanho |
| :---- | :-------- | :------------------- | :--- |
| `tiny`, `base`, `small`, `medium`, `large-v3`, `large-v3-turbo` | cerca de cem | sim | de cerca de 75 MB a cerca de 3 GB |
| `tiny.en`, `base.en`, `small.en`, `medium.en` | só inglês | não | igual ao mesmo tamanho acima |

- **Para tudo o que não seja inglês, escolha um modelo multilingue** — um cujo nome não tenha `.en`. Um
  modelo só de inglês recusa outro idioma: o comando diz isso antes de fazer qualquer trabalho.
- **Os modelos menores são claramente piores fora do inglês.** Para francês, alemão, espanhol
  e afins, `small` é um mínimo, e `medium` ou `large-v3-turbo` dão resultados muito melhores ao custo
  de mais memória e mais tempo.
- **A detecção do idioma vale o que vale o modelo.** Um modelo pequeno pode se enganar num início feito de
  música ou de silêncio; indique o idioma com `--language` quando o souber.

## ffmpeg

O [`ffmpeg`](https://ffmpeg.org/download.html) extrai a faixa de áudio de um vídeo, e o `ffprobe`, que vem
com ele, lista as faixas. Instale-o com o gerenciador de pacotes da sua distribuição. Sem ele, um arquivo de
áudio que o motor lê tal como está (`wav`, `mp3`, `flac`, `ogg`) ainda pode ser transcrito, mas um vídeo
não.

## Verificar que funciona

```bash
subedit-cli transcribe --model /path/to/ggml-small.bin --output film.srt film.mkv
```

Se faltar alguma coisa, a mensagem diz o quê e remete para esta página.
