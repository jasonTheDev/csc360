/* 
    Jason Kepler - V00848837
    Helper functions for Diskinfo, Disklist, Diskput and Diskget
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
#include <string.h>

#include "diskhelper.h"


/*------------ Output ------------*/

void printDivider() {
    printf("\n==================\n");
}

/*------------ Functions on disk mapping ------------*/

int getNumFatCopies(char * p) {
    return (uint8_t)p[16];
}


int getSectorsPerFAT(char * p) {
    return (uint8_t)p[22] + ((p[23] & 0b11111111) << 8);
}


int getTotalSectorCount(char * p) {
    return (uint8_t) p[19] + ((uint8_t) p[20] << 8);
}


int getDataSectorCount(char * p) {
    return getTotalSectorCount(p) - DATA_AREA_INDEX;
}


int getSizeOfDisk(char * p) {
    return getTotalSectorCount(p) * BYTES_PER_SECTOR;
}


int getFreeSizeofDisk(char * p) {
    int sector_count = getDataSectorCount(p);

    unsigned short entry;
    int num_free_entries = 0;
    for (int n = 2; n < sector_count + 2; n++) {
        entry = getFATEntry(p, n);
        if (entry == 0x00) {
            num_free_entries++;
        }
    }

    return num_free_entries * BYTES_PER_SECTOR;
}


char *getDataEntry(char *p, int fat_index) {
    return p + (31 + fat_index) * BYTES_PER_SECTOR;
}


int getFATEntry(char * p, int fat_index) {
    unsigned short entry;
    p += BYTES_PER_SECTOR;
    int location = 3 * fat_index / 2;

    unsigned short first_byte = (uint8_t) p[location];
    unsigned short second_byte = (uint8_t) p[location + 1];

    if (fat_index % 2 == 0) {
        second_byte = second_byte & 0x0F;
        entry = first_byte + (second_byte << 8);
    }
    else {
        first_byte = first_byte & 0xF0;
        second_byte = (second_byte << 4) & 0xFF0;
        entry = (first_byte >> 4) + second_byte;
    }

    return entry;
}


int isLastCluster(char *p, int fat_index) {
    uint16_t fat_entry = getFATEntry(p, fat_index);
    return fat_entry >= 0xFF8;
}


/*------------ Functions on Data Entries ------------*/

int isRemainingFree(char *data_entry) {
    return (uint8_t)data_entry[0] == 0x00;
}


int isLongFileName(char *data_entry) {
    return (uint8_t)data_entry[11] == 0x0F;
}


int getFirstLogicalCluster(char *data_entry) {
    return (uint8_t)data_entry[26] + ((data_entry[27] & 0b11111111) << 8);
}


int isFreeEntry(char *data_entry) {
    return (uint8_t)data_entry[0] == 0xE5;
}


int isParentOrSelf(char *data_entry) {
    return data_entry[0] == '.';
}


int isValidFileOrSubdir(char *data_entry) {
    return  !isParentOrSelf(data_entry)
            && !isVolumeLabel(data_entry)
            && !isFreeEntry(data_entry)
            && !isLongFileName(data_entry);
}


int isValidFile(char *data_entry) {
    return isValidFileOrSubdir(data_entry)
            && !isSubdir(data_entry);
}


int isValidSubdir(char *data_entry) {
    return isValidFileOrSubdir(data_entry)
            && isSubdir(data_entry);
}


int getFileSize(char *data_entry) {
    return (uint8_t)data_entry[28]
            + ((data_entry[29] & 0b11111111) << 8)
            + ((data_entry[30] & 0b11111111) << 16)
            + ((data_entry[31] & 0b11111111) << 24);
}


int isVolumeLabel(char *data_entry) {
    return (uint8_t)data_entry[11] == 0x08;
}


int isSubdir(char * data_entry) {
    return (data_entry[11] & 0b00010000) != 0;
}


char *getFileName(char *data_entry, char *filename) {
    int i = 0;
    while (i < 8 && data_entry[i] != ' ') {
        filename[i] = data_entry[i];
        i++;
    }
    filename[i++] = '.';
    int j = 8;
    while (j < 11 && data_entry[j] != ' ') {
        filename[i] = data_entry[j];
        i++;
        j++;
    }
    filename[i] = '\0';

    return filename;
}


char *getDirName(char *data_entry, char *dir_name) {
    int i = 0;
    while (i < 8 && data_entry[i] != ' ') {
        dir_name[i] = data_entry[i];
        i++;
    }
    dir_name[i] = '\0';

    return dir_name;
}
