# `disjoint`

<!-- machine-translated -->
> Questa pagina è una traduzione automatica e non è ancora stata rivista da un lettore della lingua.

```
subedit-cli disjoint (--output FILE | --output-dir DIR | --in-place)
                     [--minimum TIME] [--max-lines N] [--dashes] [--sort]
                     [--dry-run] [--recursive]
                     <file>...
```

Taglia i sottotitoli sovrapposti in modo che **nessun intervallo `[start, end]` ne
sovrapponga più un altro**. È il comando `Make Ranges Disjoint…` della finestra — [lo stesso motore](../subedit-gui/lines.md#rendere-disgiunti-gli-intervalli).

**Nulla viene accorciato né allungato**: in ogni istante, il file mostra gli stessi testi di prima. Ciò che
cambia è il modo in cui è tagliata la linea temporale:

- viene tagliata in **ogni inizio e in ogni fine** dei sottotitoli di un gruppo di sovrapposizioni;
- un intervallo coperto da **un solo** sottotitolo resta quel sottotitolo;
- un intervallo coperto da **più** sottotitoli diventa **un** sottotitolo le cui righe sono le loro, **nell'
  ordine dei rispettivi inizi** — il più anticipato sulla riga 1, il successivo sulla riga 2;
- un sottotitolo che non si sovrappone a nulla non viene toccato, e l'ordine del resto viene mantenuto.

<!-- example: subedit-cli disjoint --help -->
```console
$ subedit-cli disjoint --help
Cut overlapping subtitles so that no range overlaps another, merging what overlaps
Usage: subedit-cli disjoint [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to make disjoint

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --minimum TIME              Shortest interval kept; a shorter one is taken over by a neighbour (default 0.08)
  --max-lines N               Most lines of a merged subtitle; more are shown in windows, 0 for no limit (default 2)
  --dashes                    Put a dash before the text of each merged subtitle
  --sort                      Put the subtitles in order of their start first; without it, a file out of order is refused
  --output TEXT               File to write, for a single input
  --output-dir TEXT           Directory to write into
  --in-place                  Write back over the inputs
  --dry-run                   Work out and say what would be written, and write nothing
```

## Argomenti e opzioni

| Opzione | Obbligatorio | Valori | Predefinito |
| :----- | :----- | :------ | :----- |
| `<file>...` | sì | uno o più percorsi | — |
| `--minimum` | no | una durata: `0.08`, `1.5`, `00:00:00,080`. Zero mantiene ogni intervallo | `0.08` (circa due fotogrammi) |
| `--max-lines` | no | un numero intero: il massimo di righe di un sottotitolo unito; `0` toglie il limite | `2` |
| `--dashes` | no | un'opzione senza valore: un trattino davanti al testo che ogni sottotitolo porta a un sottotitolo unito | disattivata |
| `--sort` | no | un'opzione senza valore: mette i sottotitoli nell'ordine del loro inizio **prima** — vedere [Serve un file in ordine](#serve-un-file-in-ordine) | disattivata |
| `--recursive`, `-r` | no | un'opzione senza valore | disattivata — vedere [Elaborare un albero di directory](lots.md) |
| `--output` / `--output-dir` / `--in-place` | **una delle tre**, tranne con `--dry-run` | vedere [Invocazione](invocation.md#la-destinazione) | — |
| `--dry-run` | no | un'opzione senza valore: calcola e riferisce, non scrive nulla — vedere [Vedere prima di scrivere](invocation.md#vedere-prima-di-scrivere) | disattivata |

<!-- example: printf '1\n00:00:00,000 --> 00:00:05,000\nA\n\n2\n00:00:03,000 --> 00:00:08,000\nB\n\n' > chevauche.srt; subedit-cli disjoint --output disjoint.srt chevauche.srt; cat disjoint.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:05,000\nA\n\n2\n00:00:03,000 --> 00:00:08,000\nB\n\n' > chevauche.srt; subedit-cli disjoint --output disjoint.srt chevauche.srt; cat disjoint.srt
chevauche.srt: 2 subtitles split, 1 merged -> disjoint.srt
1
00:00:00,000 --> 00:00:03,000
A

2
00:00:03,000 --> 00:00:05,000
A
B

3
00:00:05,000 --> 00:00:08,000
B
```

`A [0, 5]` e `B [3, 8]` danno tre sottotitoli disgiunti: `A` da solo, `A` e `B` insieme, `B` da solo. Nessun
istante viene perso né aggiunto.

## Che cosa decide l'operazione

**Una riga non viene mai duplicata.** Due sottotitoli con lo stesso testo danno una sola riga dove si
sovrappongono, e due parti vicine che mostrano la stessa cosa formano un unico sottotitolo. Il testo di un
sottotitolo tagliato in più parti viene invece **ripetuto in ciascuna**: è ciò che compare sullo schermo in quel
momento.

**Un intervallo molto breve viene assorbito da un vicino.** Sotto il valore di `--minimum`, un intervallo scompare: il vicino
**estende il proprio intervallo per coprirlo**, in modo che nessun istante resti vuoto e nulla si sposti di più di quella
soglia. Il vicino scelto è quello le cui righe **contengono già** quelle dell'intervallo, altrimenti il
più lungo. Il testo dell'intervallo va perso solo se nessun vicino lo porta, e in tal caso per meno della soglia.
**Un sottotitolo intero più breve della soglia non viene mai assorbito**: viene lasciato, e conteggiato.

**Più righe del limite: finestre.** Un sottotitolo unito che avrebbe più righe del valore di `--max-lines`
viene mostrato **in finestre successive** di quella dimensione, in parti uguali della durata. Tre righe su tre
secondi:

<!-- example: printf '1\n00:00:00,000 --> 00:00:03,000\nA\n\n2\n00:00:00,000 --> 00:00:03,000\nB\n\n3\n00:00:00,000 --> 00:00:03,000\nC\n\n' > trois.srt; subedit-cli disjoint --output fenetres.srt trois.srt; cat fenetres.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:03,000\nA\n\n2\n00:00:00,000 --> 00:00:03,000\nB\n\n3\n00:00:00,000 --> 00:00:03,000\nC\n\n' > trois.srt; subedit-cli disjoint --output fenetres.srt trois.srt; cat fenetres.srt
trois.srt: 0 subtitles split, 1 merged; 1 shown in windows for holding too many lines -> fenetres.srt
1
00:00:00,000 --> 00:00:01,500
A
B

2
00:00:01,500 --> 00:00:03,000
B
C
```

Quattro righe danno tre finestre, e così via: la finestra scorre di una riga alla volta. Nulla viene
troncato né tagliato di nuovo.

**La traduzione segue gli stessi tagli**, poiché un sottotitolo porta entrambi i testi. **Ciò che il formato porta
in più** — uno stile, una posizione — è quello del primo sottotitolo che copre; la perdita viene conteggiata quando
i sottotitoli uniti differiscono.

**Un sottotitolo la cui fine precede l'inizio** non viene interpretato: viene lasciato dov'è, e conteggiato.

## Serve un file in ordine

I gruppi sono serie di vicini: fuori ordine, i vicini non sono quelli che si sovrappongono. Un file
i cui inizi non sono in ordine viene quindi **rifiutato**, e il messaggio lo dice:

<!-- example: printf '1\n00:00:03,000 --> 00:00:08,000\nB\n\n2\n00:00:00,000 --> 00:00:05,000\nA\n\n' > desordre.srt; subedit-cli disjoint --output x.srt desordre.srt; echo "code $?" -->
```console
$ printf '1\n00:00:03,000 --> 00:00:08,000\nB\n\n2\n00:00:00,000 --> 00:00:05,000\nA\n\n' > desordre.srt; subedit-cli disjoint --output x.srt desordre.srt; echo "code $?"
desordre.srt: The subtitles are not in the order of their start: sort them first. Use --sort to do that here.
code 2
```

`--sort` esegue prima l'ordinamento — lo stesso di [`sort`](sort.md) — e lo dice. Il nucleo non ordina mai di propria iniziativa.

Un secondo passaggio non cambia nulla: una volta che gli intervalli sono disgiunti, non c'è più nulla da tagliare, e la
frase dice `already disjoint`.

## Output

**Output standard** — nulla: il risultato è il file scritto.

| Livello | Che cosa si aggiunge sull'errore standard |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles split, M merged -> <destination>`, seguito dopo un `;` da ciò che non è stato possibile fare per intero: `K shown in windows for holding too many lines`, `K very short taken over by a neighbour`, `K subtitles shorter than the limit left whole`, `K merged with styles or positions that differ, the first kept`, `K subtitles ending before the start left alone` — oppure `<path>: already disjoint -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` |
| 3 | `<path>: N bytes read, M written`, poi **ogni messaggio diagnostico di lettura** |

`N subtitles split` conta i sottotitoli originali tagliati in più parti, `M merged` le parti che
mostrano più sottotitoli insieme.

### In JSON

Con [`--format json`](invocation.md#un-output-che-uno-script-può-leggere), i `counts` di ciascun file:

| Conteggio | Che cosa dice |
| :----- | :----------- |
| `subtitles` | i sottotitoli del file scritto |
| `split`, `merged` | come nella frase |
| `windowed` | le parti mostrate in finestre |
| `absorbed` | gli intervalli assorbiti da un vicino |
| `too_short` | i sottotitoli più brevi della soglia, lasciati interi |
| `ending_before_start` | i sottotitoli la cui fine precede l'inizio, lasciati stare |
| `extras_dropped` | le parti unite il cui stile o la cui posizione differiscono |
| `moved` | solo con `--sort`: le posizioni cambiate |

## Codici di uscita

Quelli dello strumento: `0` se ogni file è stato elaborato — un file senza sovrapposizioni lo è —, `2` se
nessuno, `3` se solo alcuni, `1` in caso di errore d'uso.

## Errori

| Che cosa li provoca | Messaggio |
| :------------------- | :------ |
| un file fuori ordine senza `--sort` | `<path>: The subtitles are not in the order of their start: sort them first. Use --sort to do that here.` |
| `--minimum` non è una durata, o è negativo | `--minimum: …` |
| `--max-lines` non è un numero intero | `--max-lines: "…" is not a number of lines: expected a whole number, 0 for no limit` |

Un file illeggibile fallisce come per qualsiasi sottocomando; gli errori della destinazione sono comuni ai
sottocomandi che scrivono: vedere [Invocazione](invocation.md#la-destinazione).

## Negli altri comandi

[`convert`](convert.md#rendere-disgiunti-gli-intervalli), [`append`](append.md#rendere-disgiunti-gli-intervalli) e
[`split-file`](split-file.md#rendere-disgiunti-gli-intervalli) accettano `--disjoint`, che fa la stessa cosa con
le impostazioni predefinite, prima di scrivere.

Su un file segmentato male — un testo spezzato in parti che si susseguono, fornite fuori ordine —, il solo ordinamento e il solo taglio
danno righe che passano da una parte all'altra: prima [`join-repeats`](join-repeats.md), oppure
[`repair`](repair.md), che li concatena tutti e tre.
