# Process Manager (PMan)
MPan is a process manager written in C. It demonstrate the use of `fork()`, `exec()` and other system calls used to manage multi-process programs.

## To run PMan:
1. **Compile PMan using the Makefile**
    
    ````bash
    make
    ````
    
2. **Run PMan from the shell**
    
    ````bash
    ./pman
    ````
    You should see the following prompt appear `PMan >`.

## Usage:
1. **Start a background process**
    
    ````bash
    bg ./[program] [options]
    ````
    Runs the given program in the background and adds it to list of managed processes.
    
    Example: `PMan > bg ls -l`
    
2. **List the processes currently running in PMan**
    
    ````bash
    bglist
    ````
    Prints the process id and path of all managed programs.
    
3. **Kill a given process**
    
    ````bash
    bgkill [pid]
    ````
    Terminates the process with give pid, if it is a managed process.
    
    Example: `PMan > bgkill 2900`
    
4. **Pause a given process**
    
    ````bash
    bgstop [pid]
    ````
    Pauses the process with pid, if it is a managed process.
    
    Example: `PMan > bgstop 2902`
    
5. **Start a paused process**
    
    ````bash
    bgstart [pid]
    ````
    Re-starts a paused process, if it is a managed process.
    
    Example: `PMan > bgstart 2902`
    
6. **Print information about a given process**
    
    ````bash
    pstat [pid]
    ````
    Prints PCB information about given process, if it is a managed process.
    comm, state, utime, stime, rss, voluntary_ctxt_switches and nonvoluntary_ctxt_switches
    
    Example: `PMan > pstat 2902`
    
7. **Exit PMan**
    
    ````bash
    q
    ````
    Terminates all managed processes and exits PMan.
