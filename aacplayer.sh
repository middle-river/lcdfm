#!/bin/sh

/bin/echo -n -e "\x11q\x12n\x13p\x14 "

vlc -I ncurses -v 2 --volume 127 "$1" 2>/dev/null

/bin/echo -n -e "\x11h\x12j\x13k\x14l"
