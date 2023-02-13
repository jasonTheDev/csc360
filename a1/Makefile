.phony all:
all: pman

pman: main.c linked_list.c pman_helper.c
	gcc -Wall main.c linked_list.c pman_helper.c -o pman

.PHONY clean:
clean:
	-rm -rf *.o *.exe
