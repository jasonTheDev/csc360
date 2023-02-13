# A FAT12 Disk Management Program
diskinfo, disklist, diskget and diskput

## To run
**Compile using the Makefile**
````bash
make
````
This will compile all four of the Disk Management programs.
## Usage
1. **Run Diskinfo from the shell**
    ````bash
    ./diskinfo DISKPATH
    ````
    Prints out information about the given disk.

    Example: `./diskinfo test/disk1.IMA`

2. **Run Disklist from the shell**
    ````bash
    ./disklist DISKPATH
    ````
    List files and directories of the given disk.

    Example: `./disklist test/disk1.IMA`

3. **Run Diskget from the shell**
    ````bash
    ./diskget DISKPATH FILE
    ````
    Copies the file from disks root directory into the working directory, if file exists.

    Example: `./diskget test/disk1.IMA REMINDER.TXT`

4. **Run Diskput from the shell**
    ````bash
    ./diskput DISKPATH LOCAL/PATH/TO/FILE
    ````
    Copies the file of same name in working directory to the path given in disk.

    Example: `./diskput test/disk2.IMA SUB8/README.md`
