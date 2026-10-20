# Instalação

<!-- machine-translated -->
> Esta página é uma tradução automática e ainda não foi revisada por um leitor do idioma.

**Esta página vale para os dois programas** — `subedit-cli` e `subedit-gui`.
Eles são compilados e instalados juntos.

Dois caminhos: **um pacote nativo**, `.deb` ou `.rpm`, ou **compilar a partir do
código-fonte**, o que instala onde se quiser.

## Os pacotes

Ambos são produzidos pelo CPack a partir das **mesmas regras de instalação**:
colocam os mesmos arquivos nos mesmos lugares, e diferem apenas nos
nomes dos pacotes de que dependem — as mesmas bibliotecas têm nomes
diferentes no Debian e no Fedora.

```bash
sudo apt install ./subedit_<version>_amd64.deb      # Debian, Ubuntu
sudo dnf install ./subedit-<version>.x86_64.rpm     # Fedora, and relatives
```

| O que o pacote instala | Onde |
| :---------------------- | :-- |
| `subedit-cli`, `subedit-gui` | `/usr/bin` |
| a entrada de menu, o ícone, os metadados da central de programas | `/usr/share/applications`, `/usr/share/icons`, `/usr/share/metainfo` |
| este manual, em Markdown | `/usr/share/subedit/manual` |
| sua tradução para cada idioma instalado — catalão, tcheco, alemão, espanhol, finlandês, francês, galego, hebraico, húngaro, interlíngua, islandês, italiano, holandês, polonês, português, português do Brasil | `/usr/share/subedit/manual/<language>` — `ca`, `cs`, `de`, `es`, `fi`, `fr`, `gl`, `he`, `hu`, `ie`, `is`, `it_IT`, `nl`, `pl`, `pt`, `pt_BR` |
| os padrões de correção do Gaupol | `/usr/share/subedit/patterns` |
| as páginas de manual de `subedit-cli`, uma por subcomando, e de `subedit-gui` — veja [As páginas de manual](invocation.md#as-páginas-de-manual) | `/usr/share/man/man1` |
| as páginas de `subedit-cli` em cada idioma instalado — catalão, tcheco, alemão, espanhol, finlandês, francês, galego, hebraico, húngaro, interlíngua, islandês, italiano, holandês, polonês, português, português do Brasil | `/usr/share/man/<language>/man1` — `ca`, `cs`, `de`, `es`, `fi`, `fr`, `gl`, `he`, `hu`, `ie`, `is`, `it_IT`, `nl`, `pl`, `pt`, `pt_BR` |

O `ffmpeg` é **recomendado e não obrigatório** nos dois pacotes: sem ele, a
janela apenas deixa de oferecer a taxa de quadros que o filme declara — veja
[o que o `ffmpeg` muda](../subedit-gui/video.md#ffmpeg-não-é-obrigatório).

### O que os pacotes exigem da distribuição

**Instalar com `apt` ou `dnf`, nunca com `dpkg -i` ou `rpm -i`.** Esses dois
comandos depositam o pacote sem resolver suas dependências: limitam-se a
listar as que faltam, e param. Isto é o que `rpm -i` responde em
um Fedora que tem o necessário ao alcance do `dnf`:

```console
$ rpm -i subedit-0.13.0-1.fc43.x86_64.rpm
error: Failed dependencies:
        libicui18n.so.77()(64bit) is needed by subedit-0.13.0-1.fc43.x86_64
        libicuuc.so.77()(64bit) is needed by subedit-0.13.0-1.fc43.x86_64
```

`sudo dnf install ./subedit-<version>-1.fc43.x86_64.rpm` busca essas
bibliotecas, onde existirem.

**O `.rpm` a pegar é o do seu Fedora**: o nome do arquivo traz a
versão, `fc42`, `fc43`, `fc44`. Cada um é compilado nesse Fedora, com o
ICU dele — `cat /etc/fedora-release` diz qual pegar. Um `.rpm` de outro
Fedora pede um ICU que o seu não tem.

| Dependência | `.deb` | `.rpm` | Restrição |
| :--------- | :----- | :----- | :--------- |
| Qt 6: `Widgets`, `Gui`, `Core` | `libqt6widgets6`, `libqt6gui6`, `libqt6core6` | `qt6-qtbase-gui` | **≥ 6.4**, para a janela |
| libmpv | `libmpv2` ou `libmpv1` | `mpv-libs` | o binário liga-se a `libmpv.so.2` |
| Enchant 2 | `libenchant-2-2` | `enchant2` | o corretor ortográfico, e as tarefas `join-words` e `split-words` de [`correct`](correct.md) |
| **ICU** | **`libicu74`** | `libicu`, isto é, `libicuuc.so.N` e `libicui18n.so.N` **do Fedora que o compilou** | **a versão que o Fedora do `.rpm` tem**, exatamente — veja abaixo |
| glibc e libstdc++ | — | — | **recentes o bastante**: as do Ubuntu 24.04 para o `.deb`, as do Fedora do `.rpm` para este |

**O ICU é a restrição que importa, e ela não pode ser flexibilizada
alterando uma linha do pacote.** Os símbolos do ICU carregam o número de sua
versão principal (`u_strToUTF8_74`): um binário compilado com o ICU 74 não pode
se ligar ao ICU 76 ou 77. Uma dependência escrita de forma mais ampla deixaria o pacote
instalar e depois falhar na inicialização — declará-la exata é o que
se mantém honesto. **É por isso que o `.rpm` não é compilado uma só vez, mas em
cada Fedora lançado**, com o ICU dele; o `.deb` é compilado no Ubuntu 24.04.

| Distribuição | O que oferece |
| :----------- | :---------------- |
| Ubuntu 24.04 | o `.deb` instala: o ICU está na versão 74 ali |
| Fedora 42, 43, 44 | o `.rpm` da sua versão (`…fc42…`, `…fc43…`, `…fc44…`) instala **pelo `dnf`**, compilado com o ICU desse Fedora |
| Fedora rawhide (o próximo), as outras distribuições `.rpm` | **não**: nenhum `.rpm` é compilado para elas — compilar a partir do código-fonte |
| qualquer distribuição Debian ou Ubuntu cujo ICU não esteja na versão 74 | **não**: o `.deb` exige `libicu74` — compilar a partir do código-fonte |

**Compilar a partir do código-fonte é a alternativa, e não tem essa
restrição**: liga-se ao ICU da máquina, qualquer que seja a versão.
É também o que se precisa em uma distribuição cuja glibc é mais antiga
que a dos pacotes.

### O que é testado em cada um, e o que não é

**Os dois não são verificados até o mesmo ponto, e é melhor dizê-lo do que deixar
acreditar em uma paridade que não existe.** O desenvolvimento acontece no Ubuntu; um
`.rpm` compilado ali não pode ser instalado ali. Ele é instalado em outro lugar: em um
Fedora dentro de um contêiner, uma vez por semana.

| | `.deb` | `.rpm` |
| :--- | :----- | :----- |
| a lista de arquivos | verificada | verificada, e **comparada com a do `.deb`** |
| as dependências declaradas | presença verificada | presença verificada |
| que os nomes das dependências existem na distribuição | sim, são os do Ubuntu | **sim**, resolvidos pelo `dnf` |
| que o pacote instala | não — isso exige direitos de administrador | **sim**, em um Fedora dentro de um contêiner |
| que os binários instalados iniciam | sim, a partir de um prefixo temporário | **sim**, a partir do pacote instalado |

**A verificação no Fedora testa o `.rpm` compilado no Ubuntu, não os que são
lançados.** Essa imagem não tem ICU 74 próprio: o pacote instala ali porque
o Fedora mantém um `libicu74` de compatibilidade, e a verificação ficaria verde enquanto
esse pacote existisse. Os arquivos `.rpm` lançados são compilados em seu próprio Fedora, e não
dependem mais dessa compatibilidade. Veja
[o que os pacotes exigem](#o-que-os-pacotes-exigem-da-distribuição).

**Comparar as duas listas é a verificação que mais importa.** Os
dois pacotes saem da mesma instalação: uma diferença entre eles seria um
defeito das regras de instalação, não do formato.

**O que o `.deb` ainda não prova é que ele instala**, e o motivo
não mudou: `dpkg -i` exige direitos de administrador, que uma porta de qualidade
não tem e não deve pedir. O `.rpm`, por sua vez, instala em um
contêiner que não pertence a ninguém. Veja
[ADR 0023](../../../../adr/0023-deb-et-rpm-pour-la-premiere-livraison.md).

**Flatpak e AppImage não são oferecidos**, e é uma escolha — não um descuido.
Ela é explicada no mesmo ADR, e é **definitiva**: uma distribuição que
não é da família Debian nem da família Fedora compila a partir do
código-fonte, o que a próxima seção descreve por inteiro.

## Compilar a partir do código-fonte

### Pré-requisitos

| O que é necessário | Para quê | Debian, Ubuntu |
| :------------ | :------- | :------------- |
| CMake ≥ 3.28 | a compilação | `cmake` |
| um compilador C++23 — o GCC 13 serve | — | `g++` |
| `make`, `pkg-config` | a compilação | `make`, `pkg-config` |
| CLI11 | ler a linha de comando | `libcli11-dev` |
| Qt 6 ≥ 6.4, módulo `Widgets` | a janela | `qt6-base-dev` |
| `libmpv` | o reprodutor integrado | `libmpv-dev` |
| ICU | ler as codificações | `libicu-dev` |
| Enchant 2 | a biblioteca do corretor ortográfico | `libenchant-2-dev` |

**Os quatro últimos são necessários mesmo para compilar apenas
o `subedit-cli`**: a configuração do CMake os procura para o projeto inteiro e
para se faltarem. O ICU e o Enchant são necessários por dois motivos — o executável os carrega, um
para converter as codificações, o outro para o corretor ortográfico — e
são as únicas bibliotecas de terceiros que ele carrega em tempo de execução.

**Uma conexão de rede no primeiro `cmake`**: a biblioteca de testes Catch2
é baixada do GitHub na configuração. Ela não é baixada de novo
depois. Para compilar apenas a ferramenta, sem rede, veja abaixo.

#### O que não é obrigatório

| O que é opcional | O que se perde sem isso |
| :-------------------- | :--------------------- |
| `ffmpeg`, por causa do seu `ffprobe` | a janela deixa de oferecer a taxa de quadros que o filme declara — [os detalhes](../subedit-gui/video.md#ffmpeg-não-é-obrigatório) |
| um servidor gráfico | nada para o `subedit-cli`; o `subedit-gui` precisa de um para se exibir |

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
`subedit-gui`. O alvo não compila mais nada: nem o benchmark, nem o
harness de ponta a ponta, que têm seus próprios alvos.

**Prefira este alvo aos dois comandos `cmake` equivalentes**, que não têm
como saber quantos processos a otimização em tempo de ligação pode
iniciar: a cada ligação, ela inicia tantos quantos são os núcleos.
`make release` repassa-lhe `JOBS`, como faz com o restante da compilação.

### Instalar

```bash
make release
cmake --install build/release --prefix ~/.local
```

O que é instalado, nos caminhos que o `GNUInstallDirs` define para o
prefixo escolhido:

| Arquivo | Onde |
| :------ | :-- |
| `subedit-cli`, `subedit-gui` | `<prefix>/bin` |
| a entrada de menu da área de trabalho | `<prefix>/share/applications` |
| o ícone | `<prefix>/share/icons/hicolor/scalable/apps` |
| os metadados da central de programas | `<prefix>/share/metainfo` |
| este manual, em Markdown | `<prefix>/share/subedit/manual` |
| sua tradução para cada idioma instalado — catalão, tcheco, alemão, espanhol, finlandês, francês, galego, hebraico, húngaro, interlíngua, islandês, italiano, holandês, polonês, português, português do Brasil | `<prefix>/share/subedit/manual/<language>` — `ca`, `cs`, `de`, `es`, `fi`, `fr`, `gl`, `he`, `hu`, `ie`, `is`, `it_IT`, `nl`, `pl`, `pt`, `pt_BR` |
| os padrões de correção do Gaupol | `<prefix>/share/subedit/patterns` |
| as páginas de manual dos dois binários, uma por subcomando para `subedit-cli` | `<prefix>/share/man/man1` |
| as páginas de `subedit-cli` em cada idioma instalado — catalão, tcheco, alemão, espanhol, finlandês, francês, galego, hebraico, húngaro, interlíngua, islandês, italiano, holandês, polonês, português, português do Brasil | `<prefix>/share/man/<language>/man1` — `ca`, `cs`, `de`, `es`, `fi`, `fr`, `gl`, `he`, `hu`, `ie`, `is`, `it_IT`, `nl`, `pl`, `pt`, `pt_BR` |

**São os mesmos arquivos que os pacotes instalam**: os pacotes saem deles,
em vez de serem descritos uma segunda vez.

Com `--prefix ~/.local`, os dois binários vão para `~/.local/bin`,
que está no `PATH` da maioria das distribuições. Um prefixo de sistema —
`/usr/local`, por exemplo — exige os direitos correspondentes.

**Uma entrada de menu colocada em `~/.local` nem sempre aparece de imediato**:
as áreas de trabalho relêem seu cache no próprio ritmo. O `subedit-gui` inicia
a partir de um terminal enquanto isso, como qualquer outro comando.

`DESTDIR` é respeitado, o que um empacotador espera: uma instalação em
área de preparação não toca em nada fora do diretório de preparação.

```bash
DESTDIR=/tmp/scene cmake --install build/release --prefix /usr
```

### Compilar a ferramenta sozinha

Os testes são compilados por padrão, e são eles que pedem a rede —
`make release` não os compila, mas configura o projeto, e é a
configuração que baixa o Catch2. Desativá-los uma vez basta: a
configuração fica no cache do CMake, e as compilações seguintes a mantêm.

```bash
cmake --preset release -DSUBEDIT_BUILD_TESTS=OFF
make release
```

Isso não dispensa nenhum colaborador: a porta de qualidade
os exige.

### Se a compilação falha na ligação

Uma avalanche de "undefined reference to `std::cout`" em código válido
significa em geral que a alternativa `c++` do sistema aponta para `gcc` em vez de
`g++`: o C++ compila, mas a biblioteca padrão não é ligada.

```bash
ls -l /etc/alternatives/c++                                       # check
sudo update-alternatives --install /usr/bin/c++ c++ /usr/bin/g++ 100   # fix
CXX=g++ make build                                                # work around
```

A configuração do CMake detecta esse caso e para com esta mensagem em vez de
deixar a ligação falhar.

### Se a instalação do pacote reclama de `libicu`

```console
error: Failed dependencies:
        libicuuc.so.74()(64bit) is needed by subedit-<version>.x86_64
```

Três causas, distinguidas pelo comando digitado e pelo nome do arquivo:

- **`rpm -i`**: ele não resolve nada. Tente de novo com
  `sudo dnf install ./subedit-<version>.fcNN.x86_64.rpm`;
- **um `.rpm` sem `fcNN` no nome**, ou de um Fedora diferente do seu:
  ele pede o ICU da máquina que o compilou. Pegue aquele cujo `fcNN`
  é a saída de `cat /etc/fedora-release`;
- **nenhum `.rpm` para a sua versão**: Fedora rawhide ou outra distribuição.
  Nenhum pacote deste repositório instalará ali;
  [compile a partir do código-fonte](#compilar-a-partir-do-código-fonte).
