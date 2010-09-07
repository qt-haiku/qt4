all: clean qsystray qsystray_gcc2

qsystray: 
		setgcc gcc4
		sleep 1
		make -f Makefile.gcc4

qsystray_gcc2: 
		setgcc gcc2
		sleep 1
		make -f Makefile.gcc2
		setgcc gcc4
		sleep 1
		
clean:
		make -f Makefile.gcc4 clean
		make -f Makefile.gcc2 clean

install:
		make -f Makefile.gcc4 install
		make -f Makefile.gcc2 install

		make -f Makefile.gcc4 clean
		make -f Makefile.gcc2 clean

