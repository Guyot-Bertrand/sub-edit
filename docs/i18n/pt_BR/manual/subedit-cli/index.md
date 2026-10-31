# subedit-cli — manual

<!-- machine-translated -->
> Esta página é uma tradução automática e ainda não foi revisada por um leitor do idioma.

A ferramenta de linha de comando do subedit: trabalhar com arquivos de legendas sem interface
gráfica, em lote ou a partir de um script. Para o outro programa e para saber
por onde começar, veja
[o manual](../index.md).

> **Estado atual.** Os dezessete subcomandos do `subedit-cli` existem: inspecionar,
> converter, deslocar, transformar, ressincronizar a taxa de quadros, alinhar a uma grade de quadros, remover
> as menções para deficientes auditivos, ajustar as durações, substituir um texto, mudar as maiúsculas e minúsculas,
> o itálico e os travessões de diálogo, recolocar as legendas em ordem, corrigir os
> textos com os padrões do Gaupol, colocar arquivos um após o outro, cortar um
> arquivo em dois e gravar uma tradução nas posições de seu arquivo principal. Este manual
> descreve o que existe, nunca o que está planejado; o que vem a seguir está no
> [roteiro](../../../../feuille-de-route.md).

## Seções

| Seção | Conteúdo |
| :------ | :------ |
| [Instalação](installation.md) | compilar e instalar a ferramenta |
| [Invocação](invocation.md) | opções globais, saídas, códigos de saída |
| [O arquivo de configurações](invocation.md#o-arquivo-de-configurações) | o que `adjust` e `correct` leem do arquivo de configurações da janela, `--config`, `--no-config` |
| [Processar uma árvore de diretórios](lots.md) | vários arquivos, um diretório inteiro, `--recursive` |
| [`inspect`](inspect.md) | relatar do que um arquivo é feito |
| [`convert`](convert.md) | gravar um arquivo em outro formato ou outra forma |
| [`shift`](shift.md) | mover todas as posições pelo mesmo valor |
| [`transform`](transform.md) | corrigir todas as posições a partir de dois pontos sabidamente corretos |
| [`framerate`](framerate.md) | ressincronizar um arquivo de uma taxa de quadros para outra |
| [`snap`](snap.md) | recolocar as marcas de tempo sobre os quadros de uma taxa de quadros |
| [`hearing-impaired`](hearing-impaired.md) | remover as menções para deficientes auditivos |
| [`adjust`](adjust.md) | ajustar as durações a uma velocidade de leitura e a limites |
| [`replace`](replace.md) | substituir um texto sem quebrar uma marcação |
| [`case`](case.md) | colocar os textos em iniciais maiúsculas, primeira letra maiúscula, maiúsculas ou minúsculas |
| [`italics`](italics.md) | colocar os textos em itálico ou retirar o itálico deles |
| [`dialogue-dashes`](dialogue-dashes.md) | colocar travessões de diálogo ou retirá-los |
| [`sort`](sort.md) | recolocar as legendas na ordem de seu início |
| [`join-repeats`](join-repeats.md) | formar uma única legenda com os pedaços de um mesmo texto que se sucedem |
| [`disjoint`](disjoint.md) | cortar as legendas que se sobrepõem para que nenhum intervalo se sobreponha mais a outro |
| [`repair`](repair.md) | encadear `join-repeats`, `sort` e `disjoint` em um arquivo mal segmentado, fora de ordem e com sobreposições |
| [`correct`](correct.md) | corrigir os textos com os padrões do Gaupol: menções, união e divisão de palavras, erros comuns, uso de maiúsculas, quebra de linhas |
| [`append`](append.md) | colocar arquivos após o primeiro, em um único arquivo |
| [`split-file`](split-file.md) | cortar um arquivo em dois, com a segunda parte trazida de volta à origem |
| [`pair`](pair.md) | gravar uma tradução nas posições de seu arquivo principal e dizer como suas linhas se alinharam |
| [`transcribe`](transcribe.md) | escrever a fala de um vídeo ou de um arquivo de áudio como legendas |
| [`sync`](sync.md) | reajustar o tempo das legendas de um arquivo sobre o áudio de um filme |
| [Instalar as ferramentas de transcrição](install-transcription.md) | o motor, o modelo e o `ffmpeg`, que o programa não fornece |
| [`config`](config.md) | listar, mostrar e alterar as configurações das operações compartilhadas com a janela |
