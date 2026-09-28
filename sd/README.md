# SD card layout for NFCNoisy

Format the card **FAT32**.

```
/sounds/cat.wav
/sounds/dog.wav
/sounds/cow.wav
...
/mapping.txt          (created by the toy after setup)
```

About 10 short animal WAVs is the starting set. Names are sorted A→Z in setup mode.

## mapping.txt

Written automatically. You can also edit it on a computer:

```
# NFCNoisy UID -> wav  (SETUP is the pairing card)
SETUP 04AABBCCDDEEFF
04AABB112233 /sounds/cat.wav
04AABB445566 /sounds/dog.wav
```

UIDs are printed on the serial monitor when you tap.

## Convert a file

```bash
ffmpeg -i input.mp3 -ar 22050 -ac 1 -sample_fmt s16 cat.wav
```
