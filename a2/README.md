# Airline Check-in System (ACS)
ACS is an airline check-in simulator written in C. It uses multithreading to simulate the customers and clerks. It demonstrates the use of the pthread library, mutex locks, and condition variables when writing a multi-threaded program.

## To run ACS:
1. **Compile ACS using the Makefile**
   
    ````bash
    make
    ````
    
2. **Run ACS from the shell**
    ````bash
    ./ACS [filename]
    ````
    Simulates the check-in sequence described in given file and prints stats.

    Example: `./ACS test/input1.txt`