# `config`

<!-- machine-translated -->
> Bu sayfa makine çevirisidir ve henüz dili bilen bir okuyucu tarafından gözden geçirilmemiştir.

```
subedit-cli [--config FILE | --no-config] [--format text|json] config list
subedit-cli [--config FILE | --no-config] [--format text|json] config show
subedit-cli [--config FILE] config set KEY VALUE
subedit-cli [--config FILE] config unset KEY
```

Ayar dosyasının pencere ile komut satırının paylaştığı bölümünü, yani **işlem ayarlarını**
listelemek, göstermek ve değiştirmek için kullanılır. `Adjust Durations…` ve `Correct Texts…` değerlerini orada hatırlar;
[`adjust`](adjust.md) ve [`correct`](correct.md) de bunları varsayılan olarak okur — bkz.
[Ayar dosyası](invocation.md#ayar-dosyası).

**Yapmadıkları:** hiçbir **görüntü tercihini** (pencerenin
geometrisi, sütunlar, tema, arama seçenekleri, yazım denetimi, video…) ne okur ne yazar. Bunlar
pencerenin işidir; pencerede ayarlanır ve [kılavuzunda](../subedit-gui/preferences.md#dosya) anlatılır.
Hiçbir altyazı dosyasına da dokunmaz.

<!-- example: subedit-cli config --help -->
```console
$ subedit-cli config --help
List, show and change the operation settings
Usage: subedit-cli config [OPTIONS] SUBCOMMAND

Options:
  -h,--help                   Print this help message and exit

Subcommands:
  list                        Write every operation key, what it is for, what it accepts and its default
  show                        Write the settings file used, then every operation key with its value
  set                         Write KEY = VALUE into the settings file, keeping every other setting
  unset                       Put KEY back to its default in the settings file, keeping every other setting
```

## Alt komutlar

| Alt komut | Ne yapar | Dosya |
| :---------- | :----------- | :--- |
| `config list` | her işlem anahtarını, ne işe yaradığıyla, neyi kabul ettiğiyle, varsayılanıyla ve kimin kullandığıyla yazar — **değer yok** | hiçbirini okumaz |
| `config show` | kullanılan dosyayı ve var olup olmadığını yazar, ardından her işlem anahtarını değeri, değerin nereden geldiği ve varsayılanıyla yazar | okur |
| `config set KEY VALUE` | `VALUE` değerini denetler, ardından diğer tüm ayarları koruyarak dosyayı `KEY = VALUE` ile yeniden yazar | okur ve yazar |
| `config unset KEY` | `KEY` anahtarını varsayılanına döndürür; dosya her zaman bir varsayılanı yorum satırı olarak yazdığı için yorum satırı yapılarak yazılır | okur ve yazar |

| Seçenek | Zorunlu | Değerler | Varsayılan |
| :----- | :------- | :----- | :------ |
| `KEY` | evet, `set` ve `unset` için | `config list` komutunun adlandırdığı bir işlem anahtarı | — |
| `VALUE` | evet, `set` için | `config list` komutunun anahtarın kabul ettiğini söylediği şey | — |

`config` komutunun kendi seçeneği yoktur. `config` komutundan **önce** yazılan genel seçenekler
dosyayı ve çıktının biçimini belirler:

| Genel seçenek | `config` üzerindeki etkisi |
| :------------ | :----------------- |
| `--config FILE` | `show`, `set` ve `unset`, kullanıcının dosyası yerine, henüz var olmasa bile `FILE` üzerinde işlem yapar; `list` bunu yok sayar |
| `--no-config` | `list` her zamanki gibi çalışır; `show`, `File: none (--no-config)` ile varsayılanları gösterir; `set` ve `unset` reddedilir: yazılacak dosya yoktur |
| `--format json` | `list` ve `show` satır başına bir JSON nesnesi yazar; `set` ve `unset` standart çıktıya hiçbir şey yazmaz |
| `-q`, `--quiet` | `set` ve `unset` artık yazdıklarını onaylamaz |

## Hangi dosya

`--config` olmadan, kullanıcının dosyası:

```
$XDG_CONFIG_HOME/subedit/settings.conf
```

veya `XDG_CONFIG_HOME` tanımsız, boş ya da mutlak yol olmadığında `~/.config/subedit/settings.conf`.
Pencerenin okuyup yazdığı dosya budur; `config show` yolunu ilk satırında yazar:

<!-- example: XDG_CONFIG_HOME=/home/vous/.config subedit-cli config show | head -1 -->
```console
$ XDG_CONFIG_HOME=/home/vous/.config subedit-cli config show | head -1
File: /home/vous/.config/subedit/settings.conf (not found)
```

Ne `XDG_CONFIG_HOME` ne de `HOME` bir yer vermiyorsa kullanıcının dosyası yoktur: `show`, `set`
ve `unset` reddedilir ve `--config FILE` bir dosya adlandırır.

## `config list`

**İşlem anahtarlarının kataloğu**, dosyanın sırasıyla: her anahtar, ardından girintili dört
satır — ne işe yaradığı, `Accepts:` (kapalı küme veya biçim ve sınırlar), `Default:` ve
`Used by:` (onu okuyan `subedit-cli` komutları ve pencere eylemleri). Hiçbir dosya okumaz,
hiçbirine ihtiyaç duymaz ve `--no-config` ile de çalışır.

<!-- example: subedit-cli config list | head -10 -->
```console
$ subedit-cli config list | head -10
duration-adjust.speed
    Reading speed the durations are brought to.
    Accepts: a number of characters per second, above 0, decimals allowed
    Default: 15
    Used by: subedit-cli adjust; Adjust Durations…
duration-adjust.lengthen
    Let the reading speed move an end later.
    Accepts: true or false
    Default: true
    Used by: subedit-cli adjust; Adjust Durations…
```

**45 anahtar** listeler:

<!-- example: subedit-cli config list | grep -c '^[a-z]' -->
```console
$ subedit-cli config list | grep -c '^[a-z]'
45
```

**Yirmi beşini komut satırı okur**; diğer yirmisini yalnızca pencere okur
ve `Used by:` satırları bunu, yerlerini alan `correct` seçeneğiyle birlikte söyler:

| Anahtar | Okuyan |
| :-- | :------ |
| `duration-adjust.speed`, `duration-adjust.lengthen`, `duration-adjust.shorten` | [`adjust`](adjust.md) ve `Adjust Durations…` |
| `duration-adjust.minimum-enabled`, `duration-adjust.minimum-ms` | `adjust` ve `Adjust Durations…` |
| `duration-adjust.maximum-enabled`, `duration-adjust.maximum-ms` | `adjust` ve `Adjust Durations…` |
| `duration-adjust.gap-enabled`, `duration-adjust.gap-ms` | `adjust` ve `Adjust Durations…` |
| `correction.human`, `correction.ocr` | [`correct`](correct.md) ve `Correct Texts…` |
| `correction.remove-blank` | `correct` ve `Correct Texts…` |
| `correction.line-break.max-lines` | `correct` ve `Correct Texts…` |
| `transcription.engine`, `transcription.executable`, `transcription.model`, `transcription.vad-model`, `transcription.language`, `transcription.suppress-notes`, `transcription.merge-gap-ms`, `transcription.max-duration-ms`, `transcription.max-characters` | [`transcribe`](transcribe.md) ve `Transcribe Audio…` |
| `alignment.engine`, `alignment.min-match`, `alignment.doubt-gap` | [`sync`](sync.md) |
| diğer her `correction.*` anahtarı | yalnızca `Correct Texts…` — `correct`, `--tasks`, `--code`, `--max-length`, `--skip-length`, `--skip-lines`, `--language`, `--enable` ve `--disable` değerlerini kendi satırından alır |

Satır sayıları, `correction.line-break.max-lines` ve `correction.line-break.skip-max-lines`,
1 ile 1000 arasında tam sayılardır — `correct --max-lines` sınırları —, satır uzunlukları ise,
`correction.line-break.max-length` ve `correction.line-break.skip-max-length`, sıfırdan büyük
sayılardır: `set` gerisini reddeder ve elle düzenlenmiş bir dosyada bunlardan biri varsa varsayılan korunur.

`correct`, görevleri, kodu ve satır uzunluğunu bilerek kendi satırından alır: bir komut satırında önceden
hiçbir şey işaretlenmez ve pencere satırları em cinsinden ölçerken komut satırı
karakter sayar. Pencerenin yönettiği desen kutularının listesi olan `correction.activations`,
kataloğun bir anahtarı değildir: `correct` komutunda `--enable` ve `--disable` vardır.

`--format json` ile her anahtar için bir nesne yazılır — `key`, `description`, `accepts`, `default`, `used_by`
ve `surface`, `"both"` veya `"window-only"`:

<!-- example: subedit-cli --format json config list | head -1 -->
```console
$ subedit-cli --format json config list | head -1
{"schema":1,"command":"config","key":"duration-adjust.speed","description":"Reading speed the durations are brought to.","accepts":"a number of characters per second, above 0, decimals allowed","default":"15","used_by":"subedit-cli adjust; Adjust Durations…","surface":"both"}
```

## `config show`

**Kullanılan dosya, ardından her işlem anahtarı değeriyle**: `File: <path> (found)`,
`(not found)` veya `(unreadable: <reason>)`, sonra anahtar başına bir satır —
dosya ayarlıyorsa `[file]` ve varsayılan, ayarlamıyorsa `[default]`. Dosyanın
okunamayan bir değerle içerdiği bir anahtar varsayılanını gösterir. Görüntü tercihleri listelenmez.

<!-- example: subedit-cli --config mine/settings.conf config set duration-adjust.speed 12; subedit-cli --config mine/settings.conf config show | head -4 -->
```console
$ subedit-cli --config mine/settings.conf config set duration-adjust.speed 12; subedit-cli --config mine/settings.conf config show | head -4
duration-adjust.speed = 12
File: mine/settings.conf (found)
duration-adjust.speed = 12   [file]   (default 15)
duration-adjust.lengthen = true   [default]
duration-adjust.shorten = false   [default]
```

Eksik bir dosya hata değildir: `show` varsayılanları yazar ve kod `0` olur — `--config` ile adlandırılan bir `FILE`
için ise [`adjust`](adjust.md) ve [`correct`](correct.md) tam tersine reddeder
(bkz. [Ayar dosyası](invocation.md#ayar-dosyası)); burada söz konusu olan kullanıcının dosyasıdır.
`--no-config` ile:

<!-- example: subedit-cli --no-config config show | head -2 -->
```console
$ subedit-cli --no-config config show | head -2
File: none (--no-config)
duration-adjust.speed = 15   [default]
```

`--format json` ile ilk nesne `{"file", "exists"}` olur — `--no-config` ile `file` değeri `null` —,
ardından her anahtar için bir nesne gelir: `key`, `value`, `source` (`"file"` veya `"default"`), `default`,
`used_by`, `surface`.

<!-- example: subedit-cli --config mine/settings.conf config set duration-adjust.speed 12; subedit-cli --format json --config mine/settings.conf config show | head -2 -->
```console
$ subedit-cli --config mine/settings.conf config set duration-adjust.speed 12; subedit-cli --format json --config mine/settings.conf config show | head -2
duration-adjust.speed = 12
{"schema":1,"command":"config","file":"mine/settings.conf","exists":true}
{"schema":1,"command":"config","key":"duration-adjust.speed","value":"12","source":"file","default":"15","used_by":"subedit-cli adjust; Adjust Durations…","surface":"both"}
```

## `config set` ve `config unset`

**`set`, değeri dosyanın kendi okuyucusuyla denetler** — pencerenin kullandığı okuyucu — ve
okunamıyorsa, hiçbir şey yazılmadan önce reddeder. **`unset`** varsayılanı yorum satırı yaparak
geri yazar: böylece anahtar, sonraki bir sürüm varsayılanı değiştirse bile varsayılanı izler.
Zaten varsayılanında olan bir anahtarı sıfırlamak başarılı olur ve hiçbir şeyi değiştirmez.

İkisi de **dosyayı bütünüyle ve atomik olarak yeniden yazar**: içindeki her görüntü tercihi korunur,
işlem bloğu önce, görüntü bloğu sonra gelir; her biri kendi başlığı altındadır. **Dosyanın okuyamadığı bir değer
ve bilinmeyen bir anahtar korunmaz** — yeniden yazma işini pencere yapar ve pencere de bunları
atar. **Atılan her değer bildirilir**; varsayılan düzeyde standart hata çıktısına, görüntü
tercihleri dahil: `<key>: unreadable value '<value>' replaced by its default`. Dosya ve
dizini eksikse oluşturulur.

Onay, varsayılan düzeyde standart hata çıktısına gider: `KEY = VALUE` (`set` için),
`KEY reset to its default (DEFAULT)` (`unset` için).

<!-- example: subedit-cli --config mine/settings.conf config set duration-adjust.gap-ms 100; grep -e '^[a-z]' -e '^# [A-Z][a-z]* [a-z]*:' mine/settings.conf; subedit-cli --config mine/settings.conf config unset duration-adjust.gap-ms; grep gap-ms mine/settings.conf -->
```console
$ subedit-cli --config mine/settings.conf config set duration-adjust.gap-ms 100; grep -e '^[a-z]' -e '^# [A-Z][a-z]* [a-z]*:' mine/settings.conf; subedit-cli --config mine/settings.conf config unset duration-adjust.gap-ms; grep gap-ms mine/settings.conf
duration-adjust.gap-ms = 100
# Operation settings: shared by the window and the command line (subedit-cli reads these).
duration-adjust.gap-ms = 100
# Display preferences: the window's own, the command line never reads them.
duration-adjust.gap-ms reset to its default (0)
#duration-adjust.gap-ms = 0
#transcription.merge-gap-ms = 400
```

## Çıkış kodları

| Kod | Anlam |
| :--- | :------ |
| `0` | tamam — eksik bir dosyada `show` ve varsayılanındaki bir anahtarda `unset` dahil |
| `1` | kullanım hatası: alt komut yok, eksik bir bağımsız değişken, reddedilen bir anahtar veya değer, `set` veya `unset` ile `--no-config`, boş adlı `--config`, kullanıcı dosyası yok ve `--config` yok; hiçbir şey yazılmaz |
| `2` | dosya okunamıyor veya yazılamıyor; hiçbir şey yazılmaz |

## Hatalar

| Neyin tetiklediği | Standart hata çıktısına ne yazılır |
| :--------------- | :------------------------------------- |
| bir görüntü tercihi | `<key> is a display preference; set it in the window` |
| bilinmeyen bir anahtar | `unknown setting <key>; subedit-cli config list names them` |
| anahtarın kabul etmediği bir değer | `cannot read '<value>' for <key>; subedit-cli config list says what it accepts` |
| `correction.activations` | `correction.activations is managed by the window; use --enable/--disable on correct` |
| `--no-config` ile `set` veya `unset` | `--no-config has nothing to write to` |
| kullanıcı dosyası bulunamıyor ve `--config` yok | `no settings file could be located; give one with --config FILE` |
| `--no-config` ile `--config` | `--config excludes --no-config` |
| boş adlı `--config` | `--config needs a file name` |
| `set` veya `unset` için var olan ama okunamayan bir dosya | `<path>: cannot be read; nothing written` |
| yazılamayan bir dosya | `<path>: cannot be written` veya `<path>: cannot be written: permission denied` |

<!-- example: subedit-cli --config mine/settings.conf config set general.theme dark; echo "code=$?"; subedit-cli --config mine/settings.conf config set duration-adjust.speed fast; echo "code=$?"; subedit-cli --no-config config unset duration-adjust.speed; echo "code=$?" -->
```console
$ subedit-cli --config mine/settings.conf config set general.theme dark; echo "code=$?"; subedit-cli --config mine/settings.conf config set duration-adjust.speed fast; echo "code=$?"; subedit-cli --no-config config unset duration-adjust.speed; echo "code=$?"
general.theme is a display preference; set it in the window
code=1
cannot read 'fast' for duration-adjust.speed; subedit-cli config list says what it accepts
code=1
--no-config has nothing to write to
code=1
```
