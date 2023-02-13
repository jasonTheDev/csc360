/* 
    Jason Kepler - V00848837
    Diskput - A FAT12 Disk Manager
    CSC 360
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


/*------------ Definitions ------------*/

#define PATH_SEP '/'


/*------------ Helper Functions ------------*/

void printUsage() {
    printf("Usage: ./diskput DISK PATH/FILE\n");
}


/*
 * Capitalizes the given string
 * 
 */
void capitalizeString(char *string) {
    int len = strlen(string);

    for (int i = 0; i < len; i++) {
        string[i] = toupper(string[i]);
    }
}


/*
 * Gets the filename in the given path
 * Returns: pointer to filename
 */
char *getFilenameFormPath(char *path) {
    int len = strlen(path);
    if (path[len-1] == PATH_SEP) {
        return NULL;
    }

    int i;
    for (i = len-1; i >= 0; i--) {
        if (path[i] == PATH_SEP) {
            return &path[i+1];
        }
    }
    return path;
}


/*
 * Extracts the name of the next subdirectory from path
 * Returns: pointer to remaining path, or NULL if end of path
 */
char *getNextSubdirName(char *path, char*subdir_name) {
    char *filename = getFilenameFormPath(path);

    if (path == filename) {
        return NULL;
    }
    
    int i = 0;
    while (path[i] != PATH_SEP) {
        if (i > 8) {
            printf("ERROR: directory name too long.\n");
            exit(EXIT_FAILURE);
        }
        subdir_name[i] = path[i];
        i++;
    }
    subdir_name[i] = '\0';

    return &path[i+1];
}


/*
 * Sets the entry to have the given filename
 * 
 */
void setEntryToFilename(char *data_entry, char *filename) {
    int i = 0, j = 0;
    while (filename[j] != '.') {
        data_entry[i] = filename[j];
        i++;
        j++;
    }
    while (i < 8) {
        data_entry[i] = ' ';
        i++;
    }

    j++;
    int len = strlen(filename);
    while (j < len) {
        data_entry[i] = filename[j];
        i++;
        j++;
    }
}


/*
 * Gets the first free entry in FAT
 * Returns: the fat index
 */
int getFirstFreeFATIndex(char *p) {

    int fat_index = 2;
    int fat_entry = getFATEntry(p, fat_index);
    while (fat_entry != 0x00) {
        fat_index++;
        fat_entry = getFATEntry(p , fat_index);
    }

    return fat_index;
}


/*
 * Gets the second free entry in FAT
 * Returns: the fat index
 */
int getSecondFreeFATIndex(char *p) {

    int fat_index = 2;
    int fat_entry = getFATEntry(p, fat_index);
    while (fat_entry != 0x00) {
        fat_index++;
        fat_entry = getFATEntry(p , fat_index);
    }

    fat_index++;
    fat_entry = getFATEntry(p, fat_index);
    while (fat_entry != 0x00) {
        fat_index++;
        fat_entry = getFATEntry(p, fat_index);
    }

    return fat_index;
}


/*
 * Sets the fat index to new value
 * 
 */
void setFATEntry(char *p, int fat_index, uint16_t new_entry) {
    int location = 3 * fat_index / 2;
    p += BYTES_PER_SECTOR + location;

    if (fat_index % 2 == 0) {
        p[0] = (uint8_t) new_entry;
        p[1] = (uint8_t) ((uint8_t) p[1] & 0b11110000) + (uint8_t) ((new_entry & 0b111100000000) >> 8);
    }
    else {
        p[0] = (uint8_t) ((uint8_t) p[0] & 0b00001111) + ((new_entry << 4) & 0b11110000);
        p[1] = (uint8_t) ((new_entry >> 4) & 0b11111111);   
    }
}


/*
 * Recursively writes the contents of the file to the disk
 * 
 */
void rWriteFileToDisk(char *p, char *put_p, int size, int fat_index) {
    int next_fat_index;
    int fat_entry;
    char *entry;

    entry = getDataEntry(p, fat_index);

    // get the next fat entry
    fat_entry = getFATEntry(p, fat_index);
    if (fat_entry == 0x00) {
        next_fat_index = getSecondFreeFATIndex(p);
    }
    else if (fat_entry >= 0xFF8) {
        next_fat_index = getFirstFreeFATIndex(p);
    }
    else {
        next_fat_index = fat_entry;
    }

    // base case, last bytes of file
    if (size <= BYTES_PER_SECTOR) {
        memcpy(entry, put_p, (size_t) size);
        setFATEntry(p, fat_index, 0xFFF);
    }

    // otherwise, write 512 bytes
    else {
        setFATEntry(p, fat_index, next_fat_index);
        memcpy(entry, put_p, (size_t) BYTES_PER_SECTOR);
        rWriteFileToDisk(p, put_p + BYTES_PER_SECTOR, size - BYTES_PER_SECTOR, next_fat_index);
    }
}


