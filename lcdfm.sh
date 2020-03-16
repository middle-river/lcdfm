#!/bin/sh

export LANG=ja_JP.UTF-8
export TERMINFO=/usr/local/lib/lcdfm/terminfo
export TERM=miniterm
export COLUMNS=30
export LINES=15

dir=$(dirname $0)
cocot -t EUC-JP -p UTF-8 $dir/lcdfm /home/share/music/playlist $dir/lcdfm.conf
