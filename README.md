# joynd

**joynd** translates SDL joystick input to X11 key presses.

## Help
```

```

## Manpage
```
()									    ()



				 June 1, 2018				    ()

```

## Examples
Prior to sens9x-gtk I run the following scripts:
```
#!/bin/sh
joynd -i 0 \
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