/*
 * Sets the first logical custer of entry
 * 
 */
void setFirstLogicalCluster(char *data_entry, unsigned new_cluster) {
    data_entry[26] = (uint8_t) new_cluster;
    data_entry[27] = (uint8_t) (new_cluster >> 8);
}


/*
 * Sets the file size of entry
 * 
 */
void setEntryFileSize(char *data_entry, int size) {
    data_entry[28] = (uint8_t) size;
    data_entry[29] = (uint8_t) (size >> 8);
    data_entry[30] = (uint8_t) (size >> 16);
    data_entry[31] = (uint8_t) (size >> 24);
}


/*
 * Checks if the filename  matches the name given in the path
 * 
 */
int isMatchingFilename(char *data_entry, char *filename) {
    char entry_filename[13];
    getFileName(data_entry, &entry_filename[0]);
    return strcmp(entry_filename, filename) == 0;
}


/*
 * Checks if the subdir name matches the name given in the path
 * 
 */
int isMatchingSubirname(char *data_entry, char *name_from_path) {
    char entry_name[9];
    getDirName(data_entry, &entry_name[0]);
    return strcmp(entry_name, name_from_path) == 0;
}


/*
 * Gets a pointer to the file entry if present root
 * Returns: pointer to entry on success, NULL otherwise
 */
char *getEntryIfFileInRoot(char *p, char *filename) {
    char *entry = p + ROOT_SECTOR_INDEX * BYTES_PER_SECTOR;

    while (!isRemainingFree(entry)) {
        if (isValidFile(entry) && isMatchingFilename(entry, filename)) {
            return entry;
        }
        entry += 32;
    }
    return NULL;
}


/*
 * Gets a pointer to the file entry if present in given directory
 * Returns: pointer to entry on success, NULL otherwise
 */
char *getEntryIfFileInSubdir(char *p, int fat_index, char *filename) {
    char *entry = getDataEntry(p, fat_index);

    int i = 0;
    while (i < 16 && !isRemainingFree(entry)) {
        if (isValidFile(entry) && isMatchingFilename(entry, filename)) {
            return entry;
        }
        i++;
        entry += 32;
    }
    return NULL;
}


/*
 * Gets pointer to the first free entry of root
 * Returns: pointer to entry
 */
char *getFirstFreeEntryInRoot(char *p) {
    char *entry = p + ROOT_SECTOR_INDEX * BYTES_PER_SECTOR;

    while (!isRemainingFree(entry) && !isFreeEntry(entry)) {
        entry += 32;
    }
    return entry;
}


/*
 * Gets pointer to the first free entry of the given Directory
 * Returns: pointer to entry on success, NULL otherwise
 */
char *getFirstFreeEntryInDir(char *p, int fat_index) {
    char *entry = getDataEntry(p, fat_index);

    int i = 0;
    while (i < 16) {
        if (isRemainingFree(entry) || isFreeEntry(entry)) {
            return entry;
        }
        i++;
        entry += 32;
    }


    if (!isLastCluster(p, fat_index)) {
        return getFirstFreeEntryInDir(p, getFATEntry(p, fat_index));
    }

    return NULL;
}


/*
 * Recursively finds the fat index of the subdirectory specified in the path
 * Returns: fat index on success, -1 otherwise
 */
int rGetFATIndexOFSubdir(char *p, int fat_index, char *path) {
    char *remaining_path, *entry;

    char subdir_name[9];
    if ((remaining_path = getNextSubdirName(path, &subdir_name[0])) == NULL) {
        return fat_index;
    }
    else {
        entry = getDataEntry(p, fat_index);

        int i = 0;
        while (i < 16 && !isRemainingFree(entry)) {
            if (isSubdir(entry) && isMatchingSubirname(entry, subdir_name)) {
                int cluster = getFirstLogicalCluster(entry);
                return rGetFATIndexOFSubdir(p, cluster, remaining_path);
            }
            i++;
            entry += 32;
        }

        if (!isLastCluster(p, fat_index)) {
            return rGetFATIndexOFSubdir(p, getFATEntry(p, fat_index), path);
        }
    }

    return -1;
}


/*------------ Main Function ------------
 *
 * Diskput
 * Copies the contents of a given file on the user's working directory
 * to a file of the same name in the sepified directory on disk
 */
