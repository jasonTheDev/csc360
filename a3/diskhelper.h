#ifndef _DISKHELPER_H_
#define _DISKHELPER_H_

/*------------ Definitions ------------*/
#define TRUE 1
#define FALSE 0

#define BYTES_PER_SECTOR 512
#define ROOT_SECTOR_INDEX 19
#define DATA_AREA_INDEX 33


/*------------ Output ------------*/
void printDivider();

/*------------ Functions on disk mapping ------------*/
int getNumFatCopies(char * p);
int getSectorsPerFAT(char * p);
int getTotalSectorCount(char * p);
int getDataSectorCount(char * p);
int getSizeOfDisk(char * p);
int getFreeSizeofDisk(char * p);

char *getDataEntry(char *p, int fat_index);
int getFATEntry(char * p, int fat_index);
int isLastCluster(char *p, int fat_index);

/*------------ Functions on Data Entries ------------*/
int isRemainingFree(char *data_entry);
int isLongFileName(char *data_entry);
int getFirstLogicalCluster(char *data_entry);
int isFreeEntry(char *data_entry);
int isParentOrSelf(char *data_entry);
int isValidFileOrSubdir(char *data_entry);
int isValidFile(char *data_entry);
int isValidSubdir(char *data_entry);
int getFileSize(char *data_entry);
int isVolumeLabel(char *data_entry);
int isSubdir(char * data_entry);

char *getFileName(char *data_entry, char *filename);
char *getDirName(char *data_entry, char *dir_name);

#endif