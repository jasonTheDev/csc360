.PHONY all:
all: ACS

ACS: ACS.c safe_pthread.c acs_helper.c
	gcc -Wall ACS.c safe_pthread.c acs_helper.c -o ACS -pthread

.PHONY clean:
clean:
	rm -rf *.o *.exe
