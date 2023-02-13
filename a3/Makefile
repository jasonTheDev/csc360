.PHONY all:
all: diskinfo disklist diskget diskput

diskinfo: diskinfo.c diskhelper.c
	gcc -Wall diskinfo.c diskhelper.c -o diskinfo

disklist: disklist.c diskhelper.c
	gcc -Wall disklist.c diskhelper.c -o disklist

diskget: diskget.c diskhelper.c
	gcc -Wall diskget.c diskhelper.c -o diskget

diskput: diskput.c diskhelper.c
	gcc -Wall diskput.c diskhelper.c -o diskput

.PHONY clean:
clean:
	rm -rf *.o *.exe
