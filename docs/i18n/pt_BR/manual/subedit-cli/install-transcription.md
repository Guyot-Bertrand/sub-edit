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
cmake -B build -DBUILD_SHARED_LIBS=OFF
cmake --build build -j --config Release
```

O programa é então `build/bin/whisper-cli`. Compilado com `-DBUILD_SHARED_LIBS=OFF`, como acima, é um único arquivo que não precisa de nenhuma biblioteca ao lado, de modo que você pode copiá-lo para onde quiser. Coloque-o onde seu `PATH` o encontre, ou **indique o seu caminho** ao subedit com a configuração `transcription.executable` — na janela, as [preferências](../subedit-gui/preferences.md#transcrição) têm um campo para isso. Uma compilação para placa gráfica acrescenta uma opção à primeira linha de `cmake`: `-DGGML_CUDA=1` para uma placa NVIDIA, `-DGGML_VULKAN=1` para a maioria das outras; o subedit executa o que você instalou e não escolhe nada por você.

Para guardar apenas o programa, **instale-o** e apague o clone:

```bash
cmake --install build --prefix ~/.local
```

`--prefix` diz onde. `~/.local` não exige direitos de administrador, e o seu diretório `bin` precisa estar no seu `PATH`; sem a opção, o padrão é `/usr/local`, que pede `sudo`. O `whisper-cli` fica no diretório `bin` desse prefixo, ao lado de ferramentas que o subedit não usa; depois que ele estiver lá, o clone e o seu diretório `build` podem ir embora.

## Um modelo

O motor lê um **modelo**, um arquivo de várias centenas de megabytes a alguns gigabytes que
baixa uma vez. Os modelos oficiais estão listados em
[huggingface.co/ggerganov/whisper.cpp](https://huggingface.co/ggerganov/whisper.cpp), e o
repositório do motor tem um script que obtém um:

```bash
sh ./models/download-ggml-model.sh small
```

Depois indique o seu caminho com `--model` na linha de comando, ou com a configuração `transcription.model`,
ou nas [preferências](../subedit-gui/preferences.md#transcrição) da janela, que o memorizam.

### Um modelo de atividade de voz, para começar onde a fala começa

Opcionalmente, um **modelo de atividade de voz** (Silero) permite que o motor ignore os silêncios antes de decodificar. Sem ele, o whisper começa sua primeira legenda em 0, seja o que for que venha primeiro no filme, música incluída, e arredonda seus tempos para janelas inteiras; com ele, a primeira legenda começa onde a fala começa e as outras a seguem. O repositório do motor tem um script que obtém um:

```bash
sh ./models/download-vad-model.sh silero-v6.2.0
```

Indique o seu caminho com `--vad-model`, com a configuração `transcription.vad-model` ou nas [preferências](../subedit-gui/preferences.md#transcrição) da janela.

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
