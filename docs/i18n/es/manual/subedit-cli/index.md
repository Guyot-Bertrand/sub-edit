# subedit-cli — manual

<!-- machine-translated -->
> Esta página es una traducción automática y todavía no la ha revisado ningún lector del idioma.

La herramienta de línea de comandos de subedit: trabajar con archivos de subtítulos sin interfaz
gráfica, por lotes o desde un script. Para el otro programa y para saber
por dónde empezar, véase
[el manual](../index.md).

> **Estado actual.** Los diecisiete subcomandos de `subedit-cli` existen: inspeccionar,
> convertir, desplazar, transformar, reajustar los tiempos a otra velocidad de fotogramas, alinear a una rejilla de fotogramas, eliminar
> las anotaciones para sordos, ajustar las duraciones, reemplazar un texto, cambiar la capitalización,
> la cursiva y las rayas de diálogo, volver a poner en orden los subtítulos, corregir los
> textos con los patrones de Gaupol, poner archivos uno tras otro, cortar un
> archivo en dos y escribir una traducción en las posiciones de su archivo principal. Este manual
> describe lo que existe, nunca lo que está previsto; lo que viene después está en
> la [hoja de ruta](../../../../feuille-de-route.md).

## Secciones

| Sección | Contenido |
| :------ | :------ |
| [Instalación](installation.md) | compilar e instalar la herramienta |
| [Invocación](invocation.md) | opciones globales, salidas, códigos de salida |
| [El archivo de configuración](invocation.md#el-archivo-de-configuración) | lo que `adjust` y `correct` leen del archivo de configuración de la ventana, `--config`, `--no-config` |
| [Procesar un árbol](lots.md) | varios archivos, un directorio entero, `--recursive` |
| [`inspect`](inspect.md) | informar de qué está hecho un archivo |
| [`convert`](convert.md) | escribir un archivo en otro formato o con otra forma |
| [`shift`](shift.md) | mover todas las posiciones la misma cantidad |
| [`transform`](transform.md) | corregir todas las posiciones a partir de dos puntos que se saben correctos |
| [`framerate`](framerate.md) | reajustar los tiempos de un archivo de una velocidad de fotogramas a otra |
| [`snap`](snap.md) | volver a poner las marcas de tiempo sobre los fotogramas de una velocidad de fotogramas |
| [`hearing-impaired`](hearing-impaired.md) | eliminar las anotaciones para sordos |
| [`adjust`](adjust.md) | ajustar las duraciones a una velocidad de lectura y a unos límites |
| [`replace`](replace.md) | reemplazar un texto sin romper una etiqueta |
| [`case`](case.md) | poner los textos en tipo título, tipo oración, mayúsculas o minúsculas |
| [`italics`](italics.md) | poner los textos en cursiva, o quitarles la cursiva |
| [`dialogue-dashes`](dialogue-dashes.md) | poner las rayas de diálogo, o quitarlas |
| [`sort`](sort.md) | volver a poner los subtítulos en el orden de su inicio |
| [`join-repeats`](join-repeats.md) | hacer un solo subtítulo con los trozos de un mismo texto que se suceden |
| [`disjoint`](disjoint.md) | recortar los subtítulos solapados para que ningún intervalo se solape ya con otro |
| [`repair`](repair.md) | encadenar `join-repeats`, `sort` y `disjoint` sobre un archivo mal segmentado, desordenado y con solapamientos |
| [`correct`](correct.md) | corregir los textos con los patrones de Gaupol: anotaciones, unir y separar palabras, errores comunes, capitalización, salto de línea |
| [`append`](append.md) | poner archivos tras el primero, en un único archivo |
| [`split-file`](split-file.md) | cortar un archivo en dos, con la cola llevada de vuelta al origen |
| [`pair`](pair.md) | escribir una traducción en las posiciones de su archivo principal, e indicar cómo se emparejaron sus líneas |
| [`config`](config.md) | listar, mostrar y cambiar la configuración de las operaciones compartida con la ventana |
