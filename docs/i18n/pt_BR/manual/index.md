# subedit — manual

<!-- machine-translated -->
> Esta página é uma tradução automática e ainda não foi revisada por um leitor do idioma.

O subedit lê, inspeciona e ressincroniza arquivos de legendas. Ele vem em dois
programas que compartilham o mesmo núcleo e, portanto, as mesmas regras e as mesmas palavras.

| Programa | Para quê | Seu manual |
| :-------- | :-------------- | :--------- |
| `subedit-gui` | abrir um arquivo, vê-lo em uma tabela, editá-lo, acompanhá-lo junto com o filme | [o manual da janela](subedit-gui/index.md) |
| `subedit-cli` | processar arquivos em lote, a partir de um script, sem interface | [o manual da linha de comando](subedit-cli/index.md) |

Ambos leem e escrevem **os nove formatos** — SubRip, WebVTT, SubViewer 2,
Sub Station Alpha, Advanced SSA, MicroDVD, MPL2, TMPlayer, LRC — e
reconhecem o formato pelo conteúdo, e não pela extensão. **A codificação também
é reconhecida pelo conteúdo**: um arquivo em Latin-1, em CP1252 ou em UTF-16
é aberto, informa-se como foi lido e ele é gravado de volta na própria codificação — um
arquivo aberto e depois salvo sem alterações não muda nem um byte.

## Por onde começar

