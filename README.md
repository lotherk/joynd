# joynd

**joynd** translates SDL joystick input to X11 key presses.

## Help
```
joynd 0.1 Copyright (C) 2018 Konrad Lother <k@hiddenbox.org>

This software is supplied WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. This is free
software, and you are welcome to redistribute it under certain conditions; see
the template COPYING for details.

Translates joystick input to key presses

Usage: joynd [-h|--help] [-V|--version] [-l|--list] [-iINT|--input=INT]
         [-D|--debug] [-bN=KEY[+KEY1[+KEY2]]|--map-button=N=KEY[+KEY1[+KEY2]]]
         [-aN[:MIN:MAX]=KEY[+KEY1[+KEY2]]|--map-axis=N[:MIN:MAX]=KEY[+KEY1[+KEY2]]]
         [-d|--daemon]

  -h, --help                    Print help and exit
  -V, --version                 Print version and exit


  -l, --list                    List all available joysticks and exit
                                  (default=off)
  -i, --input=INT               Define joystick input number (0 is first, 1 is
                                  second, ...)
  -D, --debug                   Show debug informations  (default=off)
  -b, --map-button=N=KEY[+KEY1[+KEY2]]
                                Map button number N to key KEY
  -a, --map-axis=N[:MIN:MAX]=KEY[+KEY1[+KEY2]]
                                Map axis N to KEY
  -d, --daemon                  Fork into background  (default=off)

See joynd(1) for more informations and examples.

```

## Manpage
```
()									    ()



				 June 1, 2018				    ()

```

## Examples
Prior to sens9x-gtk I run the following script:
```
#!/bin/sh
joynd -d -i 0 \
        -b 7=q \
        -b 4=w \
        -b 5=e \
        -b 6=r \
        -b 13=t \
        -b 14=z \
        -b 15=u \
        -b 12=i \
        -b 3=o \
        -b 0=p \
        -b 10=a \
        -b 11=s
```

