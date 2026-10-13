# Example

The command below is replayed by the manual generator, and its sample subtitle is French:

<!-- exemple: printf '1\n00:00:01,000 --> 00:00:03,000\nReculez ! [Il\nhurle] Tout de suite, et ne revenez pas !\n\n' > a.srt; subedit-cli hearing-impaired --output b.srt a.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nReculez ! [Il\nhurle] Tout de suite, et ne revenez pas !\n\n' > a.srt; subedit-cli hearing-impaired --output b.srt a.srt
a.srt: 1 subtitle cleaned, 0 removed -> b.srt
```
