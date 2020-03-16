#!/bin/sh

sed -i 's/geteuid/getppid/' /usr/bin/vlc
