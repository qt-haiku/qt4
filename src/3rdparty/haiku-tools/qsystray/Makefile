CC=g++

all: clean qsystray

qsystray: qsystray.o
		$(CC) -g qsystray.o -ltracker -lroot -lbe -o$@
		xres -o qsystray qsystray.rsrc

qsystray.o:
		$(CC) -g -c qsystray.cpp

clean:
		rm -f ./*.o
		rm -f ./qsystray

install:
		cp -f ./qsystray /boot/common/bin/qsystray