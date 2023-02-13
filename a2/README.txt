Jason Kepler
csc360 - Assignment 2
Airline Check-in System (ACS)


To run ACS:
1)  Run the command: make
    - Compiles ACS by running the provided Makefile

2)  Then the command: ./ACS [filename]
    - Starts the ACS program, simulates checkin and prints stats

About:
Implemented of a 2 tier system involving business class and economy class queues. However, queueing system
is implemented using arrays and loops so that it can easily be converted to a multi tier system. For instant
a system with 5 different classes of customers, where the higher the class id the higher the priority.