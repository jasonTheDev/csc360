/* 
    Jason Kepler - V00848837
    Diskinfo - A FAT12 Disk Manager
    CSC 360
*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>     // memcpy()

#include "diskhelper.h"


/*------------ Definitions ------------*/

#define OS_NAME_SIZE 8
#define OS_NAME_OFFSET 3
#define VOL_LABLE_SIZE 11
#define VOL_LABLE_OFFSET 43


/*------------ Helper Functions ------------*/

void printUsage() {
    printf("Usage: ./diskinfo DISK\n");
}


/*
 * Prints the formatted information about the disk
 * 
 */
void printDiskInfo( char *os_name,
                    char *vol_lable,
                    int disk_size,
                    int free_disk_size,
                    int num_files,
                    int num_fat_copies,
                    int sectors_per_fat)
{
    printf("OS Name: %s\n", os_name);
    printf("Label of the disk: %s\n", vol_lable);
    printf("Total size of the disk: %d bytes\n", disk_size);
    printf("Free size of the disk: %d bytes\n", free_disk_size);
    printDivider();
    printf("The number of files in the disk: %d\n", num_files);
    printDivider();
    printf("Number of FAT copies: %d\n", num_fat_copies);
    printf("Sectors per FAT: %d\n", sectors_per_fat);
}


/*
 * Gets the malloc'ed string from mapped data given offset and length
 * Returns: malloc'ed string
 */
char * cpyAsString(char *src, int offset, int length) {
    char * str;
    str = (char *) malloc(sizeof(char) * (length + 1));
    if (str == NULL) {
        printf("ERROR: malloc failed\n");
		exit(EXIT_FAILURE);
    }
    memcpy(str, src + offset, sizeof(char) * length);
    str[length] = '\0';

    return str;
}


/*
 * Gets the volume OS of disk from mapped memory
 * Returns: volume OS as malloc'ed string
 */
char * getOSName(char * p) {
    return cpyAsString(p, OS_NAME_OFFSET, OS_NAME_SIZE);
}


/*
 * Gets the volume label of disk from mapped memory
 * Returns: volume label as malloc'ed string
 */
char * getVolumeLabel(char * p) {
    char *label = cpyAsString(p, VOL_LABLE_OFFSET, VOL_LABLE_SIZE);
    if (label[0] == ' ') {
        p += BYTES_PER_SECTOR * ROOT_SECTOR_INDEX;
        while (p[0] != 0x00) {
			if (isVolumeLabel(p)) {
                memcpy(label, p, VOL_LABLE_SIZE);
                break;
			}
			p += 32;
		}
    }

    return label;
}


/*
 * Recursively counts all files in the subdirectory indicated by the
 * given fat_index, and all files in the subdirectories inside subdirectory
 * Returns: count of all files in subdirectory
 */
int rCountFilesInSubdir(char *p, int fat_index) {
    char *entry = getDataEntry(p, fat_index);

    int count = 0, i = 0;
    while (i < 16 && !isRemainingFree(entry)) {
        if (isValidFile(entry)) {
            count++;
        }
        else if (isValidSubdir(entry)) {
            int fat_index_of_subdir = getFirstLogicalCluster(entry);
            count += rCountFilesInSubdir(p, fat_index_of_subdir);
        }
        i++;
        entry += 32;
    }

    // current directory is longer than one sector
    if (!isLastCluster(p, fat_index)) {
        count += rCountFilesInSubdir(p, getFATEntry(p, fat_index));
    }

    return count;
}


/*
 * Recursively counts all files on the disk starting at the root
 * Returns: count of all files
 */
int getTotalNumFiles(char * p) {
    char *entry = p + BYTES_PER_SECTOR * ROOT_SECTOR_INDEX;

    int count = 0;
    while (!isRemainingFree(entry)) {
        if (isValidFile(entry)) {
            count++;
        }
        else if (isValidSubdir(entry)) {
            int fat_index_of_subdir = getFirstLogicalCluster(entry);
            count += rCountFilesInSubdir(p, fat_index_of_subdir);
        }
        entry += 32;
    }

    return count;
}


/*------------ Main Function ------------
 *
 * Diskinfo
 * Prints infomation about a disk imaged passed to program in arguments
 */
int main(int argc, char *argv[])
{
	int fd, disk_size, free_disk_size, num_files, num_fat_copies, sectors_per_fat;
    struct stat sb;
    char *p, *os_name, *vol_lable;

    // check arguments
    if (argc != 2) {
        printf("ERROR: incorrect number of arguments\n");
        printUsage();
        exit(EXIT_FAILURE);
    }

    // open file
	fd = open(argv[1], O_RDWR);
    if (fd == -1) {
        printf("ERROR: failed to open file %s\n", argv[1]);
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

    // get info to print
    os_name = getOSName(p);
    vol_lable = getVolumeLabel(p);
    disk_size = getSizeOfDisk(p);
    free_disk_size = getFreeSizeofDisk(p);
    num_files = getTotalNumFiles(p);
    num_fat_copies = getNumFatCopies(p);
    sectors_per_fat = getSectorsPerFAT(p);

    // print info
    printDiskInfo(os_name, vol_lable, disk_size, free_disk_size, num_files, num_fat_copies, sectors_per_fat);
	
    free(os_name);
    free(vol_lable);

	return 0;
}