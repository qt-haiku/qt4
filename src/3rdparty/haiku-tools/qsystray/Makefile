CC=g++
SIGNATURE="application/x-vnd.QtSystrayManager_gcc4"

all: qsystray

qsystray: qsystray.o
		$(CC) -g qsystray.o -ltracker -lroot -lbe -oqsystray
		settype -s $(SIGNATURE) ./qsystray

qsystray.o:
		$(CC) -g -DGCC4BUILD=1 -DAPP_SIGNATURE="\"$(SIGNATURE)\"" -c qsystray.cpp

clean:
		rm -f ./*.o
		rm -f ./qsystray
		rm -f ./qsystray_gcc2

install:
		cp -f ./qsystray /boot/common/bin/qsystray
