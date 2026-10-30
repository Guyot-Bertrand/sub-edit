# subedit — manual

<!-- machine-translated -->
> Esta página é uma tradução automática e ainda não foi revista por nenhum leitor do idioma.

O subedit lê, inspeciona e resincroniza ficheiros de legendas. Apresenta-se como dois
programas que partilham o mesmo núcleo e, portanto, as mesmas regras e as mesmas palavras.

| Programa | Para quê | O seu manual |
| :-------- | :-------------- | :--------- |
| `subedit-gui` | abrir um ficheiro, vê-lo numa tabela, editá-lo, acompanhá-lo com o filme | [o manual da janela](subedit-gui/index.md) |
| `subedit-cli` | processar ficheiros em lotes, a partir de um script, sem interface | [o manual da linha de comandos](subedit-cli/index.md) |

Ambos leem e escrevem **os nove formatos** — SubRip, WebVTT, SubViewer 2,
Sub Station Alpha, Advanced SSA, MicroDVD, MPL2, TMPlayer, LRC — e
reconhecem o formato pelo seu conteúdo e não pela sua extensão. **A codificação também
é reconhecida pelo conteúdo**: um ficheiro em Latin-1, em CP1252 ou em UTF-16
abre-se, diz-se como foi lido e volta a ser escrito na sua própria codificação — um
ficheiro aberto e depois guardado sem alterações não se move um único byte.

## Por onde começar

