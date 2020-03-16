#!/bin/sh

/bin/echo -n -e "\x11q\x12f\x13d\x14s"

env LANG=ja_JP.UTF-8 mpg123 --utf8 -C -@ "$1" 2>&1 | sed -u -e "s/    /\n/" | sed -u -e "s/^  *//" | sed -u -e "s/  *$//"

/bin/echo -n -e "\x11h\x12j\x13k\x14l"