int main(int argc, char *argv[])
{
	int disk_fd, put_fd, put_size, free_disk_size, free_fat_index;
    struct stat disk_sb, put_sb;
    char *p, *put_p, *put_filename, *free_entry, *path;

    free_entry = NULL;

    // check arguments
    if (argc != 3) {
        printf("ERROR: incorrect number of arguments\n");
        printUsage();
        exit(EXIT_FAILURE);
    }

    // check if file exits on linux
    if ((put_filename = getFilenameFormPath(argv[2])) == NULL) {
        printf("ERROR: invalid path/to/file: %s\n", argv[2]);
        printUsage();
        exit(EXIT_FAILURE);
    }

    // check length of filename, no long names
    if (strlen(put_filename) > 12) {
        printf("ERROR: filename too long\n");
        exit(EXIT_FAILURE);
    }

    // open put file
	put_fd = open(put_filename, O_RDWR);
    if (put_fd == -1) {
        printf("File not found: %s\n", put_filename);
        exit(EXIT_FAILURE);
    }
	fstat(put_fd, &put_sb);

    // open disk
	disk_fd = open(argv[1], O_RDWR);
    if (disk_fd == -1) {
        printf("Disk image not found: %s\n", argv[1]);
        exit(EXIT_FAILURE);
    }
	fstat(disk_fd, &disk_sb);

    // map file
	put_p = mmap(NULL, put_sb.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, put_fd, 0);
	if (put_p == MAP_FAILED) {
		printf("ERROR: failed to map memory of %s\n", put_filename);
		exit(EXIT_FAILURE);
	}
    put_size = (uint64_t)put_sb.st_size;
    close(put_fd);

    // map disk image
	p = mmap(NULL, disk_sb.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, disk_fd, 0);
	if (p == MAP_FAILED) {
		printf("ERROR: failed to map memory of %s\n", argv[1]);
		exit(EXIT_FAILURE);
	}
    close(disk_fd);

    // check enough space
    if ((free_disk_size = getFreeSizeofDisk(p)) < put_size) {
        printf("Not enough free space in the disk image.\n");
        exit(EXIT_FAILURE);
    }

    // format path
    capitalizeString(argv[2]);
    path = argv[2];

    // skip the '/' at the start of the path
    if (path[0] == PATH_SEP) {
        path += 1;
    }

    char subdir_name[9];
    // path points to the root directory
    if ((path = getNextSubdirName(path, &subdir_name[0])) == NULL) {
        
        // file already exits in root
        if ((free_entry = getEntryIfFileInRoot(p, put_filename)) != NULL) {
            setEntryFileSize(free_entry, put_size);
            rWriteFileToDisk(p, put_p, put_size, getFirstLogicalCluster(free_entry));
        }

        // file doesn't exist, write to free entry
        else if ((free_entry = getFirstFreeEntryInRoot(p)) != NULL) {
            free_fat_index = getFirstFreeFATIndex(p);
            setEntryToFilename(free_entry, put_filename);
            setFirstLogicalCluster(free_entry, free_fat_index);
            setEntryFileSize(free_entry, put_size);
            rWriteFileToDisk(p, put_p, put_size, free_fat_index);
        }
    }

    // path doesn't point to the root directory
    else {
        int subdir_fat_index;
        char *entry = p + ROOT_SECTOR_INDEX * BYTES_PER_SECTOR;

        while (TRUE) {
            if (isSubdir(entry) && isMatchingSubirname(entry, &subdir_name[0])) {
                subdir_fat_index = getFirstLogicalCluster(entry);
                subdir_fat_index = rGetFATIndexOFSubdir(p, subdir_fat_index, path);
                break;
            }
            else if (isRemainingFree(entry)) {
                printf("The directory was not found.\n");
                exit(EXIT_FAILURE);
            }
            entry += 32;
        }
        if (subdir_fat_index < 2) {
            printf("The directory was not found.\n");
            exit(EXIT_FAILURE);
        }

        // file exits already... override
        if ((free_entry = getEntryIfFileInSubdir(p, subdir_fat_index, put_filename)) != NULL) {
            setEntryFileSize(free_entry, put_size);
            rWriteFileToDisk(p, put_p, put_size, getFirstLogicalCluster(free_entry));
        }

        // write to a free entry in the subdir
        else if ((free_entry = getFirstFreeEntryInDir(p, subdir_fat_index)) != NULL) {
            free_fat_index = getFirstFreeFATIndex(p);
            setEntryToFilename(free_entry, put_filename);
            setFirstLogicalCluster(free_entry, free_fat_index);
            setEntryFileSize(free_entry, put_size);
            rWriteFileToDisk(p, put_p, put_size, free_fat_index);
        }

        // no free entries in subdir, need to add entries
        else {
            // update fat entries
            free_fat_index = getFirstFreeFATIndex(p);
            setFATEntry(p, subdir_fat_index, free_fat_index);
            setFATEntry(p, free_fat_index, 0xFFF);
            
            // skip '.' and '..' directories
            entry = getDataEntry(p, free_fat_index);
            while (!isFreeEntry(entry) && !isRemainingFree(entry)) {
                entry += 32;
            }
            free_entry = entry;

            // write file to disk
            free_fat_index = getFirstFreeFATIndex(p);
            setEntryToFilename(free_entry, put_filename);
            setFirstLogicalCluster(free_entry, free_fat_index);
            setEntryFileSize(free_entry, put_size);
            rWriteFileToDisk(p, put_p, put_size, free_fat_index);            
        }
    }

    return 0;
}