| O que se pretende | Para onde ir |
| :----------------- | :------- |
| instalar a ferramenta | [Instalação](subedit-cli/installation.md) — vale para os dois programas |
| abrir um ficheiro e ver o que contém | [A tabela](subedit-gui/table.md) |
| resincronizar legendas desfasadas de uma quantidade constante | [As operações](subedit-gui/operations.md#shift-positions), ou [`shift`](subedit-cli/shift.md) |
| resincronizar um ficheiro escrito para outra taxa de fotogramas | [`framerate`](subedit-cli/framerate.md), e [`snap`](subedit-cli/snap.md) se a sincronização já estiver certa |
| saber contra que taxa de fotogramas um ficheiro foi escrito | [A grelha de fotogramas](subedit-gui/frame-grid.md), ou [`inspect`](subedit-cli/inspect.md) |
| remover as menções para deficientes auditivos | [As operações](subedit-gui/operations.md#remove-hearing-impaired-mentions), ou [`hearing-impaired`](subedit-cli/hearing-impaired.md) |
| corrigir menções, erros comuns, maiúsculas e quebra de linhas numa única passagem | [`Correct Texts…`](subedit-gui/correct-texts.md), ou [`correct`](subedit-cli/correct.md) |
| verificar a ortografia palavra a palavra, com os dicionários do sistema | [`Check Spelling…`](subedit-gui/check-spelling.md) |
| alongar legendas demasiado curtas para serem lidas | [`Adjust Durations…`](subedit-gui/operations.md#adjust-durations), ou [`adjust`](subedit-cli/adjust.md) |
| repor em ordem um ficheiro cujas legendas não o estão | [`sort`](subedit-cli/sort.md) |
| fazer uma única legenda das partes de um mesmo texto que se seguem | [`Join Repeated Subtitles`](subedit-gui/lines.md#unir-legendas-repetidas), ou [`join-repeats`](subedit-cli/join-repeats.md) |
| endireitar, de uma só vez, um ficheiro mal segmentado, fora de ordem e com sobreposições | [`repair`](subedit-cli/repair.md) |
| remover as sobreposições entre legendas | [`Make Ranges Disjoint…`](subedit-gui/lines.md#tornar-os-intervalos-disjuntos), ou [`disjoint`](subedit-cli/disjoint.md) |
| pôr as duas partes de um filme uma a seguir à outra | [`Append File…`](subedit-gui/operations.md#append-file), ou [`append`](subedit-cli/append.md) |
| cortar um filme em duas partes | [`Split Project…`](subedit-gui/operations.md#split-project), ou [`split-file`](subedit-cli/split-file.md) |
| alterar as maiúsculas e minúsculas, o itálico ou os hífens de diálogo de um ficheiro inteiro | [As operações](subedit-gui/operations.md#italic), ou [`case`](subedit-cli/case.md), [`italics`](subedit-cli/italics.md), [`dialogue-dashes`](subedit-cli/dialogue-dashes.md) |
| procurar uma palavra e substituí-la, sem quebrar as etiquetas | [Localizar e substituir](subedit-gui/find-and-replace.md), ou [`replace`](subedit-cli/replace.md) |
| escrever a fala de um filme como legendas | [`Transcribe Audio…`](subedit-gui/video.md#transcrever-o-áudio) ou [`transcribe`](subedit-cli/transcribe.md) — depois de [instalar as ferramentas](subedit-cli/install-transcription.md) |
| reajustar um ficheiro de legendas pelo som de um filme | [`Synchronize with Audio…`](subedit-gui/video.md#sincronizar-com-o-áudio) ou [`sync`](subedit-cli/sync.md) — depois de [instalar as ferramentas](subedit-cli/install-transcription.md) |
| traduzir ao lado do texto original | [Abrir uma tradução](subedit-gui/files.md#abrir-uma-tradução), ou [`pair`](subedit-cli/pair.md) para a resincronizar pelo ficheiro principal |
| processar uma centena de ficheiros de uma só vez | [Processar uma árvore de pastas](subedit-cli/lots.md), depois [Invocação](subedit-cli/invocation.md) |

**Entre `framerate` e `snap`, um engano passa despercebido.** Em caso de
dúvida, [`snap`](subedit-cli/snap.md#snap-não-é-framerate-e-confundi-los-não-se-nota)
abre com a comparação dos dois.

## O que este manual promete

**Descreve o que existe, nunca o que está planeado.** O que virá, e
por que ordem, está no [roadmap](../../../feuille-de-route.md).

**Este manual está escrito em inglês e traduzido.** As suas traduções seguem-no
página a página, na pasta `docs/i18n/` do repositório, uma árvore por idioma. Os
programas falam o idioma do seu ambiente; os nomes que mostram — subcomandos,
opções, rótulos de menus — são aqui citados tal como o programa os escreve em inglês.

**Os exemplos e as imagens são gerados, o texto não.** Os blocos
`console` são produzidos executando realmente o comando; as capturas de
ecrã, construindo a janela verdadeira e fotografando-a. Nenhum
dos dois pode, portanto, descrever uma ferramenta que já não exista. O que fica sem rede
é o texto à volta, cuja correção assenta na revisão.

**As capturas de ecrã mostram as duas paletas**, `Light` e `Dark` — as que
[as preferências](subedit-gui/preferences.md#o-tema) oferecem.

## As duas interfaces dizem as mesmas palavras

Uma razão para ler qualquer um dos manuais quando se usa o outro: as
mensagens de erro, os nomes dos formatos, os veredictos da grelha e os rótulos de
anulação são escritos **uma só vez**, no núcleo, e ambos os programas
os citam. Um ficheiro recusado dá a mesma frase dos dois lados.

O que difere é o que cada interface sabe fazer:

| | `subedit-gui` | `subedit-cli` |
| :--- | :------------ | :------------ |
| vários ficheiros de uma vez | sim — um separador por projeto | sim |
| uma tradução ao lado do ficheiro principal | sim | sim — `-t` nos subcomandos de texto, e [`pair`](subedit-cli/pair.md) para a resincronizar |
| anexar um ficheiro, dividir um projeto | sim | sim — [`append`](subedit-cli/append.md), [`split-file`](subedit-cli/split-file.md) |
| anular | sim, mil entradas | não aplicável — nada é alterado no próprio ficheiro sem que se peça |
| editar um texto ou uma posição à mão | sim | não |
| inserir, apagar, unir e dividir linhas | sim | não |
| localizar e substituir | sim | sim — [`replace`](subedit-cli/replace.md) |
| cortar, copiar e colar textos | sim | não |
| ajustar as durações, o itálico, as maiúsculas e minúsculas, os hífens | sim | sim — [`adjust`](subedit-cli/adjust.md), [`italics`](subedit-cli/italics.md), [`case`](subedit-cli/case.md), [`dialogue-dashes`](subedit-cli/dialogue-dashes.md) |
| corrigir os textos por padrões | sim — `Correct Texts…` | sim — [`correct`](subedit-cli/correct.md) |
| verificar a ortografia palavra a palavra | sim — `Check Spelling…` | não |
| acompanhar o filme enquanto se sincroniza | sim | não |
| escrever por cima da entrada | `Save` | `--in-place` |
