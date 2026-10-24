# Instalar as ferramentas de transcrição

<!-- machine-translated -->
> Esta página é uma tradução automática e ainda não foi revista por nenhum leitor do idioma.

Transcrever áudio em legendas — [`transcribe`](transcribe.md) na linha de comandos,
[`Transcribe Audio…`](../subedit-gui/video.md#transcrever-o-áudio) na janela — precisa de três
coisas que o **subedit não fornece nem descarrega**: um motor, um modelo para esse motor e o
`ffmpeg`. Instala-os você e indica ao subedit onde está o modelo.

**Sem elas, nada se estraga.** O resto do programa funciona como antes; só a transcrição
se desativa, com uma mensagem que remete para aqui.

## O motor: whisper.cpp

O [whisper.cpp](https://github.com/ggml-org/whisper.cpp) é o motor que o subedit executa. É um programa,
`whisper-cli`, que corre num processador normal — uma placa gráfica apenas o torna mais rápido. Compile-o
como diz a sua própria documentação (basta um compilador de C++ e o CMake):

```bash
git clone https://github.com/ggml-org/whisper.cpp
cd whisper.cpp
cmake -B build -DBUILD_SHARED_LIBS=OFF
cmake --build build -j --config Release
```

O programa é então `build/bin/whisper-cli`. Compilado com `-DBUILD_SHARED_LIBS=OFF`, como acima, é um único ficheiro que não precisa de nenhuma biblioteca ao lado, pelo que o pode copiar para onde quiser. Coloque-o onde o seu `PATH` o encontre, ou **indique o seu caminho** ao subedit com a definição `transcription.executable` — na janela, as [preferências](../subedit-gui/preferences.md#transcrição) têm um campo para isso. Uma compilação para placa gráfica acrescenta uma opção à primeira linha de `cmake`: `-DGGML_CUDA=1` para uma placa NVIDIA, `-DGGML_VULKAN=1` para a maioria das outras; o subedit executa o que instalou e não escolhe nada por si.

Para guardar apenas o programa, **instale-o** e apague o clone:

```bash
cmake --install build --prefix ~/.local
```

`--prefix` diz onde. `~/.local` não exige direitos de administrador, e o seu diretório `bin` tem de estar no seu `PATH`; sem a opção, o valor predefinido é `/usr/local`, que pede `sudo`. O `whisper-cli` fica no diretório `bin` desse prefixo, ao lado de ferramentas que o subedit não usa; depois de lá estar, o clone e o seu diretório `build` podem sair.

## Um modelo

O motor lê um **modelo**, um ficheiro de várias centenas de megabytes a alguns gigabytes que
descarrega uma vez. Os modelos oficiais estão listados em
[huggingface.co/ggerganov/whisper.cpp](https://huggingface.co/ggerganov/whisper.cpp), e o
repositório do motor tem um script que obtém um:

```bash
sh ./models/download-ggml-model.sh small
```

Depois indique o seu caminho com `--model` na linha de comandos, ou com a definição `transcription.model`,
ou nas [preferências](../subedit-gui/preferences.md#transcrição) da janela, que o memorizam.

### Que modelo para que idioma

O Whisper não muda de modelo depois de encontrar o idioma: **o modelo que indica decide o que ele
pode fazer.**

| Modelo | Idiomas | Deteta o idioma | Tamanho |
| :---- | :-------- | :------------------- | :--- |
| `tiny`, `base`, `small`, `medium`, `large-v3`, `large-v3-turbo` | cerca de cem | sim | de cerca de 75 MB a cerca de 3 GB |
| `tiny.en`, `base.en`, `small.en`, `medium.en` | só inglês | não | igual ao mesmo tamanho acima |

- **Para tudo o que não seja inglês, escolha um modelo multilingue** — um cujo nome não tenha `.en`. Um
  modelo só de inglês recusa outro idioma: o comando diz-lo antes de fazer qualquer trabalho.
- **Os modelos mais pequenos são claramente piores fora do inglês.** Para francês, alemão, espanhol
  e afins, `small` é um mínimo, e `medium` ou `large-v3-turbo` dão resultados muito melhores ao preço
  de mais memória e mais tempo.
- **A deteção do idioma vale o que vale o modelo.** Um modelo pequeno pode enganar-se num início feito de
  música ou de silêncio; indique o idioma com `--language` quando o souber.

## ffmpeg

O [`ffmpeg`](https://ffmpeg.org/download.html) extrai a faixa de áudio de um vídeo, e o `ffprobe`, que vem
com ele, lista as faixas. Instale-o com o gestor de pacotes da sua distribuição. Sem ele, um ficheiro de
áudio que o motor lê tal como está (`wav`, `mp3`, `flac`, `ogg`) ainda pode ser transcrito, mas um vídeo
não.

## Verificar que funciona

```bash
subedit-cli transcribe --model /path/to/ggml-small.bin --output film.srt film.mkv
```

Se faltar alguma coisa, a mensagem diz o quê e remete para esta página.