| O que se quer | Aonde ir |
| :----------------- | :------- |
| instalar a ferramenta | [Instalação](subedit-cli/installation.md) — vale para os dois programas |
| abrir um arquivo e ver o que ele contém | [A tabela](subedit-gui/table.md) |
| ressincronizar legendas deslocadas por um valor constante | [As operações](subedit-gui/operations.md#shift-positions) ou [`shift`](subedit-cli/shift.md) |
| ressincronizar um arquivo escrito para outra taxa de quadros | [`framerate`](subedit-cli/framerate.md) e [`snap`](subedit-cli/snap.md), se a sincronização já estiver correta |
| saber com qual taxa de quadros um arquivo foi escrito | [A grade de quadros](subedit-gui/frame-grid.md) ou [`inspect`](subedit-cli/inspect.md) |
| remover as menções para deficientes auditivos | [As operações](subedit-gui/operations.md#remove-hearing-impaired-mentions) ou [`hearing-impaired`](subedit-cli/hearing-impaired.md) |
| corrigir menções, erros comuns, uso de maiúsculas e quebras de linha em uma única passagem | [`Correct Texts…`](subedit-gui/correct-texts.md) ou [`correct`](subedit-cli/correct.md) |
| verificar a ortografia palavra por palavra, com os dicionários do sistema | [`Check Spelling…`](subedit-gui/check-spelling.md) |
| alongar legendas curtas demais para serem lidas | [`Adjust Durations…`](subedit-gui/operations.md#adjust-durations) ou [`adjust`](subedit-cli/adjust.md) |
| recolocar em ordem um arquivo cujas legendas estão fora de ordem | [`sort`](subedit-cli/sort.md) |
| formar uma única legenda com os pedaços de um mesmo texto que se sucedem | [`Join Repeated Subtitles`](subedit-gui/lines.md#unir-legendas-repetidas) ou [`join-repeats`](subedit-cli/join-repeats.md) |
| endireitar de uma só vez um arquivo mal segmentado, fora de ordem e com sobreposições | [`repair`](subedit-cli/repair.md) |
| remover as sobreposições entre legendas | [`Make Ranges Disjoint…`](subedit-gui/lines.md#tornar-os-intervalos-disjuntos) ou [`disjoint`](subedit-cli/disjoint.md) |
| colocar as duas partes de um filme uma após a outra | [`Append File…`](subedit-gui/operations.md#append-file) ou [`append`](subedit-cli/append.md) |
| cortar um filme em duas partes | [`Split Project…`](subedit-gui/operations.md#split-project) ou [`split-file`](subedit-cli/split-file.md) |
| mudar as maiúsculas e minúsculas, o itálico ou os travessões de diálogo de um arquivo inteiro | [As operações](subedit-gui/operations.md#italic) ou [`case`](subedit-cli/case.md), [`italics`](subedit-cli/italics.md), [`dialogue-dashes`](subedit-cli/dialogue-dashes.md) |
| procurar uma palavra e substituí-la, sem quebrar as marcações | [Localizar e substituir](subedit-gui/find-and-replace.md) ou [`replace`](subedit-cli/replace.md) |
| escrever a fala de um filme como legendas | [`Transcribe Audio…`](subedit-gui/video.md#transcrever-o-áudio) ou [`transcribe`](subedit-cli/transcribe.md) — depois de [instalar as ferramentas](subedit-cli/install-transcription.md) |
| reajustar o tempo de um arquivo de legendas sobre o áudio de um filme | [`sync`](subedit-cli/sync.md) — depois de [instalar as ferramentas](subedit-cli/install-transcription.md) |
| traduzir ao lado do texto original | [Abrir uma tradução](subedit-gui/files.md#abrir-uma-tradução) ou [`pair`](subedit-cli/pair.md) para ressincronizá-la com base no arquivo principal |
| processar cem arquivos de uma vez | [Processar uma árvore de diretórios](subedit-cli/lots.md) e depois [Invocação](subedit-cli/invocation.md) |

**Entre `framerate` e `snap`, um erro passa despercebido.** Em caso de
dúvida, [`snap`](subedit-cli/snap.md#snap-não-é-framerate-e-confundi-los-não-aparece)
começa com a comparação dos dois.

## O que este manual promete

**Ele descreve o que existe, nunca o que está planejado.** O que virá, e em
que ordem, está no [roteiro](../../../feuille-de-route.md).

**Este manual é escrito em inglês e traduzido.** Suas traduções o acompanham
página por página, no diretório `docs/i18n/` do repositório, uma árvore por idioma. Os
programas falam o idioma do ambiente; os nomes que eles exibem — subcomandos,
opções, rótulos de menu — são citados aqui como o programa os escreve em inglês.

**Os exemplos e as imagens são gerados, o texto não.** Os
blocos `console` são produzidos executando de fato o comando; as
capturas de tela, construindo a janela real e fotografando-a. Nenhum
dos dois pode, portanto, descrever uma ferramenta que não existe mais. O que fica sem rede de proteção
é o texto ao redor, cuja correção depende da revisão.

**As capturas de tela mostram as duas paletas**, `Light` e `Dark` — as que
[as preferências](subedit-gui/preferences.md#o-tema) oferecem.

## As duas superfícies dizem as mesmas palavras

Um motivo para ler qualquer um dos dois manuais ao usar o outro programa: as
mensagens de erro, os nomes dos formatos, os veredictos sobre a grade e os rótulos
de desfazer são escritos **uma única vez**, no núcleo, e os dois programas
os citam. Um arquivo recusado gera a mesma frase nos dois lados.

O que muda é o que cada superfície pode fazer:

| | `subedit-gui` | `subedit-cli` |
| :--- | :------------ | :------------ |
| vários arquivos de uma vez | sim — uma aba por projeto | sim |
| uma tradução ao lado do arquivo principal | sim | sim — `-t` nos subcomandos de texto e [`pair`](subedit-cli/pair.md) para ressincronizá-la |
| anexar um arquivo, dividir um projeto | sim | sim — [`append`](subedit-cli/append.md), [`split-file`](subedit-cli/split-file.md) |
| desfazer | sim, mil entradas | não se aplica — nada é alterado no próprio arquivo sem que se peça |
| editar um texto ou uma posição manualmente | sim | não |
| inserir, excluir, unir e dividir linhas | sim | não |
| localizar e substituir | sim | sim — [`replace`](subedit-cli/replace.md) |
| recortar, copiar e colar textos | sim | não |
| ajustar as durações, o itálico, as maiúsculas e minúsculas, os travessões | sim | sim — [`adjust`](subedit-cli/adjust.md), [`italics`](subedit-cli/italics.md), [`case`](subedit-cli/case.md), [`dialogue-dashes`](subedit-cli/dialogue-dashes.md) |
| corrigir os textos por padrões | sim — `Correct Texts…` | sim — [`correct`](subedit-cli/correct.md) |
| verificar a ortografia palavra por palavra | sim — `Check Spelling…` | não |
| assistir ao filme durante a sincronização | sim | não |
| gravar sobre a entrada | `Save` | `--in-place` |
