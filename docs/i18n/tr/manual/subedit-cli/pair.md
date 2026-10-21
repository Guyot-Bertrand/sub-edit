# `pair`

<!-- machine-translated -->
> Bu sayfa makine çevirisidir ve henüz dili bilen bir okuyucu tarafından gözden geçirilmemiştir.

```
subedit-cli pair (--output FILE | --output-dir DIR | --in-place)
                 -t FILE [--align-method position|number]
                 [--dry-run]
                 <main>
```

**Çevirinin ana dosyanın konumlarına göre yeniden zamanlanmış hâlini** yazar ve satırlarının altyazılarını nasıl bulduğunu bildirir.
Pencerenin `Open Translation…` eylemidir — [aynı eşleme](../subedit-gui/files.md#çeviri-açma),
aynı cümle — ve ardından sonucun yazılması gelir; pencere bunu `Save Translation` ile yapar.

**Eşlemenin, tek başına işlenen bir dosyanın yapmadığı tek şeyi yaptığı yer burasıdır.** `-t` alan diğer alt komutlar
okudukları çeviriyi değiştirir — [`case`](case.md), [`replace`](replace.md)… —; `pair` ise metinde
hiçbir şeyi değiştirmez: çeviriyi ana dosyanın üzerine **yatırır** ve yatırmanın ürettiğini yazar. Ana dosya,
satırları almak için okunur, **asla yazılmaz**.

- **Yazılan şey çeviridir**, **kendi yolunda ve kendi biçiminde**: çeviri dosyasının biçimi, kodlaması ve
  satır sonlarıyla. Altyazıları **ana dosyanın konumlarındadır**: işaretin yanına düşmüş bir satır,
  çevirdiği altyazının sınırlarını geri alır.
- **`-t` zorunludur.** Çevirisiz bir `pair` çağrısının yatıracağı bir şey yoktur: bu bir kullanım hatasıdır, kod `1`.
- **Tek bir ana dosya** alınır ve `--recursive` yoktur: bir çift iki dosyadır ve bir ağaç,
  hangi çevirinin hangi ana dosyaya ait olduğunu söyleyemez. Bir grup için çift başına bir çağrı yapılır.
- **`--output` yazılan çeviri dosyasının adını verir, `--output-dir` onun temel adını oraya koyar, `--in-place`
  onu yeniden yazar**; `-t` alan her alt komutta olduğu gibi — bkz. [Bir çeviri](invocation.md#bir-çeviri).
- **`--document` yoktur**: hedeflenen tek belge vardır, o da çeviridir.

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

## Bağımsız değişkenler ve seçenekler

| Seçenek | Zorunlu | Değerler | Varsayılan |
| :----- | :----- | :------ | :----- |
| `<main>` | evet | **tek** bir yol | — |
| `-t`, `--translation-file` | **evet** | çeviri dosyası | — |
| `--align-method` | hayır | `position` veya `number` — bkz. [Bir çeviri](invocation.md#bir-çeviri) | `position` |
| `--output` / `--output-dir` / `--in-place` | `--dry-run` dışında **üçünden biri** | bkz. [Çalıştırma](invocation.md#hedef) | — |
| `--dry-run` | hayır | bir bayrak: hesaplar ve bildirir, hiçbir şey yazmaz — bkz. [Yazmadan önce görme](invocation.md#yazmadan-önce-görme) | kapalı |

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

Çeviri 0,3 sn geç kalıyordu: her satır, ortasının üst üste bindiği altyazıyı buldu ve
yazılan dosya ana dosyanın konumlarını taşıyor.

## Hizalamanın bildirdikleri

Cümle, **pencerenin** bir çeviriyi açarken söylediği cümledir; her bölümü yalnızca
sıfır olmadığında yazılır — bölümlerinin tablosu için bkz. [`inspect -t`](inspect.md#bir-çeviri--t). **1. düzeyden
itibaren yazılır**, diğer alt komutlarda olduğu gibi 2. düzeyde değil: burada hizalama,
işlemenin bir ayrıntısı değil, sonucun kendisidir.

**Hizalamanın yazılan dosyaya yaptıkları**:

- **kimsenin çevirmediği bir altyazı metinsiz bir blok olarak yazılır**; bu, hiçbir anomali olmadan okunur;
- **bir satırdan doğan altyazı** — hiçbir şeyin ulaşmadığı bir satır — yöntem ne olursa olsun **kendi
  konumlarıyla** yazılır: yazılan dosya bu yüzden ana dosyadan daha çok altyazı sayabilir;
- **belgenin çeviri olarak taşıdığı her şey** yatırmadan önce temizlenir; bu komut satırında geçerli değildir:
  ana dosya tek başına okunur.

**İki yöntem**, satırların her biri kendi altyazısına düştüğünde aynı dosyayı verir,
bir satır eksik olduğunda ya da çeviri kaydığında vermez — yöntemin bir seçenek olması
ve konumun varsayılan olması bu yüzdendir. `src/test/data/paires/` dizinindeki sekiz durum bunları listeler ve her
yöntem için beklenen dosyayı içerir.

## Çıktı

**Standart çıktı** — hiçbir şey: sonuç, yazılan dosyadır.

| Düzey | Hata çıktısına eklenenler |
| :----- | :------------------------------------- |
| 1 | `<translation>: translation: N lines attached; … -> <destination>` — **yol çevirinin yoludur**, yani yazılan dosyanın; `--dry-run` ile `-> <destination>` yerine `(dry run, nothing written)` yazılır |
| 2 | `<translation>: SubRip, UTF-8, no BOM, LF line endings kept` — **çevirinin** biçimi, kodlaması, işareti ve satır sonları, olduğu gibi geri konarak |
| 3 | `<translation>: N bytes read, M written`, ardından ana dosyanın ve çevirinin **her okuma tanılama iletisi** — bkz. [Çalıştırma](invocation.md#tanılama-iletilerini-okuma) |

### JSON biçiminde

[`--format json`](invocation.md#bir-betiğin-okuyabileceği-çıktı) ile **tek bir nesne** yazılır; `file` çeviridir,
`destination` ise yazılan dosyadır. `counts.subtitles` **yazılan dosyanın altyazılarını** sayar, bir satırın
doğurduklarıyla birlikte. `alignment` anahtarı — `{"file", "method", "attached", "born", "untranslated", "out_of_order"}` —
cümleyle aynı sayıları taşır; [`inspect`](inspect.md#json-biçiminde) komutunun `translation` anahtarı gibi.

## Çıkış kodları

Aracın kodlarıdır: çeviri yazılmışsa — ya da `--dry-run` ile hesaplanmışsa — `0`, iki dosyadan biri
okunamadıysa ya da hedefe yazılamadıysa `2`, kullanım hatasında `1`. **Kötü hizalanmış bir çeviri
başarısızlık değildir**: yazılır ve cümle neyin yerini bulamadığını söyler.

## Hatalar

| Neyin tetiklediği | İleti |
| :------------------- | :------ |
| `-t` yok | `pair needs the translation file to lay over the main one: use -t` |
| `--document` | `The following argument was not expected: --document` — `pair` için yoktur |
| birden fazla ana dosya | `The following argument was not expected: <path>` |
| çeviri, ana dosyanın kendisi | `<translation>: the file is already open as the main document`, kod `2` |
| dosyalardan biri eksik, okunamaz ya da bilinmeyen biçimde | her yerdeki ile aynı, hatalı dosyanın yoluyla |
