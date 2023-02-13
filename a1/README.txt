Jason Kepler
csc360 - Assignment 1
Process Manager (PMan)


To run PMan:
1)  Run the command: make
    - Compiles PMan by running the Makefile
2)  Then the command: ./pman
    - Starts the PMan program, you should see the PMan prompt appear

Useage:
1)  bg ./[program] [options]
    - Runs the given program in the background and adds it to PMans list of managed processes
    - examples:
        PMan > bg ./test
        PMan > bg ls -l
2)  bglist
    - Prints the process id and path of all managed programs
3)  bgkill [pid]
    - terminates the process with give pid, if it is a managed process
    - example:
        PMan > bgkill 2900
4)  bgstop [pid]
    - Pauses the process with pid, if it is a managed process
    - example: 
        PMan > bgstop 2902
5)  bgstart [pid]
    - Re-starts a paused process, if it is a managed process
    - example: 
        PMan > bgstart 2902
6)  pstat [pid]
    - Prints PCB information about given process, if it is a managed process
    - comm, state, utime, stime, rss, voluntary_ctxt_switches and nonvoluntary_ctxt_switches
    - example:
        PMan > pstat 2902
7)  q
    - Terminates all managed processes and exits PMan
