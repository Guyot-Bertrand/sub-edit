# Zpracování stromu

<!-- machine-translated -->
> Tato stránka je strojovým překladem a dosud ji nezkontroloval žádný čtenář tohoto jazyka.

```
subedit-cli <subcommand> --recursive [options] <directory>...
```

Každý podpříkaz, který čte dávku souborů — včetně `inspect`, a kromě
[`append`](append.md), [`split-file`](split-file.md) a [`pair`](pair.md), jejichž arita je arita dvojice nebo
filmu —, přijímá s volbou `--recursive` (`-r`) místo souboru **adresář**.
Tato stránka popisuje, co procházení zahrnuje, v jakém pořadí a kam zapisuje.

**Nic z toho, co následuje, nemění to, co se se souborem provede**: každý se čte,
zpracovává a zapisuje samostatně, selhání jednoho nezastaví ostatní a
čtyři návratové kódy jsou ty, které uvádí [spuštění](invocation.md#návratové-kódy).
Procházení vytvoří seznam cest, který zbytek dávky zpracuje, jako by byly
zadány ručně.

## Bez `--recursive` se adresář odmítne

Chyba použití, kód `1`, žádný soubor se nezpracuje ani nezapíše:

<!-- example: mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; subedit-cli inspect films; echo "code=$?" -->
```console
$ mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; subedit-cli inspect films; echo "code=$?"
films: is a directory: use --recursive
code=1
```

## Co procházení zahrnuje

Procházení je **rekurzivní, deterministické a bez překvapení**:

- **pořadí** — položky každého adresáře se berou v pořadí jejich
  názvů, **bajt po bajtu** (velká písmena před malými, bez ohledu na
  jazyk počítače), a podadresář se prochází na svém místě, do
  hloubky. Dvě spuštění dají stejné bajty na každém
  počítači;
- **symbolické odkazy se nesledují** — ani na soubor, ani na
  adresář: strom, který obsahuje odkaz na některého ze svých rodičů, by neměl
  konec;
- **skryté soubory a adresáře se ignorují** — ty, jejichž název začíná
  tečkou;
- **ponechávají se jen přípony známých formátů**: `.srt`, `.vtt`,
  `.ssa`, `.ass`, `.lrc`, `.sub`, bez ohledu na velikost písmen. **`.txt` je vyloučena**:
  označuje dva formáty, ale hlavně každý `LISEZMOI.txt` na světě, a dávka, která
  by na každém z nich selhala, by byla nepoužitelná.

**Soubor uvedený na příkazovém řádku se nikdy nefiltruje.** Uvést ho znamená
chtít ho: `subedit-cli shift --by 1 --in-place notes.txt` ho zpracuje, ať má
jakoukoli příponu, a ozve se, jen pokud jeho obsah není v žádném formátu.

**Co bylo vynecháno, se spočítá** a sdělí na úrovni `-vv`:

<!-- example: mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; printf 'note\n' > films/LISEZMOI.txt; subedit-cli -vv inspect --recursive films 2>&1 | head -1 -->
```console
$ mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; printf 'note\n' > films/LISEZMOI.txt; subedit-cli -vv inspect --recursive films 2>&1 | head -1
films: 1 file left aside, with no extension of a known format
```

Adresář, který neobsahuje nic k procházení, to sdělí na výchozí úrovni
(`<directory>: holds no file in a format this tool walks for`) a není to chyba.

## Kam procházení zapisuje

**Pod `--output-dir` se strom zachová**: cesta souboru
**vzhledem k zadanému adresáři** se zopakuje pod výstupním adresářem. Název
zadaného adresáře do ní nepatří — je to sémantika `rsync` s
koncovým lomítkem, ne `cp -r`: uvede se cíl a přidá se k němu
požadovaná úroveň (`--output-dir sortie/films`).

<!-- example: mkdir -p films/a/b; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/x.srt; cp films/x.srt films/a/x.srt; cp films/x.srt films/a/b/y.srt; printf 'note\n' > films/LISEZMOI.txt; subedit-cli shift --by 1 --recursive --output-dir sortie films; find sortie -type f | sort -->
```console
$ mkdir -p films/a/b; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/x.srt; cp films/x.srt films/a/x.srt; cp films/x.srt films/a/b/y.srt; printf 'note\n' > films/LISEZMOI.txt; subedit-cli shift --by 1 --recursive --output-dir sortie films; find sortie -type f | sort
films/a/b/y.srt: 1 subtitle shifted by 1.000 s -> sortie/a/b/y.srt
films/a/x.srt: 1 subtitle shifted by 1.000 s -> sortie/a/x.srt
films/x.srt: 1 subtitle shifted by 1.000 s -> sortie/x.srt
3 of 3 files shifted
sortie/a/b/y.srt
sortie/a/x.srt
sortie/x.srt
```

Výstupní adresář a adresáře stromu se **vytvoří**, pokud chybějí. S
`--in-place` se každý ponechaný soubor zapíše zpět na své místo. Přípona, kterou
`convert` změní, se změní i ve stromu: `films/a/x.srt` převedený
do WebVTT se zapíše jako `sortie/a/x.vtt`.

**Výstupní adresář, pokud leží uvnitř procházeného stromu, se z procházení vyloučí**:
druhé spuštění znovu nečte to, co zapsalo první.

## Dva adresáře a kolize

Rozhoduje pravidlo [cíle zápisu](invocation.md#cíl-zápisu): dva soubory,
které by skončily na stejné cestě, se odmítnou **dřív, než se cokoli zapíše**, kód `1`, a
uvedou se oba vstupy i cíl.

<!-- example: mkdir -p un/a deux/a; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > un/a/x.srt; cp un/a/x.srt deux/a/x.srt; subedit-cli shift --by 1 -r --output-dir sortie un deux; echo "code=$?" -->
```console
$ mkdir -p un/a deux/a; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > un/a/x.srt; cp un/a/x.srt deux/a/x.srt; subedit-cli shift --by 1 -r --output-dir sortie un deux; echo "code=$?"
sortie/a/x.srt: would be written by both un/a/x.srt and deux/a/x.srt
code=1
```

## Co to stojí

Dávka zůstává sekvenční: její trvání je součtem trvání jejích souborů.
Měření `make bench` měří procházení a tutéž práci v jediném souboru; čísla
jsou v [záznamu výkonu](../../../../mesures/performances.md).
