# `pair`

<!-- machine-translated -->
> Esta página é uma tradução automática e ainda não foi revista por nenhum leitor do idioma.

```
subedit-cli pair (--output FILE | --output-dir DIR | --in-place)
                 -t FILE [--align-method position|number]
                 [--dry-run]
                 <main>
```

Escreve **a tradução resincronizada pelas posições do ficheiro principal**, e diz como as suas linhas encontraram a
legenda correspondente. É o `Open Translation…` da janela — [o mesmo emparelhamento](../subedit-gui/files.md#abrir-uma-tradução),
a mesma frase — seguido da escrita do resultado, que a janela faz através de `Save Translation`.

**É a única coisa que o emparelhamento faz e que um ficheiro processado sozinho não faz.** Os outros subcomandos
que aceitam `-t` alteram a tradução que leem — [`case`](case.md), [`replace`](replace.md)… —; `pair` não altera
nada no texto: **coloca** a tradução sobre o ficheiro principal e escreve o que a colocação produziu. O ficheiro principal é
lido para receber as linhas, **nunca escrito**.

- **O que se escreve é a tradução**, no **seu próprio caminho e no seu próprio formato**: o formato, a codificação e
  os fins de linha do ficheiro de tradução. As suas legendas ficam **nas posições do ficheiro principal**: uma linha que
  caiu ao lado da marca recupera os limites da legenda que traduz.
- **`-t` é obrigatório.** `pair` sem tradução não tem nada para colocar: é um erro de utilização, código `1`.
- **Um único ficheiro principal**, e nada de `--recursive`: um par são dois ficheiros, e uma árvore não tem
  maneira de dizer que tradução corresponde a que ficheiro principal. Para um lote, uma invocação por par.
- **`--output` nomeia o ficheiro da tradução escrita, `--output-dir` põe lá o seu nome de base, `--in-place`
  reescreve-o**, como para qualquer subcomando que aceita `-t` — ver [Uma tradução](invocation.md#uma-tradução).
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

| Opção | Obrigatório | Valores | Predefinição |
| :----- | :----- | :------ | :----- |
| `<main>` | sim | um **único** caminho | — |
| `-t`, `--translation-file` | **sim** | o ficheiro de tradução | — |
| `--align-method` | não | `position` ou `number` — ver [Uma tradução](invocation.md#uma-tradução) | `position` |
| `--output` / `--output-dir` / `--in-place` | **uma das três**, exceto com `--dry-run` | ver [Invocação](invocation.md#o-destino) | — |
| `--dry-run` | não | um indicador: calcula e diz, não escreve nada — ver [Ver antes de escrever](invocation.md#ver-antes-de-escrever) | desativado |

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

A tradução estava 0,3 s atrasada: cada linha encontrou a legenda com que o seu meio se sobrepõe, e o
ficheiro escrito tem as posições do ficheiro principal.

## O que o alinhamento diz

A frase é **a que a janela diz** quando abre uma tradução, cada parte só quando não é
zero — ver [`inspect -t`](inspect.md#uma-tradução--t) para a tabela das suas partes. **É escrita
a partir do nível 1**, e não no nível 2 como nos outros subcomandos: aqui o alinhamento não é um
pormenor do processamento, é o resultado.

**O que o alinhamento faz ao ficheiro escrito**:

- **uma legenda que ninguém traduziu é escrita como um bloco sem texto**, que se lê sem qualquer anomalia;
- **uma legenda nascida de uma linha** — uma linha que nada alcançou — é escrita com **as suas próprias posições**, seja qual for
  o método: o ficheiro escrito pode, portanto, conter mais legendas do que o ficheiro principal;
- **o que o documento tinha como tradução é apagado** antes da colocação, o que não se aplica na linha de comandos:
  o ficheiro principal é lido sozinho.

**Os dois métodos** dão o mesmo ficheiro quando as linhas caem cada uma na sua legenda, e não
quando falta uma linha ou a tradução está desfasada — por isso o método é uma escolha, e por isso
a posição é a predefinição. Os oito casos de `src/test/data/paires/` enumeram-nos, com o ficheiro esperado para
cada método.

## Saída

**Saída padrão** — nada: o resultado é o ficheiro escrito.

| Nível | O que se acrescenta à saída de erros |
| :----- | :------------------------------------- |
| 1 | `<translation>: translation: N lines attached; … -> <destination>` — **o caminho é o da tradução**, que é o ficheiro escrito; com `--dry-run`, `(dry run, nothing written)` no lugar de `-> <destination>` |
| 2 | `<translation>: SubRip, UTF-8, no BOM, LF line endings kept` — o formato, a codificação, a marca e os fins de linha **da tradução**, repostos como estavam |
| 3 | `<translation>: N bytes read, M written`, depois **todos os diagnósticos de leitura** do ficheiro principal e da tradução — ver [Invocação](invocation.md#ler-os-diagnósticos) |

### Em JSON

Com [`--format json`](invocation.md#uma-saída-que-um-script-pode-ler), **um único objeto**, cujo `file` é a tradução e
`destination` o ficheiro escrito. `counts.subtitles` conta **as legendas do ficheiro escrito**, incluindo as que uma linha
fez nascer. A chave `alignment` — `{"file", "method", "attached", "born", "untranslated", "out_of_order"}` —
traz as mesmas contagens que a frase, tal como a chave `translation` de [`inspect`](inspect.md#em-json).

## Códigos de saída

Os da ferramenta: `0` se a tradução for escrita — ou calculada, com `--dry-run` —, `2` se um dos dois ficheiros não
puder ser lido ou se o destino não puder ser escrito, `1` num erro de utilização. **Uma tradução mal alinhada não é
uma falha**: é escrita, e a frase diz o que não encontrou o seu lugar.

## Erros

| O que os desencadeia | Mensagem |
| :------------------- | :------ |
| sem `-t` | `pair needs the translation file to lay over the main one: use -t` |
| `--document` | `The following argument was not expected: --document` — não existe para `pair` |
| mais de um ficheiro principal | `The following argument was not expected: <path>` |
| a tradução é o ficheiro principal | `<translation>: the file is already open as the main document`, código `2` |
| um dos ficheiros em falta, ilegível ou de formato desconhecido | o mesmo que em todo o lado, com o caminho do ficheiro em falta |
