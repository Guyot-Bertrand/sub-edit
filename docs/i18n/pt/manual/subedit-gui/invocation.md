# Invocação

<!-- machine-translated -->
> Esta página é uma tradução automática e ainda não foi revista por nenhum leitor do idioma.

```
subedit-gui [--version] [<file>]
```

| Argumento | Obrigatório | Valor | Predefinição |
| :------- | :------- | :---- | :------ |
| `<file>` | não | o caminho de um ficheiro de legendas a abrir | nenhum — a janela abre vazia |
| `--version` | não | — | — |

**Um único ficheiro.** Os argumentos seguintes são ignorados: a janela só
mantém um projeto de cada vez.

## `--version`

Escreve na saída padrão o bloco que o `--version` da linha de comandos escreve, e
regressa de imediato, sem abrir uma janela. Código de saída `0`.

**`subedit <version>` sozinho na primeira linha**, depois uma linha por biblioteca com que a
janela funciona, com a versão encontrada em tempo de execução e, entre parênteses, aquela
com que foi compilada; depois os idiomas da interface instalados, por código. Numa
máquina com Qt 6.4.2:

```console
$ subedit-gui --version
subedit <version>
  ICU      74.2 (built with 74.2)
  Enchant  2.3.3 (built with 2.3.3)
  Qt       6.4.2 (built with 6.4.2)
  libmpv   2.2 (built with 2.2)
  Languages: en, fr
```

