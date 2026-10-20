# Instalação

<!-- machine-translated -->
> Esta página é uma tradução automática e ainda não foi revista por nenhum leitor do idioma.

**Esta página vale para os dois programas** — `subedit-cli` e `subedit-gui`.
São compilados e instalados em conjunto.

Dois caminhos: **um pacote nativo**, `.deb` ou `.rpm`, ou **compilar a partir do
código-fonte**, que depois se instala onde se quiser.

## Os pacotes

Ambos são produzidos pelo CPack a partir de **as mesmas regras de instalação**:
colocam os mesmos ficheiros nos mesmos sítios, e só diferem nos
nomes dos pacotes de que dependem — as mesmas bibliotecas têm nomes
diferentes no Debian e no Fedora.

```bash
sudo apt install ./subedit_<version>_amd64.deb      # Debian, Ubuntu
sudo dnf install ./subedit-<version>.x86_64.rpm     # Fedora, and relatives
```

| O que o pacote instala | Onde |
| :---------------------- | :-- |
| `subedit-cli`, `subedit-gui` | `/usr/bin` |
| a entrada de menu, o ícone, os metadados do centro de software | `/usr/share/applications`, `/usr/share/icons`, `/usr/share/metainfo` |
| este manual, em Markdown | `/usr/share/subedit/manual` |
| a sua tradução para cada idioma instalado — catalão, checo, alemão, espanhol, finlandês, francês, galego, hebraico, húngaro, interlingue, islandês, italiano, neerlandês, polaco, português, português do Brasil | `/usr/share/subedit/manual/<language>` — `ca`, `cs`, `de`, `es`, `fi`, `fr`, `gl`, `he`, `hu`, `ie`, `is`, `it_IT`, `nl`, `pl`, `pt`, `pt_BR` |
| os padrões de correção do Gaupol | `/usr/share/subedit/patterns` |
| as páginas de manual de `subedit-cli`, uma por subcomando, e de `subedit-gui` — ver [As páginas de manual](invocation.md#as-páginas-de-manual) | `/usr/share/man/man1` |
| as páginas de `subedit-cli` em cada idioma instalado — catalão, checo, alemão, espanhol, finlandês, francês, galego, hebraico, húngaro, interlingue, islandês, italiano, neerlandês, polaco, português, português do Brasil | `/usr/share/man/<language>/man1` — `ca`, `cs`, `de`, `es`, `fi`, `fr`, `gl`, `he`, `hu`, `ie`, `is`, `it_IT`, `nl`, `pl`, `pt`, `pt_BR` |

O `ffmpeg` é **recomendado e não obrigatório** nos dois pacotes: sem ele, a
janela limita-se a deixar de propor a taxa de fotogramas que o filme declara — ver
[o que o `ffmpeg` altera](../subedit-gui/video.md#ffmpeg-não-é-necessário).

### O que os pacotes exigem da distribuição

**Instalar com `apt` ou `dnf`, nunca com `dpkg -i` nem `rpm -i`.** Esses dois
comandos depositam o pacote sem resolver as suas dependências: limitam-se a
enumerar as que faltam, e param. Eis o que responde o `rpm -i` num
Fedora que tem, de facto, o que é preciso ao alcance do `dnf`:

```console
$ rpm -i subedit-0.13.0-1.fc43.x86_64.rpm
error: Failed dependencies:
        libicui18n.so.77()(64bit) is needed by subedit-0.13.0-1.fc43.x86_64
        libicuuc.so.77()(64bit) is needed by subedit-0.13.0-1.fc43.x86_64
```

`sudo dnf install ./subedit-<version>-1.fc43.x86_64.rpm` vai buscar essas
bibliotecas, onde existirem.

**O `.rpm` a tomar é o do seu Fedora**: o nome do ficheiro traz a
versão, `fc42`, `fc43`, `fc44`. Cada um é compilado nesse Fedora, contra
o seu ICU — `cat /etc/fedora-release` diz qual tomar. Um `.rpm` de outro
Fedora pede um ICU que a máquina não tem.

| Dependência | `.deb` | `.rpm` | Restrição |
| :--------- | :----- | :----- | :--------- |
| Qt 6: `Widgets`, `Gui`, `Core` | `libqt6widgets6`, `libqt6gui6`, `libqt6core6` | `qt6-qtbase-gui` | **≥ 6.4**, para a janela |
| libmpv | `libmpv2` ou `libmpv1` | `mpv-libs` | o binário liga a `libmpv.so.2` |
| Enchant 2 | `libenchant-2-2` | `enchant2` | o corretor ortográfico, e as tarefas `join-words` e `split-words` de [`correct`](correct.md) |
| **ICU** | **`libicu74`** | `libicu`, isto é, as `libicuuc.so.N` e `libicui18n.so.N` **do Fedora que o compilou** | **a versão que o Fedora do `.rpm` tem**, exatamente — ver abaixo |
| glibc e libstdc++ | — | — | **suficientemente recentes**: as do Ubuntu 24.04 para o `.deb`, as do Fedora do `.rpm` para este |

**O ICU é a restrição que importa, e não se pode relaxar
alterando uma linha do pacote.** Os símbolos do ICU levam o número da sua
versão principal (`u_strToUTF8_74`): um binário compilado contra o ICU 74 não pode
ligar-se ao ICU 76 nem ao 77. Uma dependência escrita de forma mais ampla deixaria o pacote
instalar-se e depois falhar ao arrancar — declará-la exata é o que
se mantém honesto. **É por isso que o `.rpm` não é compilado uma só vez, mas em
cada Fedora publicado**, contra o seu ICU; o `.deb` é compilado no Ubuntu 24.04.

| Distribuição | O que dá |
| :----------- | :---------------- |
| Ubuntu 24.04 | o `.deb` instala-se: o ICU está aí na versão 74 |
| Fedora 42, 43, 44 | o `.rpm` da sua versão (`…fc42…`, `…fc43…`, `…fc44…`) instala-se **através do `dnf`**, compilado contra o ICU desse Fedora |
| Fedora rawhide (o seguinte), as outras distribuições `.rpm` | **não**: nenhum `.rpm` é compilado para elas — compilar a partir do código-fonte |
| qualquer distribuição Debian ou Ubuntu cujo ICU não esteja na versão 74 | **não**: o `.deb` exige `libicu74` — compilar a partir do código-fonte |

**Compilar a partir do código-fonte é a solução de recurso, e não tem esta
restrição**: liga ao ICU da máquina, seja qual for a sua versão.
É também o que é preciso numa distribuição cuja glibc é mais antiga
do que a dos pacotes.

### O que é testado de cada um, e o que não é

**Os dois não são verificados até ao mesmo ponto, e mais vale dizê-lo do que deixar
acreditar numa paridade que não existe.** O desenvolvimento faz-se no Ubuntu; um
`.rpm` compilado aí não pode ser instalado aí. É instalado noutro sítio: num
Fedora dentro de um contentor, uma vez por semana.

| | `.deb` | `.rpm` |
| :--- | :----- | :----- |
| a lista de ficheiros | verificada | verificada, e **comparada com a do `.deb`** |
| as dependências declaradas | verifica-se que estão presentes | verifica-se que estão presentes |
| que os nomes das dependências existem na distribuição | sim, são os do Ubuntu | **sim**, resolvidos pelo `dnf` |
| que o pacote se instala | não — isso exige permissões de administrador | **sim**, num Fedora dentro de um contentor |
| que os binários instalados arrancam | sim, a partir de um prefixo temporário | **sim**, a partir do pacote instalado |

**A verificação no Fedora testa o `.rpm` compilado no Ubuntu, não os que são
publicados.** Essa imagem não tem um ICU 74 próprio: o pacote instala-se aí porque
o Fedora mantém uma `libicu74` de compatibilidade, e a verificação continuaria verde enquanto
esse pacote existir. Os ficheiros `.rpm` publicados são compilados no seu próprio Fedora, e já não
dependem dessa compatibilidade. Ver
[o que os pacotes exigem](#o-que-os-pacotes-exigem-da-distribuição).

**Comparar as duas listas é a verificação que mais importa.** Os
dois pacotes saem da mesma instalação: uma diferença entre eles seria um
defeito das regras de instalação, não do formato.

**O que o `.deb` ainda não demonstra é que se instala**, e a razão
não mudou: o `dpkg -i` exige permissões de administrador, que um controlo de qualidade
não tem e não deve pedir. O `.rpm`, esse, instala-se num
contentor que não pertence a ninguém. Ver
[ADR 0023](../../../../adr/0023-deb-et-rpm-pour-la-premiere-livraison.md).

**Flatpak e AppImage não são oferecidos**, e é uma escolha — não um esquecimento.
Está explicado no mesmo ADR, e é **definitiva**: uma distribuição que
não seja nem da família Debian nem da família Fedora compila a partir do
código-fonte, o que a secção seguinte descreve por completo.

## Compilar a partir do código-fonte

### Pré-requisitos

| O que é preciso | Porquê | Debian, Ubuntu |
| :------------ | :------- | :------------- |
| CMake ≥ 3.28 | a compilação | `cmake` |
| um compilador de C++23 — o GCC 13 serve | — | `g++` |
| `make`, `pkg-config` | a compilação | `make`, `pkg-config` |
| CLI11 | ler a linha de comandos | `libcli11-dev` |
| Qt 6 ≥ 6.4, módulo `Widgets` | a janela | `qt6-base-dev` |
| `libmpv` | o reprodutor integrado | `libmpv-dev` |
| ICU | ler as codificações | `libicu-dev` |
| Enchant 2 | a biblioteca do corretor ortográfico | `libenchant-2-dev` |

**Os quatro últimos são obrigatórios mesmo para compilar apenas
`subedit-cli`**: a configuração do CMake procura-os para todo o projeto, e
para se faltarem. O ICU e o Enchant são duplamente obrigatórios — o executável carrega-os, um
para converter as codificações, o outro para o corretor ortográfico — e
são as únicas bibliotecas de terceiros que carrega em tempo de execução.

**Uma ligação de rede no primeiro `cmake`**: a biblioteca de testes Catch2
é obtida do GitHub durante a configuração. Não é transferida de novo
depois. Para compilar apenas a ferramenta, sem rede, ver abaixo.

#### O que não é obrigatório

| O que é opcional | O que se perde sem isso |
| :-------------------- | :--------------------- |
| `ffmpeg`, pelo seu `ffprobe` | a janela deixa de propor a taxa de fotogramas que o filme declara — [os pormenores](../subedit-gui/video.md#ffmpeg-não-é-necessário) |
| um servidor de ecrã | nada para `subedit-cli`; `subedit-gui` precisa de um para se mostrar |

### Compilar

```bash
git clone git@github.com:Guyot-Bertrand/sub-edit.git subedit
cd subedit
make build
```

Os binários são produzidos em `build/dev/bin/` — `subedit-cli` e
`subedit-gui`.

Para uma versão otimizada:

```bash
make release
```

Os dois binários ficam então em `build/release/bin/` — `subedit-cli` e
`subedit-gui`. O alvo não compila mais nada: nem o teste de desempenho, nem o
equipamento de testes de ponta a ponta, que têm os seus próprios alvos.

**Prefira-se este alvo aos dois comandos `cmake` equivalentes**, que não têm
maneira de saber quantos processos a otimização no momento da ligação pode
lançar: a cada ligação, lança tantos quantos os núcleos existentes.
O `make release` passa-lhe `JOBS`, tal como ao resto da compilação.

### Instalar

```bash
make release
cmake --install build/release --prefix ~/.local
```

O que é instalado, nos caminhos que o `GNUInstallDirs` define para o
prefixo escolhido:

| Ficheiro | Onde |
| :------ | :-- |
| `subedit-cli`, `subedit-gui` | `<prefix>/bin` |
| a entrada do menu do ambiente de trabalho | `<prefix>/share/applications` |
| o ícone | `<prefix>/share/icons/hicolor/scalable/apps` |
| os metadados do centro de software | `<prefix>/share/metainfo` |
| este manual, em Markdown | `<prefix>/share/subedit/manual` |
| a sua tradução para cada idioma instalado — catalão, checo, alemão, espanhol, finlandês, francês, galego, hebraico, húngaro, interlingue, islandês, italiano, neerlandês, polaco, português, português do Brasil | `<prefix>/share/subedit/manual/<language>` — `ca`, `cs`, `de`, `es`, `fi`, `fr`, `gl`, `he`, `hu`, `ie`, `is`, `it_IT`, `nl`, `pl`, `pt`, `pt_BR` |
| os padrões de correção do Gaupol | `<prefix>/share/subedit/patterns` |
| as páginas de manual dos dois binários, uma por subcomando para `subedit-cli` | `<prefix>/share/man/man1` |
| as páginas de `subedit-cli` em cada idioma instalado — catalão, checo, alemão, espanhol, finlandês, francês, galego, hebraico, húngaro, interlingue, islandês, italiano, neerlandês, polaco, português, português do Brasil | `<prefix>/share/man/<language>/man1` — `ca`, `cs`, `de`, `es`, `fi`, `fr`, `gl`, `he`, `hu`, `ie`, `is`, `it_IT`, `nl`, `pl`, `pt`, `pt_BR` |

**São os mesmos ficheiros que os pacotes instalam**: os pacotes saem deles,
em vez de serem descritos uma segunda vez.

Com `--prefix ~/.local`, os dois binários ficam em `~/.local/bin`,
que está no `PATH` da maioria das distribuições. Um prefixo do sistema —
`/usr/local`, por exemplo — exige as permissões correspondentes.

**Uma entrada de menu colocada em `~/.local` nem sempre aparece
de imediato**: os ambientes de trabalho voltam a ler a sua cache ao seu próprio ritmo. Entretanto, o `subedit-gui` arranca
a partir de um terminal, como qualquer outro comando.

`DESTDIR` é respeitado, como um empacotador espera: uma instalação em
área de preparação não toca em nada fora do diretório de preparação.

```bash
DESTDIR=/tmp/scene cmake --install build/release --prefix /usr
```

### Compilar apenas a ferramenta

Os testes são compilados por predefinição, e são eles que pedem a rede —
o `make release` não os compila, mas configura o projeto, e é a
configuração que obtém o Catch2. Desativá-los uma vez basta: a
definição fica na cache do CMake, e as compilações seguintes mantêm-na.

```bash
cmake --preset release -DSUBEDIT_BUILD_TESTS=OFF
make release
```

Isso não dispensa ninguém que contribua: o controlo de qualidade
exige-os.

### Se a compilação falhar na ligação

Uma avalancha de “undefined reference to `std::cout`” em código válido
significa habitualmente que a alternativa `c++` do sistema aponta para `gcc` em vez
de `g++`: o C++ compila, mas a biblioteca padrão não é ligada.

```bash
ls -l /etc/alternatives/c++                                       # check
sudo update-alternatives --install /usr/bin/c++ c++ /usr/bin/g++ 100   # fix
CXX=g++ make build                                                # work around
```

A configuração do CMake deteta este caso e para com esta mensagem em vez de
deixar a ligação falhar.

### Se a instalação do pacote se queixar de `libicu`

```console
error: Failed dependencies:
        libicuuc.so.74()(64bit) is needed by subedit-<version>.x86_64
```

Três causas, distinguidas pelo comando escrito e pelo nome do ficheiro:

- **`rpm -i`**: não resolve nada. Tentar de novo com
  `sudo dnf install ./subedit-<version>.fcNN.x86_64.rpm`;
- **um `.rpm` sem `fcNN` no nome**, ou de um Fedora diferente do seu:
  pede o ICU da máquina que o compilou. Tomar aquele cujo `fcNN`
  é o resultado de `cat /etc/fedora-release`;
- **nenhum `.rpm` para a sua versão**: Fedora rawhide ou outra distribuição.
  Nenhum pacote deste repositório se instalará aí;
  [compilar a partir do código-fonte](#compilar-a-partir-do-código-fonte).
