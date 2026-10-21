# Invocação

<!-- machine-translated -->
> Esta página é uma tradução automática e ainda não foi revisada por um leitor do idioma.

```
subedit-gui [--version] [<file>]
```

| Argumento | Obrigatório | Valor | Padrão |
| :------- | :------- | :---- | :------ |
| `<file>` | não | o caminho de um arquivo de legendas a abrir | nenhum — a janela abre vazia |
| `--version` | não | — | — |

**Um único arquivo.** Os argumentos seguintes são ignorados: a janela mantém
apenas um projeto por vez.

## `--version`

Escreve na saída padrão o bloco que o `--version` da linha de comando escreve, e
termina em seguida, sem abrir janela. Código de saída `0`.

**`subedit <version>` sozinho na primeira linha**, depois uma linha por biblioteca com a qual a
janela funciona, com a versão encontrada em tempo de execução e, entre parênteses, a
versão com a qual foi compilada; depois os idiomas de interface instalados, por código. Em
uma máquina com Qt 6.4.2:

```console
$ subedit-gui --version
subedit <version>
  ICU      74.2 (built with 74.2)
  Enchant  2.3.3 (built with 2.3.3)
  Qt       6.4.2 (built with 6.4.2)
  libmpv   2.2 (built with 2.2)
  Languages: en, fr
```

