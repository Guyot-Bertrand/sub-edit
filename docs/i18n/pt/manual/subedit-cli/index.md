# subedit-cli — manual

<!-- machine-translated -->
> Esta página é uma tradução automática e ainda não foi revista por nenhum leitor do idioma.

A ferramenta de linha de comandos do subedit: trabalhar com ficheiros de legendas sem interface
gráfica, em lotes ou a partir de um script. Para o outro programa e para saber
por onde começar, veja
[o manual](../index.md).

> **Estado atual.** Os dezassete subcomandos de `subedit-cli` existem: inspecionar,
> converter, deslocar, transformar, resincronizar a taxa de fotogramas, alinhar a uma grelha de fotogramas, remover
> as menções para deficientes auditivos, ajustar as durações, substituir um texto, alterar as maiúsculas e minúsculas,
> o itálico e os hífens de diálogo, voltar a pôr as legendas em ordem, corrigir os
> textos com os padrões do Gaupol, pôr ficheiros uns a seguir aos outros, cortar um
> ficheiro em dois e escrever uma tradução nas posições do seu ficheiro principal. Este manual
> descreve o que existe, nunca o que está planeado; o que vem a seguir está no
> [roadmap](../../../../feuille-de-route.md).

## Secções

| Secção | Conteúdo |
| :------ | :------ |
| [Instalação](installation.md) | compilar e instalar a ferramenta |
| [Invocação](invocation.md) | opções globais, saídas, códigos de saída |
| [O ficheiro de definições](invocation.md#o-ficheiro-de-definições) | o que `adjust` e `correct` leem do ficheiro de definições da janela, `--config`, `--no-config` |
| [Processar uma árvore de pastas](lots.md) | vários ficheiros, uma pasta inteira, `--recursive` |
| [`inspect`](inspect.md) | comunicar de que é feito um ficheiro |
| [`convert`](convert.md) | escrever um ficheiro noutro formato ou noutra forma |
| [`shift`](shift.md) | mover todas as posições da mesma quantidade |
| [`transform`](transform.md) | corrigir todas as posições a partir de dois pontos sabidamente certos |
| [`framerate`](framerate.md) | resincronizar um ficheiro de uma taxa de fotogramas para outra |
| [`snap`](snap.md) | repor os instantes sobre os fotogramas de uma taxa de fotogramas |
| [`hearing-impaired`](hearing-impaired.md) | remover as menções para deficientes auditivos |
| [`adjust`](adjust.md) | ajustar as durações a uma velocidade de leitura e a limites |
| [`replace`](replace.md) | substituir um texto sem quebrar uma etiqueta |
| [`case`](case.md) | pôr os textos em iniciais maiúsculas, maiúscula na frase, maiúsculas ou minúsculas |
| [`italics`](italics.md) | pôr os textos em itálico, ou retirar-lhes o itálico |
| [`dialogue-dashes`](dialogue-dashes.md) | pôr hífens de diálogo, ou retirá-los |
| [`sort`](sort.md) | repor as legendas pela ordem do seu início |
| [`join-repeats`](join-repeats.md) | fazer uma única legenda das partes de um mesmo texto que se seguem |
| [`disjoint`](disjoint.md) | cortar as legendas sobrepostas para que nenhum intervalo se sobreponha a outro |
| [`repair`](repair.md) | encadear `join-repeats`, `sort` e `disjoint` num ficheiro mal segmentado, fora de ordem e com sobreposições |
| [`correct`](correct.md) | corrigir os textos com os padrões do Gaupol: menções, juntar e dividir palavras, erros comuns, utilização de maiúsculas, quebra de linhas |
| [`append`](append.md) | pôr ficheiros depois do primeiro, num único ficheiro |
| [`split-file`](split-file.md) | cortar um ficheiro em dois, com a parte final trazida de volta à origem |
| [`pair`](pair.md) | escrever uma tradução nas posições do seu ficheiro principal e dizer como as suas linhas se alinharam |
| [`config`](config.md) | listar, mostrar e alterar as definições das operações partilhadas com a janela |
