# subedit-cli — manual

<!-- machine-translated -->
> Esta páxina é unha tradución automática e aínda non a revisou ningún lector da lingua.

A ferramenta de liña de ordes de subedit: traballar con ficheiros de subtítulos sen interface
gráfica, por lotes ou desde un script. Para o outro programa e para saber
por onde comezar, véxase
[o manual](../index.md).

> **Estado actual.** Existen as dezasete subordes de `subedit-cli`: inspeccionar,
> converter, desprazar, transformar, recalcular os tempos para outra taxa de fotogramas, aliñar a unha grade de fotogramas, eliminar
> as mencións para persoas xordas, axustar as duracións, substituír un texto, cambiar as maiúsculas,
> a cursiva e os trazos de diálogo, volver poñer os subtítulos en orde, corrixir os
> textos cos padróns de Gaupol, poñer ficheiros uns detrás doutros, cortar un
> ficheiro en dous, e escribir unha tradución nas posicións do seu ficheiro principal. Este manual
> describe o que existe, nunca o que está previsto; o que vén despois está na
> [folla de ruta](../../../../feuille-de-route.md).

## Seccións

| Sección | Contido |
| :------ | :------ |
| [Instalación](installation.md) | compilar e instalar a ferramenta |
| [Invocación](invocation.md) | opcións globais, saídas, códigos de saída |
| [O ficheiro de configuración](invocation.md#o-ficheiro-de-configuración) | o que `adjust` e `correct` len do ficheiro de configuración da xanela, `--config`, `--no-config` |
| [Procesar unha árbore](lots.md) | varios ficheiros, un directorio enteiro, `--recursive` |
| [`inspect`](inspect.md) | indicar de que está feito un ficheiro |
| [`convert`](convert.md) | escribir un ficheiro noutro formato ou noutra forma |
| [`shift`](shift.md) | desprazar todas as posicións na mesma cantidade |
| [`transform`](transform.md) | corrixir todas as posicións a partir de dous puntos que se sabe que son correctos |
| [`framerate`](framerate.md) | recalcular os tempos dun ficheiro dunha taxa de fotogramas a outra |
| [`snap`](snap.md) | volver poñer as marcas de tempo nos fotogramas dunha taxa de fotogramas |
| [`hearing-impaired`](hearing-impaired.md) | eliminar as mencións para persoas xordas |
| [`adjust`](adjust.md) | axustar as duracións a unha velocidade de lectura e a uns límites |
| [`replace`](replace.md) | substituír un texto sen romper ningunha etiqueta |
| [`case`](case.md) | poñer os textos en maiúsculas de título, de frase, en maiúsculas ou en minúsculas |
| [`italics`](italics.md) | poñer os textos en cursiva, ou quitarlles a cursiva |
| [`dialogue-dashes`](dialogue-dashes.md) | poñer os trazos de diálogo, ou quitalos |
| [`sort`](sort.md) | volver poñer os subtítulos na orde do seu inicio |
| [`join-repeats`](join-repeats.md) | facer un só subtítulo cos anacos dun mesmo texto que se suceden |
| [`disjoint`](disjoint.md) | cortar os subtítulos que se solapan para que ningunha franxa se solape xa con outra |
| [`repair`](repair.md) | encadear `join-repeats`, `sort` e `disjoint` nun ficheiro mal segmentado, desordenado e con solapamentos |
| [`correct`](correct.md) | corrixir os textos cos padróns de Gaupol: mencións, unión e división de palabras, erros comúns, maiúsculas, salto de liña |
| [`append`](append.md) | poñer ficheiros despois do primeiro, nun único ficheiro |
| [`split-file`](split-file.md) | cortar un ficheiro en dous, coa cola levada de volta á orixe |
| [`pair`](pair.md) | escribir unha tradución nas posicións do seu ficheiro principal, e dicir como se aliñaron as súas liñas |
| [`transcribe`](transcribe.md) | escribir a fala dun vídeo ou dun ficheiro de son como subtítulos |
| [Instalar as ferramentas de transcrición](install-transcription.md) | o motor, o modelo e `ffmpeg`, que o programa non inclúe |
| [`config`](config.md) | enumerar, mostrar e cambiar a configuración das operacións compartida coa xanela |
