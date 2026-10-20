# `pair`

<!-- machine-translated -->
> Esta página é uma tradução automática e ainda não foi revisada por um leitor do idioma.

```
subedit-cli pair (--output FILE | --output-dir DIR | --in-place)
                 -t FILE [--align-method position|number]
                 [--dry-run]
                 <main>
```

Grava **a tradução ressincronizada nas posições do arquivo principal**, e informa como suas linhas encontraram sua
legenda. É o `Open Translation…` da janela — [o mesmo emparelhamento](../subedit-gui/files.md#abrir-uma-tradução),
a mesma frase — seguido da gravação do resultado, que a janela faz por meio de `Save Translation`.

**É a única coisa que o emparelhamento faz e que um arquivo processado sozinho não faz.** Os outros subcomandos
que aceitam `-t` alteram a tradução que leem — [`case`](case.md), [`replace`](replace.md)… —; `pair` não altera
nada no texto: ele **assenta** a tradução sobre o arquivo principal e grava o que o assentamento produziu. O arquivo principal é
lido para receber as linhas, **nunca gravado**.

- **O que se grava é a tradução**, em **seu próprio caminho e em seu próprio formato**: o formato, a codificação e
  os finais de linha do arquivo de tradução. Suas legendas ficam **nas posições do arquivo principal**: uma linha que
  caiu ao lado do alvo recebe de volta os limites da legenda que ela traduz.
- **`-t` é obrigatório.** `pair` sem tradução não tem nada a assentar: é um erro de uso, código `1`.
- **Um único arquivo principal**, e nenhum `--recursive`: um par são dois arquivos, e uma árvore não tem
  como dizer qual tradução acompanha qual arquivo principal. Para um lote, uma invocação por par.
- **`--output` nomeia o arquivo da tradução gravada, `--output-dir` coloca ali o seu nome de base, `--in-place`
  a reescreve**, como em qualquer subcomando que aceita `-t` — veja [Uma tradução](invocation.md#uma-tradução).
- **`--document` não existe**: há um único documento visado, a tradução.

<!-- example: subedit-cli pair --help -->
```console
$ subedit-cli pair --help
Write a translation at the positions of its main file, and say how it lined up
Usage: subedit-cli pair [OPTIONS] main

Positionals:
  main TEXT REQUIRED          The main subtitle file

Options:
  -h,--help                   Print this help message and exit
  -t,--translation-file FILE  Translation file to lay over the subtitle file, for a single input
  --align-method position|number
                              How the lines of the translation find their subtitles
  --output TEXT               File to write, for a single input
  --output-dir TEXT           Directory to write into
  --in-place                  Write back over the inputs
  --dry-run                   Work out and say what would be written, and write nothing
```

## Argumentos e opções

| Opção | Obrigatório | Valores | Padrão |
| :----- | :----- | :------ | :----- |
| `<main>` | sim | **um único** caminho | — |
| `-t`, `--translation-file` | **sim** | o arquivo de tradução | — |
| `--align-method` | não | `position` ou `number` — veja [Uma tradução](invocation.md#uma-tradução) | `position` |
| `--output` / `--output-dir` / `--in-place` | **um dos três**, exceto com `--dry-run` | veja [Invocação](invocation.md#o-destino) | — |
| `--dry-run` | não | uma opção sem valor: calcula e informa, não grava nada — veja [Ver antes de escrever](invocation.md#ver-antes-de-escrever) | desativado |

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nHello.\n\n2\n00:00:03,000 --> 00:00:04,000\nBye.\n\n' > film.srt; printf '1\n00:00:01,300 --> 00:00:02,300\nBonjour.\n\n2\n00:00:03,300 --> 00:00:04,300\nAu revoir.\n\n' > film.fr.srt; subedit-cli pair film.srt -t film.fr.srt --output recale.fr.srt; cat recale.fr.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nHello.\n\n2\n00:00:03,000 --> 00:00:04,000\nBye.\n\n' > film.srt; printf '1\n00:00:01,300 --> 00:00:02,300\nBonjour.\n\n2\n00:00:03,300 --> 00:00:04,300\nAu revoir.\n\n' > film.fr.srt; subedit-cli pair film.srt -t film.fr.srt --output recale.fr.srt; cat recale.fr.srt
film.fr.srt: translation: 2 lines attached -> recale.fr.srt
1
00:00:01,000 --> 00:00:02,000
Bonjour.

2
00:00:03,000 --> 00:00:04,000
Au revoir.
```

A tradução estava 0,3 s atrasada: cada linha encontrou a legenda que seu ponto médio sobrepõe, e o
arquivo gravado carrega as posições do arquivo principal.

## O que o alinhamento informa

A frase é **a mesma que a janela exibe** ao abrir uma tradução, cada parte somente quando não é
zero — veja [`inspect -t`](inspect.md#uma-tradução--t) para a tabela de suas partes. **Ela é escrita
a partir do nível 1**, e não no nível 2 como nos outros subcomandos: aqui o alinhamento não é um
detalhe do processamento, é o resultado.

**O que o alinhamento faz ao arquivo gravado**:

- **uma legenda que ninguém traduziu é gravada como um bloco sem texto**, que é lido sem nenhuma anomalia;
- **uma legenda nascida de uma linha** — uma linha que nada alcançou — é gravada com **suas próprias posições**, qualquer que seja
  o método: o arquivo gravado pode, portanto, ter mais legendas do que o arquivo principal;
- **tudo o que o documento carregava como tradução é apagado** antes do assentamento, o que não se aplica à linha de comando:
  o arquivo principal é lido sozinho.

**Os dois métodos** dão o mesmo arquivo quando as linhas caem cada uma em sua legenda, e não
quando falta uma linha ou a tradução está deslocada — por isso o método é uma escolha, e por isso
a posição é o padrão. Os oito casos de `src/test/data/paires/` os enumeram, com o arquivo esperado para
cada método.

## Saída

**Saída padrão** — nada: o resultado é o arquivo gravado.

| Nível | O que se acrescenta na saída de erro |
| :----- | :------------------------------------- |
| 1 | `<translation>: translation: N lines attached; … -> <destination>` — **o caminho é o da tradução**, que é o arquivo gravado; com `--dry-run`, `(dry run, nothing written)` no lugar de `-> <destination>` |
| 2 | `<translation>: SubRip, UTF-8, no BOM, LF line endings kept` — o formato, a codificação, a marca e os finais de linha **da tradução**, restituídos como estavam |
| 3 | `<translation>: N bytes read, M written`, depois **todo diagnóstico de leitura** do arquivo principal e da tradução — veja [Invocação](invocation.md#diagnósticos-de-leitura) |

### Em JSON

Com [`--format json`](invocation.md#uma-saída-que-um-script-pode-ler), **um único objeto**, cujo `file` é a tradução e
`destination` o arquivo gravado. `counts.subtitles` conta **as legendas do arquivo gravado**, incluídas as que uma linha
fez nascer. A chave `alignment` — `{"file", "method", "attached", "born", "untranslated", "out_of_order"}` —
traz as mesmas contagens que a frase, como a chave `translation` de [`inspect`](inspect.md#em-json).

## Códigos de saída

Os da ferramenta: `0` se a tradução é gravada — ou calculada, com `--dry-run` —, `2` se um dos dois arquivos não pôde
ser lido ou se o destino não pôde ser gravado, `1` em caso de erro de uso. **Uma tradução mal alinhada não é
uma falha**: ela é gravada, e a frase diz o que não encontrou seu lugar.

## Erros

| O que os provoca | Mensagem |
| :------------------- | :------ |
| nenhum `-t` | `pair needs the translation file to lay over the main one: use -t` |
| `--document` | `The following argument was not expected: --document` — não existe para `pair` |
| mais de um arquivo principal | `The following argument was not expected: <path>` |
| a tradução é o arquivo principal | `<translation>: the file is already open as the main document`, código `2` |
| um dos arquivos ausente, ilegível ou de formato desconhecido | o mesmo de sempre, com o caminho do arquivo em questão |
