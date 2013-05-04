all : srvload srvsave srvcmd
srvload:srvload.c
	mips-linux-gnu-gcc -I/opt/mipslibs/usr/include/libxml2/ -L/opt/mipslibs/usr/lib/ -lxml2 -lz -Wall -o srvload  srvload.c
srvsave:srvsave.c
	mips-linux-gnu-gcc -I/opt/mipslibs/usr/include/libxml2/ -L/opt/mipslibs/usr/lib/ -lxml2 -lz -Wall -o srvsave  srvsave.c
srvcmd:srvcmd.c
	mips-linux-gnu-gcc -Wall -o srvcmd srvcmd.c
.PHONY:all
clean:
	rm -rf srvload srvsave srvcmd

