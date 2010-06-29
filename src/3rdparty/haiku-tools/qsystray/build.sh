#!/bin/sh
setgcc gcc4
sleep 1
make -f Makefile clean
make -f Makefile
make -f Makefile install

setgcc gcc2
sleep 1
make -f Makefile.gcc2 clean
make -f Makefile.gcc2
make -f Makefile.gcc2 install

setgcc gcc4
