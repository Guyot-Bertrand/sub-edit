# Instalar as ferramentas de transcrición

<!-- machine-translated -->
> Esta páxina é unha tradución automática e aínda non a revisou ningún lector da lingua.

Transcribir son a subtítulos — [`transcribe`](transcribe.md) na liña de ordes,
[`Transcribe Audio…`](../subedit-gui/video.md#transcribir-o-son) na xanela — necesita tres
cousas que **subedit non inclúe nin descarga**: un motor, un modelo para ese motor e
`ffmpeg`. Vostede instálaas e indica a subedit onde está o modelo.

**Nada se rompe sen elas.** O resto do programa funciona como antes; só a transcrición
se desactiva, cunha mensaxe que apunta aquí.

## O motor: whisper.cpp

[whisper.cpp](https://github.com/ggml-org/whisper.cpp) é o motor que executa subedit. É un programa,
`whisper-cli`, que funciona nun procesador normal — unha tarxeta gráfica só o fai máis rápido. Compílese
como di a súa propia documentación (abonda cun compilador de C++ e CMake):

```bash
git clone https://github.com/ggml-org/whisper.cpp
cd whisper.cpp
cmake -B build -DBUILD_SHARED_LIBS=OFF
cmake --build build -j --config Release
```

O programa é entón `build/bin/whisper-cli`. Compilado con `-DBUILD_SHARED_LIBS=OFF`, como máis arriba, é un único ficheiro que non necesita ningunha biblioteca ao seu carón, polo que pode copialo onde queira. Póñao onde o atope o seu `PATH`, ou **indique a súa ruta** a subedit co axuste `transcription.executable` — na xanela, as [preferencias](../subedit-gui/preferences.md#transcrición) teñen un campo para iso. Unha compilación para tarxeta gráfica engade unha opción á primeira liña de `cmake`: `-DGGML_CUDA=1` para unha tarxeta NVIDIA, `-DGGML_VULKAN=1` para a maioría das demais; subedit executa o que vostede instalase e non escolle nada por vostede.

Para conservar só o programa, **instáleo** e elimine o clon:

```bash
cmake --install build --prefix ~/.local
```

`--prefix` indica onde. `~/.local` non esixe dereitos de administrador, e o seu directorio `bin` debe estar no seu `PATH`; sen a opción, o valor por defecto é `/usr/local`, que pide `sudo`. `whisper-cli` queda no directorio `bin` ese prefixo, ao carón de ferramentas que subedit non usa; unha vez alí, o clon e o seu directorio `build` poden marchar.

## Un modelo

O motor le un **modelo**, un ficheiro de varios centos de megabytes a uns poucos gigabytes que
descarga unha vez. Os modelos oficiais están listados en
[huggingface.co/ggerganov/whisper.cpp](https://huggingface.co/ggerganov/whisper.cpp), e o
repositorio do motor ten un script que descarga un:

```bash
sh ./models/download-ggml-model.sh small
```

Despois indique a súa ruta con `--model` na liña de ordes, ou co axuste `transcription.model`,
ou nas [preferencias](../subedit-gui/preferences.md#transcrición) da xanela, que a lembran.

### Que modelo para que lingua

Whisper non cambia de modelo unha vez atopada a lingua: **o modelo que lle dá decide o que
pode facer.**

| Modelo | Linguas | Detecta a lingua | Tamaño |
| :---- | :-------- | :------------------- | :--- |
| `tiny`, `base`, `small`, `medium`, `large-v3`, `large-v3-turbo` | unha centena | si | desde uns 75 MB ata uns 3 GB |
| `tiny.en`, `base.en`, `small.en`, `medium.en` | só inglés | non | o mesmo tamaño ca os de arriba |

- **Para calquera cousa que non sexa inglés, tome un modelo multilingüe** — un cuxo nome non leve `.en`. Un modelo
  só de inglés rexeita outra lingua: a orde dío antes de facer calquera traballo.
- **Os modelos máis pequenos son claramente peores fóra do inglés.** Para francés, alemán, español e
  similares, `small` é un mínimo, e `medium` ou `large-v3-turbo` dan resultados moito mellores a costa
  de máis memoria e máis tempo.
- **A detección da lingua só é tan boa como o modelo.** Un modelo pequeno pode equivocarse cun comezo feito de
  música ou silencio; diga a lingua con `--language` cando a saiba.

## ffmpeg

[`ffmpeg`](https://ffmpeg.org/download.html) extrae a pista de son dun vídeo, e `ffprobe`, que vén
con el, lista as pistas. Instáleo co xestor de paquetes da súa distribución. Sen el, un ficheiro de
son que o motor le tal cal (`wav`, `mp3`, `flac`, `ogg`) aínda se pode transcribir, pero un vídeo
non.

## Comprobar que funciona

```bash
subedit-cli transcribe --model /path/to/ggml-small.bin --output film.srt film.mkv
```

Se falta algo, a mensaxe di o que, e apunta a esta páxina.
