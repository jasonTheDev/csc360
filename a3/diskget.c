/* 
 * Jason Kepler
 * Diskget - A FAT12 Disk Manager
 */

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>     // memcpy()

#include "diskhelper.h"


/*------------ Helper Functions ------------*/

void printUsage() {
    printf("Usage: ./diskget DISK FILE\n");
}


/*
 * Formats filename and extention to compare with FAT12 entries
 * 
 */
void formatFilenameAndExtention(char *input, char *filename) {
    int i = 0;
    while (input[i] != '\0') {
        if (input[i] == '.') {
            filename[i] = '.';
        }
        else {
            filename[i] = toupper(input[i]);
        }
        i++;
    }
    filename[i] = '\0';
}


/*
 * Get the entry of file if it exists in the root directory of disk
 * Return: pointer to entry if file exists, otherwise NULL
 */
char *getEntryIfFileInRoot(char *p, char *filename) {
    char *entry = p + BYTES_PER_SECTOR * ROOT_SECTOR_INDEX;

    while(!isRemainingFree(entry)) {
        if (isValidFile(entry)) {
            char entry_name[13];
            getFileName(entry, entry_name);
            if (strcmp(entry_name, filename) == 0){
                return entry;
            }
        }
        entry += 32;
    }

    return NULL;
}


/*
 * Writes data to a file in working directory
 * Return: 0 on success, -1 otherwise
 */
int writeDataToFile(char *p, FILE *fptr, int fat_index, size_t size) {
    char *entry = getDataEntry(p, fat_index);

    while (size > BYTES_PER_SECTOR) {
        if (fat_index < 2) {return -1;}
        fwrite(entry, BYTES_PER_SECTOR, 1, fptr);
        size -= BYTES_PER_SECTOR;
        fat_index = getFATEntry(p, fat_index);
        entry = getDataEntry(p, fat_index);
    }

    fwrite(entry, size, 1, fptr);
    return 0;
}


/*------------ Main Function ------------
 *
 * Diskget
 * Find at given file in the root directory of the given disk and writes
 * it's contents to a file of the same name in the users working directory
 */
int main(int argc, char *argv[])
{
	int fd, fat_index, file_size;
    struct stat sb;
    char *p, *root_entry;
    FILE *fptr;

    // check arguments
    if (argc != 3) {
        printf("ERROR: incorrect number of arguments\n");
        printUsage();
        exit(EXIT_FAILURE);
    }

    // open file
	fd = open(argv[1], O_RDWR);
    if (fd == -1) {
        printf("ERROR: failed to open disk image %s\n", argv[1]);
        exit(EXIT_FAILURE);
    }

	fstat(fd, &sb);

    // map file
	p = mmap(NULL, sb.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (p == MAP_FAILED) {
		printf("ERROR: failed to map memory\n");
		exit(EXIT_FAILURE);
	}
    close(fd);

    // check length of filename, no long names
    if (strlen(argv[2]) > 12) {
        printf("ERROR: filename too long\n");
        exit(EXIT_FAILURE);
    }

    // get data entry of file if in root
    char filename[13];
    formatFilenameAndExtention(argv[2], &filename[0]);
    if ((root_entry = getEntryIfFileInRoot(p, &filename[0])) == NULL) {
        printf("File not found.\n");
        exit(EXIT_FAILURE);
    }

    // write file from disk to working directory
    if ((fptr = fopen(&filename[0], "wb")) == NULL) {
        printf("ERROR: failed to open file %s\n", argv[2]);
        exit(EXIT_FAILURE);
    }
    file_size = getFileSize(root_entry);
    fat_index = getFirstLogicalCluster(root_entry);
    writeDataToFile(p, fptr, fat_index, file_size);

    fclose(fptr);

    return 0;
}