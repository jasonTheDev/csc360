/* 
 * Jason Kepler
 * Disklist - A FAT12 Disk Manager
 */

#include <stdio.h>
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
    printf("Usage: ./disklist DISK\n");
}


int getCreationYear(char *data_entry) {
    return ((data_entry[17] & 0b11111110) >> 1) + 1980;
}


int getCreationMonth(char *data_entry) {
    return ((data_entry[17] & 0b00000001) << 3) + ((data_entry[16] & 0b11100000) >> 5);
}


int getCreationDay(char *data_entry) {
    return data_entry[16] & 0b00011111;
}


int getCreationHour(char *data_entry) {
    return (data_entry[15] & 0b11111000) >> 3;
}


int getCreationMinute(char *data_entry) {
    return ((data_entry[15] & 0b00000111) << 3) + ((data_entry[14] & 0b11100000) >> 5);
}


void printDirectoryHeader(char *dir_name) {
    printf("%s", dir_name);
    printDivider();
}


void printDirName(char *data_entry) {
    char dir_name[9];
    getDirName(data_entry, &dir_name[0]);
    printf("%s", dir_name);
}


/*
 * Prints the formatted information about a directory
 * 
 */
void printSubdirInfo(char *data_entry) {
    char dir_name[9];
    getDirName(data_entry, &dir_name[0]);
    int size = getFileSize(data_entry);

    printf("D %10u %20s\n", size, dir_name);
}


/*
 * Prints the formatted information about a file
 * 
 */
void printFileInfo(char *data_entry) {
    char filename[13];
    getFileName(data_entry, &filename[0]);
    int size = getFileSize(data_entry);
    int year = getCreationYear(data_entry);
    int month = getCreationMonth(data_entry);
    int day = getCreationDay(data_entry);
    int hour = getCreationHour(data_entry);
    int minute = getCreationMinute(data_entry);

    printf("F %10u %20s %u-%02u-%02u %02u:%02u\n", size, filename, year, month, day, hour, minute);
}


/*
 * Prints the formatted information about directory and subdirectories
 * 
 */
void rPrintList(char *p, int fat_index) {
    char *entry = getDataEntry(p, fat_index);

    // print current directory info
    int i = 0;
    while (i < 16 && !isRemainingFree(entry)) {
        if (isValidFile(entry)) {
            printFileInfo(entry);
        }
        else if (isValidSubdir(entry)) {
            printSubdirInfo(entry);
        }
        i++;
        entry += 32;
    }

    // current directory is longer than one sector
    if (!isLastCluster(p, fat_index)) {
        rPrintList(p, getFATEntry(p, fat_index));
    }

    // recurse on all subdirs in current dir
    entry = getDataEntry(p, fat_index);
    i = 0;
    while (i < 16 && !isRemainingFree(entry)) {
        if (isValidSubdir(entry)) {
            printDirName(entry);
            printDivider();
            int fat_index_of_subdir = getFirstLogicalCluster(entry);
            rPrintList(p, fat_index_of_subdir);
        }
        i++;
        entry += 32;
    }
}


/*
 * Prints the formatted information about disk starting at root
 * 
 */
void printList(char *p) {
    char *entry = p + BYTES_PER_SECTOR * ROOT_SECTOR_INDEX;

    printf("ROOT");
    printDivider();

    // print out info for root directory
    while (!isRemainingFree(entry)) {
        if (isValidFile(entry)) {
            printFileInfo(entry);
        }
        else if (isValidSubdir(entry)) {
            printSubdirInfo(entry);
        }
        entry += 32;
    }

    entry = p + BYTES_PER_SECTOR * ROOT_SECTOR_INDEX;
    while (!isRemainingFree(entry)) {
        if (isValidSubdir(entry)) {
            printDirName(entry);
            printDivider();
            int fat_index_of_subdir = getFirstLogicalCluster(entry);
            rPrintList(p, fat_index_of_subdir);
        }
        entry += 32;
    }
}


/*------------ Main Function ------------
 *
 * Disklist
 * Prints list of inforamtion about each directory and file on disk
 */
int main(int argc, char *argv[])
{
	int fd;
    struct stat sb;
    char *p;

    // check arguments
    if (argc != 2) {
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

    // print list of directories and files
    printList(p);

    return 0;
}