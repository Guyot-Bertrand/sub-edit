# Instalación

<!-- machine-translated -->
> Esta páxina é unha tradución automática e aínda non a revisou ningún lector da lingua.

**Esta páxina vale para os dous programas** — `subedit-cli` e `subedit-gui`.
Compílanse e instálanse xuntos.

Dous camiños: **un paquete nativo**, `.deb` ou `.rpm`, ou **compilar desde
o código fonte**, que despois se instala onde se queira.

## Os paquetes

Os dous prodúceos CPack a partir das **mesmas regras de instalación**:
poñen os mesmos ficheiros nos mesmos lugares, e só difiren nos
nomes dos paquetes dos que dependen — as mesmas bibliotecas chámanse
de forma distinta en Debian e en Fedora.

```bash
sudo apt install ./subedit_<version>_amd64.deb      # Debian, Ubuntu
sudo dnf install ./subedit-<version>.x86_64.rpm     # Fedora, and relatives
```

| O que instala o paquete | Onde |
| :---------------------- | :-- |
| `subedit-cli`, `subedit-gui` | `/usr/bin` |
| a entrada do menú, a icona, os metadatos do centro de software | `/usr/share/applications`, `/usr/share/icons`, `/usr/share/metainfo` |
| este manual, en Markdown | `/usr/share/subedit/manual` |
| a súa tradución a cada lingua instalada — catalán, checo, alemán, español, finés, francés, galego | `/usr/share/subedit/manual/<language>` — `ca`, `cs`, `de`, `es`, `fi`, `fr`, `gl` |
| os padróns de corrección de Gaupol | `/usr/share/subedit/patterns` |
| as páxinas de manual de `subedit-cli`, unha por suborde, e de `subedit-gui` — véxase [As páxinas de manual](invocation.md#as-páxinas-de-manual) | `/usr/share/man/man1` |
| as páxinas de `subedit-cli` en cada lingua instalada — catalán, checo, alemán, español, finés, francés, galego | `/usr/share/man/<language>/man1` — `ca`, `cs`, `de`, `es`, `fi`, `fr`, `gl` |

`ffmpeg` é **recomendado e non obrigatorio** para os dous paquetes: sen el, a
xanela só deixa de ofrecer a taxa de fotogramas que declara a película — véxase
[o que cambia `ffmpeg`](../subedit-gui/video.md#ffmpeg-non-é-obrigatorio).

### O que os paquetes requiren da distribución

**Instale con `apt` ou `dnf`, nunca con `dpkg -i` nin `rpm -i`.** Esas dúas
ordes depositan o paquete sen resolver as súas dependencias: limítanse a
enumerar as que faltan, e detéñense. Isto é o que responde `rpm -i` nun
Fedora que si ten ao alcance de `dnf` o que necesita:

```console
$ rpm -i subedit-0.13.0-1.fc43.x86_64.rpm
error: Failed dependencies:
        libicui18n.so.77()(64bit) is needed by subedit-0.13.0-1.fc43.x86_64
        libicuuc.so.77()(64bit) is needed by subedit-0.13.0-1.fc43.x86_64
```

`sudo dnf install ./subedit-<version>-1.fc43.x86_64.rpm` descarga esas
bibliotecas, alí onde existen.

**O `.rpm` que hai que coller é o do seu Fedora**: o nome do ficheiro leva a
versión, `fc42`, `fc43`, `fc44`. Cada un compílase nese Fedora, coa
súa ICU — `cat /etc/fedora-release` di cal hai que coller. Un `.rpm` doutro
Fedora pide unha ICU que o seu non ten.

| Dependencia | `.deb` | `.rpm` | Restrición |
| :--------- | :----- | :----- | :--------- |
| Qt 6: `Widgets`, `Gui`, `Core` | `libqt6widgets6`, `libqt6gui6`, `libqt6core6` | `qt6-qtbase-gui` | **≥ 6.4**, para a xanela |
| libmpv | `libmpv2` ou `libmpv1` | `mpv-libs` | o binario enlaza `libmpv.so.2` |
| Enchant 2 | `libenchant-2-2` | `enchant2` | o corrector ortográfico, e as tarefas `join-words` e `split-words` de [`correct`](correct.md) |
| **ICU** | **`libicu74`** | `libicu`, é dicir as `libicuuc.so.N` e `libicui18n.so.N` **do Fedora que o compilou** | **a versión que ten o Fedora do `.rpm`**, exactamente — véxase máis abaixo |
| glibc e libstdc++ | — | — | **o bastante recentes**: as de Ubuntu 24.04 para o `.deb`, as do Fedora do `.rpm` para este |

**ICU é a restrición que importa, e non se pode relaxar
cambiando unha liña do paquete.** Os símbolos de ICU levan o número da súa
versión maior (`u_strToUTF8_74`): un binario compilado con ICU 74 non pode
enlazar con ICU 76 nin 77. Unha dependencia escrita de forma máis ampla deixaría
instalar o paquete, e despois fallaría ao iniciar — declarala exacta é o que
segue sendo honesto. **Por iso o `.rpm` non se compila unha soa vez, senón en
cada Fedora publicado**, coa súa ICU; o `.deb` compílase en Ubuntu 24.04.

| Distribución | O que dá |
| :----------- | :---------------- |
| Ubuntu 24.04 | o `.deb` instálase: alí a ICU está na versión 74 |
| Fedora 42, 43, 44 | o `.rpm` da súa versión (`…fc42…`, `…fc43…`, `…fc44…`) instálase **mediante `dnf`**, compilado coa ICU dese Fedora |
| Fedora rawhide (o seguinte), as outras distribucións `.rpm` | **non**: non se compila ningún `.rpm` para elas — compile desde o código fonte |
| calquera distribución Debian ou Ubuntu cuxa ICU non estea na versión 74 | **non**: o `.deb` require `libicu74` — compile desde o código fonte |

**Compilar desde o código fonte é a solución alternativa, e non ten esta
restrición**: enlaza coa ICU da máquina, sexa cal sexa a súa versión.
Tamén é o que cómpre nunha distribución cuxa glibc é máis antiga
ca a dos paquetes.

### O que se proba de cada un, e o que non

**Os dous non se verifican ata o mesmo punto, e é mellor dicilo que deixar
crer nunha paridade que non existe.** O desenvolvemento faise en Ubuntu; un
`.rpm` compilado alí non se pode instalar alí. Instálase noutro lugar: nun
Fedora nun contedor, unha vez por semana.

| | `.deb` | `.rpm` |
| :--- | :----- | :----- |
| a lista de ficheiros | verificada | verificada, e **comparada coa do `.deb`** |
| as dependencias declaradas | verificada a súa presenza | verificada a súa presenza |
| que os nomes das dependencias existen na distribución | si, son os de Ubuntu | **si**, resoltos por `dnf` |
| que o paquete se instala | non — iso require dereitos de administrador | **si**, nun Fedora nun contedor |
| que os binarios instalados arrincan | si, desde un prefixo temporal | **si**, desde o paquete instalado |

**A verificación en Fedora proba o `.rpm` compilado en Ubuntu, non os que se
publican.** Esa imaxe non ten unha ICU 74 propia: o paquete instálase alí porque
Fedora mantén unha `libicu74` de compatibilidade, e a verificación estaría en verde mentres
exista ese paquete. Os ficheiros `.rpm` publicados compílanse no seu propio Fedora, e xa non
dependen desa compatibilidade. Véxase
[o que requiren os paquetes](#o-que-os-paquetes-requiren-da-distribución).

**Comparar as dúas listas é a verificación que máis importa.** Os
dous paquetes saen da mesma instalación: unha diferenza entre eles sería un
defecto das regras de instalación, non do formato.

**O que o `.deb` aínda non proba é que se instala**, e o motivo
non cambiou: `dpkg -i` require dereitos de administrador, que unha porta de calidade
non ten e non debe pedir. O `.rpm`, pola súa banda, instálase nun
contedor que non é de ninguén. Véxase
[ADR 0023](../../../../adr/0023-deb-et-rpm-pour-la-premiere-livraison.md).

**Non se ofrecen Flatpak nin AppImage**, e iso é unha escolla — non un descoido.
Explícase no mesmo ADR, e é **definitiva**: unha distribución que
non é nin da familia Debian nin da familia Fedora compila desde o
código fonte, o que a sección seguinte describe por completo.

## Compilar desde o código fonte

### Requisitos previos

| O que cómpre | Por que | Debian, Ubuntu |
| :------------ | :------- | :------------- |
| CMake ≥ 3.28 | a compilación | `cmake` |
| un compilador C++23 — abonda con GCC 13 | — | `g++` |
| `make`, `pkg-config` | a compilación | `make`, `pkg-config` |
| CLI11 | ler a liña de ordes | `libcli11-dev` |
| Qt 6 ≥ 6.4, módulo `Widgets` | a xanela | `qt6-base-dev` |
| `libmpv` | o reprodutor integrado | `libmpv-dev` |
| ICU | ler as codificacións | `libicu-dev` |
| Enchant 2 | a biblioteca do corrector ortográfico | `libenchant-2-dev` |

**As catro últimas son obrigatorias mesmo para compilar só
`subedit-cli`**: a configuración de CMake búscaas para todo o proxecto, e
detense se faltan. ICU e Enchant son obrigatorias por partida dobre — o executable cárgaas, unha
para converter as codificacións, a outra para o corrector ortográfico — e
son as únicas bibliotecas de terceiros que carga no momento da execución.

**Unha conexión de rede no primeiro `cmake`**: a biblioteca de probas Catch2
descárgase de GitHub durante a configuración. Despois non se volve
descargar. Para compilar a ferramenta soa, sen rede, véxase máis abaixo.

#### O que non é obrigatorio

| O que é opcional | O que se perde sen el |
| :-------------------- | :--------------------- |
| `ffmpeg`, polo seu `ffprobe` | a xanela xa non ofrece a taxa de fotogramas que declara a película — [os detalles](../subedit-gui/video.md#ffmpeg-non-é-obrigatorio) |
| un servidor de visualización | nada para `subedit-cli`; `subedit-gui` precisa un para mostrarse |

### Compilar

```bash
git clone git@github.com:Guyot-Bertrand/sub-edit.git subedit
cd subedit
make build
```

Os binarios prodúcense en `build/dev/bin/` — `subedit-cli` e
`subedit-gui`.

Para unha versión optimizada:

```bash
make release
```

Os dous binarios están entón en `build/release/bin/` — `subedit-cli` e
`subedit-gui`. O obxectivo non compila nada máis: nin o banco de probas de rendemento, nin o
arnés de extremo a extremo, que teñen os seus propios obxectivos.

**Prefira este obxectivo ás dúas ordes `cmake` equivalentes**, que non teñen
forma de saber cantos procesos pode iniciar a optimización no momento do
enlazado: en cada enlazado, inicia tantos como núcleos haxa.
`make release` pásalle `JOBS`, como fai co resto da compilación.

### Instalar

```bash
make release
cmake --install build/release --prefix ~/.local
```

O que se instala, nas rutas que fixa `GNUInstallDirs` para o prefixo
escollido:

| Ficheiro | Onde |
| :------ | :-- |
| `subedit-cli`, `subedit-gui` | `<prefix>/bin` |
| a entrada do menú do escritorio | `<prefix>/share/applications` |
| a icona | `<prefix>/share/icons/hicolor/scalable/apps` |
| os metadatos do centro de software | `<prefix>/share/metainfo` |
| este manual, en Markdown | `<prefix>/share/subedit/manual` |
| a súa tradución a cada lingua instalada — catalán, checo, alemán, español, finés, francés, galego | `<prefix>/share/subedit/manual/<language>` — `ca`, `cs`, `de`, `es`, `fi`, `fr`, `gl` |
| os padróns de corrección de Gaupol | `<prefix>/share/subedit/patterns` |
| as páxinas de manual dos dous binarios, unha por suborde para `subedit-cli` | `<prefix>/share/man/man1` |
| as páxinas de `subedit-cli` en cada lingua instalada — catalán, checo, alemán, español, finés, francés, galego | `<prefix>/share/man/<language>/man1` — `ca`, `cs`, `de`, `es`, `fi`, `fr`, `gl` |

**Son os mesmos ficheiros que instalan os paquetes**: os paquetes saen deles,
en vez de describirse unha segunda vez.

Con `--prefix ~/.local`, os dous binarios acaban en `~/.local/bin`,
que está no `PATH` da maioría das distribucións. Un prefixo do sistema —
`/usr/local`, por exemplo — necesita os dereitos correspondentes.

**Unha entrada do menú posta en `~/.local` non sempre aparece
de inmediato**: os escritorios volven ler a súa caché ao seu propio ritmo. Mentres tanto, `subedit-gui` iníciase
desde un terminal, como calquera outra orde.

Respéctase `DESTDIR`, o que agarda un empaquetador: unha instalación en preparación
non toca nada fóra do directorio de preparación.

```bash
DESTDIR=/tmp/scene cmake --install build/release --prefix /usr
```

### Compilar a ferramenta soa

As probas compílanse de forma predeterminada, e son elas as que piden a rede —
`make release` non as compila, pero configura o proxecto, e é a
configuración a que descarga Catch2. Abonda con desactivalas unha vez: o
axuste queda na caché de CMake, e as compilacións seguintes consérvano.

```bash
cmake --preset release -DSUBEDIT_BUILD_TESTS=OFF
make release
```

Iso non exime de nada a quen contribúe: a porta de calidade
si as require.

### Se a compilación falla no enlazado

Unha avalancha de «undefined reference to `std::cout`» en código válido
adoita significar que a alternativa `c++` do sistema apunta a `gcc` en vez
de a `g++`: o C++ compílase, pero non se enlaza a biblioteca estándar.

```bash
ls -l /etc/alternatives/c++                                       # check
sudo update-alternatives --install /usr/bin/c++ c++ /usr/bin/g++ 100   # fix
CXX=g++ make build                                                # work around
```

A configuración de CMake detecta este caso e detense con esta mensaxe en vez de
deixar que falle o enlazado.

### Se a instalación do paquete se queixa de `libicu`

```console
error: Failed dependencies:
        libicuuc.so.74()(64bit) is needed by subedit-<version>.x86_64
```

Tres causas, que se distinguen pola orde escrita e polo nome do ficheiro:

- **`rpm -i`**: non resolve nada. Ténteo de novo con
  `sudo dnf install ./subedit-<version>.fcNN.x86_64.rpm`;
- **un `.rpm` sen `fcNN` no nome**, ou dun Fedora distinto do seu:
  pide a ICU da máquina que o compilou. Colla aquel cuxo `fcNN`
  é a saída de `cat /etc/fedora-release`;
- **ningún `.rpm` para a súa versión**: Fedora rawhide ou outra distribución.
  Ningún paquete deste repositorio se instalará alí;
  [compile desde o código fonte](#compilar-desde-o-código-fonte).