Os números não são copiados para aqui: são os da máquina, e este manual não tem nenhum
bloco gerado que mantivesse atualizado o do próprio programa. As bibliotecas são
as da linha de comandos, ICU e Enchant, e as duas que só a janela liga — Qt, e
libmpv, o reprodutor, indicada pela versão da sua API de cliente, a que o `mpv.pc`
anuncia. O resto está descrito em
[o manual da linha de comandos](../subedit-cli/invocation.md#--version). As palavras
`built with` e `Languages` seguem o idioma da janela; nada mais o faz.

`Ajuda ▸ Sobre o subedit` mostra as mesmas bibliotecas, a partir da mesma lista, numa tabela —
o nome, a versão carregada, a versão com que foi compilada, uma linha cada uma —, depois os
idiomas por baixo; os seus cabeçalhos seguem o idioma da janela.

## Abrir um ficheiro

```console
$ subedit-gui film.srt
```

Os formatos lidos são **os nove** que o filtro de abertura enumera — veja-se
[Abrir e guardar](files.md#abrir). O formato é reconhecido pelo conteúdo e
não pela extensão, e **a codificação também**: uma BOM
se o ficheiro tiver uma, caso contrário a codificação que os seus bytes sugerem.

## Quando a abertura falha

Um ficheiro em falta, ilegível, cujos bytes não se decodificam em nenhuma
codificação, ou cujo conteúdo nenhum formato reconhece: **a janela abre
na mesma, vazia**, e a falha é escrita no fluxo de erros.

**A mensagem nomeia a causa**, com as mesmas palavras que a linha de comandos:

| Mensagem | O que aconteceu |
| :------ | :------------ |
| `does not exist` | o caminho não designa nenhum ficheiro |
| `cannot be opened: permission denied` | o sistema recusa abri-lo |
| `cannot be read` | o sistema recusou por outro motivo |
| `cannot be decoded in the chosen encoding` | os bytes não se leem na codificação retida |
| `is in no format this tool knows` | nenhum formato reconhece o conteúdo |
| `holds nothing recognisable as a subtitle` | o formato é reconhecido, mas nada nele é uma legenda |

```console
$ subedit-gui notes.txt
subedit-gui: notes.txt: is in no format this tool knows
```

Abrir vazia em vez de recusar arrancar: a ferramenta continua a servir para
alguma coisa, e a falha de um argumento não é razão para não a lançar.

**Um ficheiro mal formado mas legível abre**, e o que a leitura apanhou
pelo caminho pode ler-se num botão da barra de estado — veja-se
[os diagnósticos de uma leitura](files.md#os-diagnósticos-de-uma-leitura).

## Códigos de saída

| Código | Quando |
| :--- | :--- |
| `0` | a janela abriu e depois foi fechada normalmente, ou `--version` respondeu |
| `2` | um erro inesperado antes de abrir — Qt em falta, ecrã indisponível |

A falha ao abrir um ficheiro **não altera o código de saída**: a
janela abriu, e é isso que o código comunica.

## A página de manual

Uma instalação coloca `subedit-gui(1)` em `share/man/man1`:

```console
$ man subedit-gui
```

Dá a forma de chamada, `--version`, as mensagens de falha de abertura, os
códigos de saída, as duas variáveis de ambiente e o ficheiro de definições.
**Não descreve a janela** — este manual fá-lo, e a página
remete para aqui.

## A plataforma Qt

`QT_QPA_PLATFORM` escolhe aquilo em que a janela funciona. **Se a variável
estiver definida, `subedit-gui` respeita-a sem argumento.**

Se não estiver, deixa o Qt escolher: o reprodutor integrado desenha a imagem na
própria janela, e não pede nada à plataforma — veja-se [O reprodutor](player.md).

`QT_QPA_PLATFORM=offscreen` executa a janela onde não há
servidor gráfico — que é o que a integração contínua e os testes fazem.

## Idioma

A janela fala o idioma do seu ambiente, **escolhido uma só vez, quando arranca**: mudar as
variáveis de uma janela em execução não altera nada até ao arranque seguinte. As variáveis e as regras
são as da linha de comandos — `LANGUAGE`, `LC_ALL`, `LC_MESSAGES`, `LANG`, `LC_ALL=C` para
inglês, inglês **sem uma palavra** quando o idioma pedido não está instalado — descritas em
[o manual da linha de comandos](../subedit-cli/invocation.md#idioma). Os catálogos são os mesmos
ficheiros, instalados em `<prefix>/share/subedit/locale`.

**Um idioma escolhido em `Editar ▸ Preferências…` tem prioridade**, e o ambiente só é lido
quando é `System default`, o que acontece até que se escolha um: um idioma instalado escolhido aí
prevalece sobre qualquer variável, `LC_ALL=C` incluída. A escolha, também, aplica-se no
arranque seguinte. Veja-se
[O idioma da interface](preferences.md#o-idioma-da-interface).

| O quê | Segue o idioma |
| :--- | :------------------- |
| os menus e as suas entradas, com as letras sublinhadas; a barra de ferramentas; as dicas de ferramenta | sim |
| as colunas da tabela e de `View ▸ Columns`, os separadores, a linha de estado, a barra de reprodução | sim |
| os botões padrão que o próprio Qt desenha (`OK`, `Cancel`) | sim, quando o catálogo do próprio Qt desse idioma está instalado (`qtbase_<language>.qm`); inglês no caso contrário |
| as caixas de diálogo, as páginas de `Correct Texts…`, os títulos e filtros das caixas de ficheiros, as caixas de mensagem e o que dizem | sim |
| as linhas que a janela escreve no seu fluxo de erros, `subedit-gui: …` | sim, exceto o nome do programa |
| os nomes dos padrões de correção em `Correct Texts…`, e as suas descrições nas dicas de ferramenta | sim, para os padrões distribuídos com o programa; um padrão seu é mostrado como o seu ficheiro o escreve |
| os nomes das codificações, dos fins de linha e dos idiomas de ortografia (`UTF-8`, `CRLF`, `en_US`) | nunca: são nomes, não palavras |
| os atalhos de teclado | nunca: `Ctrl+S` é `Ctrl+S` em todos os idiomas |
| uma mensagem que o catálogo não traduz | mostrada em inglês |

Esta versão instala `ca`, `cs`, `de`, `es`, `fi`, `fr`, `gl`, `he`, `hu`, `ie`, `is`, `it_IT`, `nl`, `pl`, `pt`, `pt_BR`, `ru`, `sr`, `tr` e `zh_CN` — catalão, checo, alemão,
espanhol, finlandês, francês, galego, hebraico, húngaro, interlingue, islandês, italiano, neerlandês, polaco, português, português do Brasil, russo, sérvio, turco e chinês simplificado — traduzidos na totalidade. O francês foi revisto por um leitor do
idioma; o catalão, o checo, o alemão, o espanhol, o finlandês, o galego, o hebraico, o húngaro, o interlingue, o islandês, o italiano, o neerlandês, o polaco, o português, o português do Brasil, o russo, o sérvio, o turco e o chinês simplificado são traduções automáticas e ainda não foram
revistas por nenhum leitor do idioma.
<!-- machine-translated: ca -->
<!-- machine-translated: cs -->
<!-- machine-translated: de -->
<!-- machine-translated: es -->
<!-- machine-translated: fi -->
<!-- machine-translated: gl -->
<!-- machine-translated: he -->
<!-- machine-translated: hu -->
<!-- machine-translated: ie -->
<!-- machine-translated: is -->
<!-- machine-translated: it_IT -->
<!-- machine-translated: nl -->
<!-- machine-translated: pl -->
<!-- machine-translated: pt -->
<!-- machine-translated: pt_BR -->
<!-- machine-translated: ru -->
<!-- machine-translated: sr -->
<!-- machine-translated: tr -->
<!-- machine-translated: zh_CN -->
Num ambiente de trabalho francês, a barra de menus é
`&Fichier`, `É&dition`, `&Affichage`, `&Vidéo`, `&Outils`, `&Projets`, `Aid&e`. As capturas de ecrã deste manual foram feitas em português.
