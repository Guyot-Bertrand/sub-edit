# ffprobe output for the audio tracks

`ffprobe.audio-2.txt` is what `ffprobe` wrote for `src/test/data/videos/audio-2.mkv`, captured once with:

```console
$ ffprobe -v error -select_streams a -show_entries stream=index:stream_tags=language,title audio-2.mkv
```

It is the reading that `readAudioTracks` (phase 22, audio source preparation) is tested on, so that the parser reads
what the tool writes and not what its documentation suggests. A video with no audio track makes the same command write
nothing, and a file that is not a container makes it exit with an error.
