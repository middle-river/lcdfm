#!/bin/sh

/bin/echo -n -e "\x11q\x12n\x13p\x14 "

eject -x 1

vlc -I ncurses -v 2 `cat "$1"` 2>&1

/bin/echo -n -e "\x11h\x12j\x13k\x14l"