Os números não são copiados aqui: são os da máquina, e este manual não tem nenhum
bloco gerado que mantivesse atualizados os do próprio programa. As bibliotecas são
as da linha de comando, ICU e Enchant, e as duas que somente a janela liga — Qt, e
libmpv, o reprodutor, indicada pela versão de sua API cliente, a que o `mpv.pc`
anuncia. O restante é descrito em
[o manual da linha de comando](../subedit-cli/invocation.md#--version). As palavras
`built with` e `Languages` seguem o idioma da janela; nada mais o segue.

`Ajuda ▸ Sobre o subedit` mostra as mesmas bibliotecas, da mesma lista, em uma tabela —
o nome, a versão carregada, a versão com a qual foi compilada, uma linha cada —, depois os
idiomas abaixo; seus cabeçalhos seguem o idioma da janela.

## Abrir um arquivo

```console
$ subedit-gui film.srt
```

Os formatos lidos são **os nove** que o filtro de abertura lista — veja
[Abrir e salvar](files.md#abertura). O formato é reconhecido pelo conteúdo e
não pela extensão, e **a codificação também**: um BOM
se o arquivo tiver um, senão a codificação que seus bytes sugerem.

## Quando a abertura falha

Um arquivo ausente, ilegível, cujos bytes não se decodificam em nenhuma
codificação, ou cujo conteúdo nenhum formato reconhece: **a janela abre
mesmo assim, vazia**, e a falha é escrita no fluxo de erro.

**A mensagem nomeia a causa**, com as mesmas palavras da linha de comando:

| Mensagem | O que aconteceu |
| :------ | :------------ |
| `does not exist` | o caminho não designa nenhum arquivo |
| `cannot be opened: permission denied` | o sistema se recusa a abri-lo |
| `cannot be read` | o sistema recusou por outro motivo |
| `cannot be decoded in the chosen encoding` | os bytes não se leem na codificação adotada |
| `is in no format this tool knows` | nenhum formato reconhece o conteúdo |
| `holds nothing recognisable as a subtitle` | o formato é reconhecido, mas nada nele é uma legenda |

```console
$ subedit-gui notes.txt
subedit-gui: notes.txt: is in no format this tool knows
```

Abrir vazia em vez de se recusar a iniciar: a ferramenta ainda serve para
alguma coisa, e a falha de um argumento não é motivo para não iniciar.

**Um arquivo malformado, mas legível, abre**, e o que a leitura recuperou
pelo caminho pode ser lido em um botão da barra de status — veja
[os diagnósticos de uma leitura](files.md#os-diagnósticos-de-uma-leitura).

## Códigos de saída

| Código | Quando |
| :--- | :--- |
| `0` | a janela abriu e depois foi fechada normalmente, ou `--version` respondeu |
| `2` | um erro inesperado antes da abertura — Qt ausente, tela indisponível |

A falha ao abrir um arquivo **não muda o código de saída**: a
janela abriu, e é isso que o código relata.

## A página de manual

Uma instalação coloca `subedit-gui(1)` em `share/man/man1`:

```console
$ man subedit-gui
```

Ela dá a forma de chamada, `--version`, as mensagens de falha de abertura, os
códigos de saída, as duas variáveis de ambiente e o arquivo de configurações.
**Ela não descreve a janela** — este manual o faz, e a página
remete a ele.

## A plataforma Qt

`QT_QPA_PLATFORM` escolhe sobre o que a janela é executada. **Se a variável
estiver definida, `subedit-gui` a respeita sem argumento.**

Se não estiver, deixa o Qt escolher: o reprodutor embutido desenha a imagem na
própria janela, e não pede nada à plataforma — veja
[O reprodutor](player.md).

`QT_QPA_PLATFORM=offscreen` executa a janela onde não há
servidor gráfico — que é o que a integração contínua e os testes fazem.

## Idioma

A janela fala o idioma de seu ambiente, **escolhido uma vez, quando ela inicia**: mudar as
variáveis de uma janela em execução não muda nada até a próxima inicialização. As variáveis e as regras
são as da linha de comando — `LANGUAGE`, `LC_ALL`, `LC_MESSAGES`, `LANG`, `LC_ALL=C` para
o inglês, o inglês **sem uma palavra** quando o idioma pedido não está instalado — descritas em
[o manual da linha de comando](../subedit-cli/invocation.md#idioma). Os catálogos são os mesmos
arquivos, instalados em `<prefix>/share/subedit/locale`.

**Um idioma escolhido em `Editar ▸ Preferências…` vem primeiro**, e o ambiente só é lido
quando ele é `System default`, o que ocorre até que um seja escolhido: um idioma instalado escolhido ali
prevalece sobre qualquer variável, `LC_ALL=C` inclusive. A escolha também vale a partir da próxima inicialização. Veja
[O idioma da interface](preferences.md#o-idioma-da-interface).

| O quê | Segue o idioma |
| :--- | :------------------- |
| os menus e suas entradas, com suas letras sublinhadas; a barra de ferramentas; as dicas de ferramenta | sim |
| as colunas da tabela e de `Ver ▸ Colunas`, as abas, a linha de status, a barra de reprodução | sim |
| os botões padrão que o próprio Qt desenha (`OK`, `Cancel`) | sim, quando o catálogo do próprio Qt desse idioma está instalado (`qtbase_<language>.qm`); inglês caso contrário |
| as caixas de diálogo, as páginas de `Correct Texts…`, os títulos e filtros das caixas de arquivo, as caixas de mensagem e o que elas dizem | sim |
| as linhas que a janela escreve em seu fluxo de erro, `subedit-gui: …` | sim, exceto o nome do programa |
| os nomes dos padrões de correção em `Correct Texts…`, e suas descrições nas dicas de ferramenta | sim, para os padrões distribuídos com o programa; um padrão próprio é mostrado como seu arquivo o escreve |
| os nomes de codificações, de finais de linha e de idiomas de ortografia (`UTF-8`, `CRLF`, `en_US`) | nunca: são nomes, não palavras |
| os atalhos de teclado | nunca: `Ctrl+S` é `Ctrl+S` em todos os idiomas |
| uma mensagem que o catálogo não traduz | mostrada em inglês |

Esta versão instala `ca`, `cs`, `de`, `es`, `fi`, `fr`, `gl`, `he`, `hu`, `ie`, `is`, `it_IT`, `nl`, `pl`, `pt`, `pt_BR`, `ru`, `sr`, `tr` e `zh_CN` — catalão, tcheco, alemão,
espanhol, finlandês, francês, galego, hebraico, húngaro, interlingue, islandês, italiano, holandês, polonês, português, português do Brasil, russo, sérvio, turco e chinês simplificado — traduzidos por completo. O francês foi revisado por um leitor do
idioma; o catalão, o tcheco, o alemão, o espanhol, o finlandês, o galego, o hebraico, o húngaro, o interlingue, o islandês, o italiano, o holandês, o polonês, o português, o português do Brasil, o russo, o sérvio, o turco e o chinês simplificado são traduções automáticas ainda não
revisadas por um leitor do idioma.
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
Em uma área de trabalho em francês, a barra de menus exibe
`&Fichier`, `É&dition`, `&Affichage`, `&Vidéo`, `&Outils`, `&Projets`, `Aid&e`. As capturas de tela
deste manual são tiradas em português do Brasil.
