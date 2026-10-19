# Instalación

<!-- machine-translated -->
> Esta página es una traducción automática y todavía no la ha revisado ningún lector del idioma.

**Esta página vale para los dos programas** — `subedit-cli` y `subedit-gui`.
Se compilan y se instalan juntos.

Dos caminos: **un paquete nativo**, `.deb` o `.rpm`, o **compilar desde el
código fuente**, que luego se instala donde se quiera.

## Los paquetes

Ambos los produce CPack a partir de **las mismas reglas de instalación**:
ponen los mismos archivos en los mismos lugares, y solo difieren en los
nombres de los paquetes de los que dependen — las mismas bibliotecas se llaman
de otra manera en Debian y en Fedora.

```bash
sudo apt install ./subedit_<version>_amd64.deb      # Debian, Ubuntu
sudo dnf install ./subedit-<version>.x86_64.rpm     # Fedora, and relatives
```

| Lo que instala el paquete | Dónde |
| :---------------------- | :-- |
| `subedit-cli`, `subedit-gui` | `/usr/bin` |
| la entrada de menú, el icono, los metadatos del centro de software | `/usr/share/applications`, `/usr/share/icons`, `/usr/share/metainfo` |
| este manual, en Markdown | `/usr/share/subedit/manual` |
| su traducción a cada idioma instalado — catalán, checo, alemán, español, finés, francés, gallego, hebreo, húngaro, interlingue | `/usr/share/subedit/manual/<language>` — `ca`, `cs`, `de`, `es`, `fi`, `fr`, `gl`, `he`, `hu`, `ie` |
| los patrones de corrección de Gaupol | `/usr/share/subedit/patterns` |
| las páginas de manual de `subedit-cli`, una por subcomando, y de `subedit-gui` — véase [Las páginas de manual](invocation.md#las-páginas-de-manual) | `/usr/share/man/man1` |
| las páginas de `subedit-cli` en cada idioma instalado — catalán, checo, alemán, español, finés, francés, gallego, hebreo, húngaro, interlingue | `/usr/share/man/<language>/man1` — `ca`, `cs`, `de`, `es`, `fi`, `fr`, `gl`, `he`, `hu`, `ie` |

`ffmpeg` está **recomendado y no es obligatorio** en los dos paquetes: sin él, la
ventana solo deja de ofrecer la velocidad de fotogramas que declara la película — véase
[lo que cambia `ffmpeg`](../subedit-gui/video.md#ffmpeg-no-es-obligatorio).

### Lo que los paquetes exigen de la distribución

**Instalar con `apt` o `dnf`, nunca con `dpkg -i` ni `rpm -i`.** Esos dos
comandos depositan el paquete sin resolver sus dependencias: se limitan a
enumerar las que faltan, y se detienen. Esto es lo que responde `rpm -i` en
un Fedora que sí tiene lo que necesita al alcance de `dnf`:

```console
$ rpm -i subedit-0.13.0-1.fc43.x86_64.rpm
error: Failed dependencies:
        libicui18n.so.77()(64bit) is needed by subedit-0.13.0-1.fc43.x86_64
        libicuuc.so.77()(64bit) is needed by subedit-0.13.0-1.fc43.x86_64
```

`sudo dnf install ./subedit-<version>-1.fc43.x86_64.rpm` va a buscar esas
bibliotecas, allí donde existen.

**El `.rpm` que hay que tomar es el de la versión de Fedora instalada**: el nombre del archivo lleva la
versión, `fc42`, `fc43`, `fc44`. Cada uno se compila en ese Fedora, contra
su ICU — `cat /etc/fedora-release` dice cuál tomar. Un `.rpm` de otro
Fedora pide una ICU que la máquina no tiene.

| Dependencia | `.deb` | `.rpm` | Restricción |
| :--------- | :----- | :----- | :--------- |
| Qt 6: `Widgets`, `Gui`, `Core` | `libqt6widgets6`, `libqt6gui6`, `libqt6core6` | `qt6-qtbase-gui` | **≥ 6.4**, para la ventana |
| libmpv | `libmpv2` o `libmpv1` | `mpv-libs` | el binario enlaza `libmpv.so.2` |
| Enchant 2 | `libenchant-2-2` | `enchant2` | el corrector ortográfico, y las tareas `join-words` y `split-words` de [`correct`](correct.md) |
| **ICU** | **`libicu74`** | `libicu`, es decir, las `libicuuc.so.N` y `libicui18n.so.N` **del Fedora que lo compiló** | **la versión que tiene el Fedora del `.rpm`**, exactamente — véase más abajo |
| glibc y libstdc++ | — | — | **suficientemente recientes**: las de Ubuntu 24.04 para el `.deb`, las del Fedora del `.rpm` para este |

**ICU es la restricción que importa, y no se puede relajar
cambiando una línea del paquete.** Los símbolos de ICU llevan el número de su
versión mayor (`u_strToUTF8_74`): un binario compilado contra ICU 74 no puede
enlazarse con ICU 76 o 77. Una dependencia escrita de forma más amplia dejaría que el paquete
se instalara y luego fallara al arrancar — declararla exacta es lo que
sigue siendo honesto. **Por eso el `.rpm` no se compila una sola vez, sino en
cada Fedora publicado**, contra su ICU; el `.deb` se compila en Ubuntu 24.04.

| Distribución | Lo que da |
| :----------- | :---------------- |
| Ubuntu 24.04 | el `.deb` se instala: ICU está ahí en la versión 74 |
| Fedora 42, 43, 44 | el `.rpm` de su versión (`…fc42…`, `…fc43…`, `…fc44…`) se instala **mediante `dnf`**, compilado contra la ICU de ese Fedora |
| Fedora rawhide (el siguiente), las demás distribuciones `.rpm` | **no**: no se compila ningún `.rpm` para ellas — compilar desde el código fuente |
| cualquier distribución Debian o Ubuntu cuya ICU no esté en la versión 74 | **no**: el `.deb` exige `libicu74` — compilar desde el código fuente |

**Compilar desde el código fuente es la solución de reserva, y no tiene esta
restricción**: enlaza con la ICU de la máquina, sea cual sea su versión.
También es lo necesario en una distribución cuya glibc es más antigua
que la de los paquetes.

### Lo que se prueba de cada uno, y lo que no

**Los dos no se comprueban hasta el mismo punto, y más vale decirlo que dejar
creer en una paridad que no existe.** El desarrollo se hace en Ubuntu; un
`.rpm` compilado allí no puede instalarse allí. Se instala en otro sitio: en un
Fedora dentro de un contenedor, una vez por semana.

| | `.deb` | `.rpm` |
| :--- | :----- | :----- |
| la lista de archivos | comprobada | comprobada, y **comparada con la del `.deb`** |
| las dependencias declaradas | se comprueba que están | se comprueba que están |
| que los nombres de las dependencias existen en la distribución | sí, son los de Ubuntu | **sí**, resueltos por `dnf` |
| que el paquete se instala | no — eso necesita derechos de administrador | **sí**, en un Fedora dentro de un contenedor |
| que los binarios instalados arrancan | sí, desde un prefijo temporal | **sí**, desde el paquete instalado |

**La comprobación en Fedora prueba el `.rpm` compilado en Ubuntu, no los que se
publican.** Esa imagen no tiene una ICU 74 propia: el paquete se instala allí porque
Fedora conserva una `libicu74` de compatibilidad, y la comprobación seguiría en verde mientras
ese paquete exista. Los archivos `.rpm` publicados se compilan en su propio Fedora, y ya no
dependen de esa compatibilidad. Véase
[lo que exigen los paquetes](#lo-que-los-paquetes-exigen-de-la-distribución).

**Comparar las dos listas es la comprobación que más importa.** Los
dos paquetes salen de la misma instalación: una diferencia entre ellos sería un
defecto de las reglas de instalación, no del formato.

**Lo que el `.deb` todavía no demuestra es que se instala**, y la razón
no ha cambiado: `dpkg -i` necesita derechos de administrador, que una puerta de calidad
no tiene y no debe pedir. El `.rpm`, por su parte, se instala en un
contenedor que no pertenece a nadie. Véase
[ADR 0023](../../../../adr/0023-deb-et-rpm-pour-la-premiere-livraison.md).

**No se ofrecen Flatpak ni AppImage**, y es una elección — no un olvido.
Se explica en el mismo ADR, y es **definitiva**: una distribución que
no es ni de la familia Debian ni de la familia Fedora compila desde el
código fuente, lo que la sección siguiente describe por completo.

## Compilar desde el código fuente

### Requisitos previos

| Lo que se necesita | Por qué | Debian, Ubuntu |
| :------------ | :------- | :------------- |
| CMake ≥ 3.28 | la compilación | `cmake` |
| un compilador de C++23 — GCC 13 sirve | — | `g++` |
| `make`, `pkg-config` | la compilación | `make`, `pkg-config` |
| CLI11 | leer la línea de comandos | `libcli11-dev` |
| Qt 6 ≥ 6.4, módulo `Widgets` | la ventana | `qt6-base-dev` |
| `libmpv` | el reproductor integrado | `libmpv-dev` |
| ICU | leer las codificaciones | `libicu-dev` |
| Enchant 2 | la biblioteca del corrector ortográfico | `libenchant-2-dev` |

**Los cuatro últimos son obligatorios incluso para compilar solo
`subedit-cli`**: la configuración de CMake los busca para todo el proyecto, y
se detiene si faltan. ICU y Enchant son doblemente obligatorios — el ejecutable los carga, uno
para convertir las codificaciones, el otro para el corrector ortográfico — y
son las únicas bibliotecas de terceros que carga en tiempo de ejecución.

**Una conexión de red en el primer `cmake`**: la biblioteca de pruebas Catch2
se descarga de GitHub durante la configuración. No se vuelve a descargar
después. Para compilar solo la herramienta, sin red, véase más abajo.

#### Lo que no es obligatorio

| Lo que es opcional | Lo que se pierde sin ello |
| :-------------------- | :--------------------- |
| `ffmpeg`, por su `ffprobe` | la ventana deja de ofrecer la velocidad de fotogramas que declara la película — [los detalles](../subedit-gui/video.md#ffmpeg-no-es-obligatorio) |
| un servidor gráfico | nada para `subedit-cli`; `subedit-gui` necesita uno para mostrarse |

### Compilar

```bash
git clone git@github.com:Guyot-Bertrand/sub-edit.git subedit
cd subedit
make build
```

Los binarios se producen en `build/dev/bin/` — `subedit-cli` y
`subedit-gui`.

Para una versión optimizada:

```bash
make release
```

Los dos binarios están entonces en `build/release/bin/` — `subedit-cli` y
`subedit-gui`. El objetivo no compila nada más: ni el banco de pruebas, ni el
arnés de extremo a extremo, que tienen sus propios objetivos.

**Es preferible este objetivo a los dos comandos `cmake` equivalentes**, que no tienen
forma de saber cuántos procesos puede lanzar la optimización en tiempo de enlace:
en cada enlace, lanza tantos como núcleos haya.
`make release` le pasa `JOBS`, igual que al resto de la compilación.

### Instalar

```bash
make release
cmake --install build/release --prefix ~/.local
```

Lo que se instala, en las rutas que `GNUInstallDirs` fija para el
prefijo elegido:

| Archivo | Dónde |
| :------ | :-- |
| `subedit-cli`, `subedit-gui` | `<prefix>/bin` |
| la entrada del menú del escritorio | `<prefix>/share/applications` |
| el icono | `<prefix>/share/icons/hicolor/scalable/apps` |
| los metadatos del centro de software | `<prefix>/share/metainfo` |
| este manual, en Markdown | `<prefix>/share/subedit/manual` |
| su traducción a cada idioma instalado — catalán, checo, alemán, español, finés, francés, gallego, hebreo, húngaro, interlingue | `<prefix>/share/subedit/manual/<language>` — `ca`, `cs`, `de`, `es`, `fi`, `fr`, `gl`, `he`, `hu`, `ie` |
| los patrones de corrección de Gaupol | `<prefix>/share/subedit/patterns` |
| las páginas de manual de los dos binarios, una por subcomando para `subedit-cli` | `<prefix>/share/man/man1` |
| las páginas de `subedit-cli` en cada idioma instalado — catalán, checo, alemán, español, finés, francés, gallego, hebreo, húngaro, interlingue | `<prefix>/share/man/<language>/man1` — `ca`, `cs`, `de`, `es`, `fi`, `fr`, `gl`, `he`, `hu`, `ie` |

**Son los mismos archivos que instalan los paquetes**: los paquetes salen de ellos,
en lugar de describirse una segunda vez.

Con `--prefix ~/.local`, los dos binarios acaban en `~/.local/bin`,
que está en el `PATH` de la mayoría de las distribuciones. Un prefijo del sistema —
`/usr/local`, por ejemplo — necesita los derechos correspondientes.

**Una entrada de menú colocada bajo `~/.local` no siempre aparece
enseguida**: los escritorios releen su caché a su propio ritmo. Mientras tanto, `subedit-gui` se lanza
desde un terminal, como cualquier otro comando.

`DESTDIR` se respeta, como espera un empaquetador: una instalación escalonada
no toca nada fuera del directorio de preparación.

```bash
DESTDIR=/tmp/scene cmake --install build/release --prefix /usr
```

### Compilar solo la herramienta

Las pruebas se compilan por defecto, y son ellas las que piden la red —
`make release` no las compila, pero configura el proyecto, y es la
configuración la que descarga Catch2. Basta con desactivarlas una vez: el
ajuste se queda en la caché de CMake, y las compilaciones siguientes lo conservan.

```bash
cmake --preset release -DSUBEDIT_BUILD_TESTS=OFF
make release
```

Eso no exime de nada a quien contribuye: la puerta de calidad
sí las exige.

### Si la compilación falla al enlazar

Una avalancha de «undefined reference to `std::cout`» en un código válido
suele significar que la alternativa `c++` del sistema apunta a `gcc` en lugar
de a `g++`: el C++ compila, pero la biblioteca estándar no se enlaza.

```bash
ls -l /etc/alternatives/c++                                       # check
sudo update-alternatives --install /usr/bin/c++ c++ /usr/bin/g++ 100   # fix
CXX=g++ make build                                                # work around
```

La configuración de CMake detecta este caso y se detiene con este mensaje en lugar de
dejar que el enlace falle.

### Si la instalación del paquete se queja de `libicu`

```console
error: Failed dependencies:
        libicuuc.so.74()(64bit) is needed by subedit-<version>.x86_64
```

Tres causas, que se distinguen por el comando tecleado y por el nombre del archivo:

- **`rpm -i`**: no resuelve nada. Volver a intentarlo con
  `sudo dnf install ./subedit-<version>.fcNN.x86_64.rpm`;
- **un `.rpm` sin `fcNN` en su nombre**, o de un Fedora distinto del instalado:
  pide la ICU de la máquina que lo compiló. Tomar aquel cuyo `fcNN`
  es la salida de `cat /etc/fedora-release`;
- **ningún `.rpm` para esa versión**: Fedora rawhide u otra distribución.
  Ningún paquete de este repositorio se instalará allí;
  [compilar desde el código fuente](#compilar-desde-el-código-fuente).
