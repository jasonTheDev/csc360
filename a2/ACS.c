/* 
    Jason Kepler - V00848837
    Airline Check-in System
    CSC 360
*/

#include <pthread.h>    //threads
#include <stdio.h>      //fopen()
#include <string.h>     //strtok()
#include <stdlib.h>     //atoi()
#include <ctype.h>      //isdigit()
#include <unistd.h>     //usleep()
#include <sys/time.h>   //gettimeofday()
#include <time.h>       //difftime()

#include "safe_pthread.h"
#include "acs_helper.h"

/*------------ Definitions ------------*/
#define NUM_QUEUES              2
#define NUM_CLASSES             NUM_QUEUES
#define NUM_CLERKS              5
#define MAX_QUEUE_LENGTH        50

// file constants
#define MAX_LINE_LENGTH         256
#define NUM_CUSTOMER_ATTRS      4
#define FILE_FORMAT_ERROR       "ERROR: file format incorrect\n"

// conversion
#define DECI_TO_MICRO           100000

// other
#define BUSINESS_CLASS          1
#define ECONOMY_CLASS           0
#define TRUE                    1
#define FALSE                   0
#define QUEUE_READY             -1
#define IDLE                    -1
#define SERVING                 0


/*------------ Structs and Global Variables ------------*/

struct customer_info{
    int user_id;
	int class_type;
    int arrival_time;
	int service_time;
};

int num_customers;

//clerks
int clerk_id[NUM_CLERKS];
int clerk_status[NUM_CLERKS];
pthread_t clerk_tid[NUM_CLERKS];

// queues
struct customer_info *queue[NUM_QUEUES][MAX_QUEUE_LENGTH];
int tail_index[NUM_QUEUES];
int head_index[NUM_QUEUES];
int queue_length[NUM_QUEUES];
int queue_status[NUM_QUEUES];
int dequeued_id[NUM_QUEUES];

// mutex
pthread_mutex_t queue_mutex[NUM_QUEUES];
pthread_mutex_t clerk_mutex[NUM_CLERKS];
pthread_mutex_t wait_mutex[NUM_CLASSES];

// convar
pthread_cond_t queue_condv[NUM_QUEUES];
pthread_cond_t clerk_condv[NUM_CLERKS];

// stats
struct timeval init_time;
double waiting_time[NUM_CLASSES];
int num_cust_in_class[NUM_CLASSES];


/*------------ Initialization Functions ------------*/

void init_queues() {
    for (int i = 0; i < NUM_QUEUES; i++) {

        tail_index[i] = 0;
        head_index[i] = 0;
        queue_length[i] = 0;
        queue_status[i] = QUEUE_READY;
        dequeued_id[i] = 0;

        for (int j = 0; j < MAX_QUEUE_LENGTH; j++) {
            queue[i][j] = NULL;
        }

        safe_mutex_init(&queue_mutex[i], NULL);
        safe_cond_init(&queue_condv[i], NULL);
    }
}

void init_clerks() {
    for (int i = 0; i < NUM_CLERKS; i++) {

        clerk_status[i] = IDLE;

        safe_mutex_init(&clerk_mutex[i], NULL);
        safe_cond_init(&clerk_condv[i], NULL);
    }
}

void init_stats() {
    for (int i = 0; i < NUM_CLASSES; i++) {
        num_cust_in_class[i] = 0;
        safe_mutex_init(&wait_mutex[i], NULL);
    }
}

/*
 * Performs all initiation needed for ACS
 */
void init_all() {
    init_clerks();
    init_queues();
    init_stats();
}


/*------------ File reading and format checking ------------*/

/*
 * Prints usage for ACS
 */
void printUsage() {
    printf("Usage: ./ASC [input filename]\n");
}

/*
 * Parses the given file and addes customer information to the customer_array
 * Returns: 1 for success
 */
int getCustomerInfo(FILE *fptr, struct customer_info customer_array[])
{
    char *user_id_ptr;
    char *class_type_ptr;
    char *arrival_time_ptr;
    char *service_time_ptr;

    char line_buffer[MAX_LINE_LENGTH];

    // go to second line
    fgets(line_buffer, MAX_LINE_LENGTH, fptr);

    int line_count = 0;
    while ( fgets(line_buffer, MAX_LINE_LENGTH, fptr) != NULL ) {

        if (( user_id_ptr = strtok(line_buffer, ":") ) == NULL 
            ||  ( class_type_ptr = strtok(NULL, ",") ) == NULL
            ||  ( arrival_time_ptr = strtok(NULL, ",") ) == NULL
            ||  ( service_time_ptr = strtok(NULL, "\n\r\t") ) == NULL)
        {
            return -1;
        }

        if ( isDigits(user_id_ptr)
            && isDigits(class_type_ptr)
            && isDigits(arrival_time_ptr)
            && isDigits(service_time_ptr)
            && (atoi(class_type_ptr) == BUSINESS_CLASS
                || atoi(class_type_ptr) == ECONOMY_CLASS)) {

            customer_array[line_count].user_id = atoi(user_id_ptr);
            customer_array[line_count].class_type = atoi(class_type_ptr);
            customer_array[line_count].arrival_time = atoi(arrival_time_ptr);
            customer_array[line_count].service_time = atoi(service_time_ptr);

            ++num_cust_in_class[customer_array[line_count].class_type];

            line_count++;
        }
        else {
            return -1;
        }
    }

    return line_count == num_customers;
}


/*------------ Queue Operations and Queue Synchronization ------------*/

void lock_all_queues() {
    for (int qid = 0; qid < NUM_QUEUES; qid++) {
        safe_mutex_lock(&queue_mutex[qid]);
    }
}


void unlock_all_queues() {
    for (int qid = NUM_QUEUES-1; qid >= 0; qid--) {
        safe_mutex_unlock(&queue_mutex[qid]);
    }
}

/*
 * Adds customer_info pointer to queue of given queue id (qid).
 * Increments the length of the queue.
 */
void enQueue(struct customer_info *customer_ptr, int qid) {
    int tail = tail_index[qid];
    queue[qid][tail] = customer_ptr;
    tail_index[qid] = (tail + 1) % MAX_QUEUE_LENGTH;
    ++queue_length[qid];
}

/*
 * Iterates through queues in order of prioity, the higher the queue id (qid)
 * the higher the priority, and deQueues from the first queue that is not empty.
 * Decrements the length of queue, sets dequeued_id of qid to the customers id,
 * and sets the queues status to equal the calling clerks id
 * Returns: the qid if customers was deQueued, otherwise -1
 */
int deQueue(int clerk_id)
{
    for (int qid = NUM_QUEUES-1; qid >= 0; qid--) {
        if (queue_status[qid] == QUEUE_READY && queue[qid][head_index[qid]] != NULL) {
            int head = head_index[qid];
            struct customer_info *dequeued = queue[qid][head];

            head_index[qid] = (head + 1) % MAX_QUEUE_LENGTH;
            --queue_length[qid];

            dequeued_id[qid] = dequeued -> user_id;
            queue_status[qid] = clerk_id;

            return qid;
        }
    }

    return -1;
}


/*------------ Statistics Collection and Output ------------*/

/*
 * Gets the relative time passed since start of simulation
 * Returns: time in seconds
 */
double rel_seconds(struct timeval *end) {
    return diff_seconds(end, &init_time);
}

/*
 * Gets the average waiting time for the given class
 * Returns: average waiting time in seconds
 */
double avg_class_wait(int class_id) {
    double avg_wait = 0.0;
    if (num_cust_in_class[class_id] > 0) {
        avg_wait = waiting_time[class_id] / num_cust_in_class[class_id];
    }
    return avg_wait;
}

/*
 * Gets the average waiting time for all classes
 * Returns: average waiting time in seconds
 */
double avg_total_wait() {
    double total_wait = 0;
    for (int i = 0; i < NUM_CLASSES; i++) {
        total_wait = total_wait + waiting_time[i];
    }
    return total_wait / num_customers;
}

/*
 * Prints divider for program output
 */
void printDivider() {
    printf("------------------------------------------------------------------------------\n");
}


/*------------------------ Threads  ------------------------*/

/*
 * Customer thread. Simulates a customer in an Airline Checkin System
 */
void *customer(void * param)
{   
    int clerk_id, qid, class, customer_id;
    struct timeval arrival_time, enter_queue_time, start_service_time, end_service_time;

    struct customer_info * customer = (struct customer_info *) param;
    qid = class = customer -> class_type;
    customer_id = customer -> user_id;

    usleep((customer -> arrival_time) * DECI_TO_MICRO);

    // Arrival
    safe_gettimeofday(&arrival_time);
    printf("%.2f\tArrives: Customer ID %d arrives.\n", rel_seconds(&arrival_time), customer_id);

    // Enter queue
    safe_mutex_lock(&queue_mutex[qid]);
    enQueue(customer, qid);
    safe_gettimeofday(&enter_queue_time);
    printf("%.2f\tEnters: Customer ID %d enters queue ID %d, length %d.\n", rel_seconds(&enter_queue_time), customer_id, qid, queue_length[qid]);

    // Wait to be served
    while (dequeued_id[qid] != customer_id) {
        safe_cond_wait(&queue_condv[qid], &queue_mutex[qid]);
    }
    clerk_id = queue_status[qid];
    queue_status[qid] = QUEUE_READY;

    // Service starts
    safe_gettimeofday(&start_service_time);
    printf("%.2f\tStarts: Customer ID %d starts being served by clerk ID %d.\n", rel_seconds(&start_service_time), customer_id, clerk_id);
    safe_mutex_unlock(&queue_mutex[qid]);

    usleep((customer -> service_time) * DECI_TO_MICRO);

    // Finish service
    safe_gettimeofday(&end_service_time);
    printf("%.2f\tFinishes: Customer ID %d finishes being served by clerk ID %d.\n", rel_seconds(&end_service_time), customer_id, clerk_id);

    safe_mutex_lock(&clerk_mutex[clerk_id]);
    clerk_status[clerk_id] = IDLE;
    safe_cond_signal(&clerk_condv[clerk_id]);   // Signal clerk thread that service is complete
    safe_mutex_unlock(&clerk_mutex[clerk_id]);

    // Collect stats
    safe_mutex_lock(&wait_mutex[class]);
    waiting_time[class] += diff_seconds(&start_service_time, &enter_queue_time);
    safe_mutex_unlock(&wait_mutex[class]);

    pthread_exit(NULL);
}


/*
 * Clerk thread. Simulates a clerk in an Airline Checkin System
 */
void *clerk(void *param)
{   
    int qid;
    int * clerk_id_ptr = (int *) param;
    int clerk_id = *clerk_id_ptr;

    while (TRUE) {
        lock_all_queues();
        if ((qid = deQueue(clerk_id)) != -1) {

            // Customer deQueued
            clerk_status[clerk_id] = SERVING;
            safe_cond_broadcast(&queue_condv[qid]);
            unlock_all_queues();

            // Wait for service to finish
            safe_mutex_lock(&clerk_mutex[clerk_id]);
            while (clerk_status[clerk_id] == SERVING) {
                safe_cond_wait(&clerk_condv[clerk_id], &clerk_mutex[clerk_id]);
            }
            safe_mutex_unlock(&clerk_mutex[clerk_id]);
        }
        else {
            unlock_all_queues();
        }
    }
}


/*
 * ACS
 */
int main(int argc, char *argv[]) {

    FILE *fptr;
    pthread_attr_t attr;

    init_all();
    pthread_attr_init(&attr);
    

    /* Error handling for program use */
    if (argc != 2) {
        printf("ERROR: incorrect number of arguments\n");
        printUsage();
        return -1;
    }

    fptr = fopen(argv[1], "r");
    if ( fptr == NULL ) {
        printf("ERROR: unable to open file %s\n", argv[1]);
        printUsage();
        return -1;
    }

    /* Error handling for input file format*/
    if ( fscanf(fptr, "%d", &num_customers) != 1 ) {
        printf(FILE_FORMAT_ERROR);
        return -1;
    }

    struct customer_info customer_array[num_customers];

    if ( getCustomerInfo(fptr, customer_array) != 1 ) {
        fclose(fptr);
        printf(FILE_FORMAT_ERROR);
        return -1;
    }
    fclose(fptr);


    /* Read file successfully. Start simulation */
    pthread_t customer_tid[num_customers];

    for (int i = 0; i < NUM_CLERKS; i++) {
        clerk_id[i] = i;
        safe_pthread_create(&clerk_tid[i], &attr, clerk, (void *) &clerk_id[i]);
    }

    safe_gettimeofday(&init_time);
    printf("TIME\t\tDESCRIPTION\n");
    printDivider();
    

    for (int i = 0; i < num_customers; i++) {
        safe_pthread_create(&customer_tid[i], &attr, customer, (void *) &customer_array[i]);
    }

    for (int i = 0; i < num_customers; i++) {
        safe_pthread_join(customer_tid[i], NULL);
    }
    
    /* Print Statistics for the System */
    printf("\nAirline Check-in System Statistics\n");
    printDivider();
    printf("The average waiting time for all customers in the system is: %.2f seconds. \n", avg_total_wait());
    printf("The average waiting time for all business-class customers is: %.2f seconds.\n", avg_class_wait(BUSINESS_CLASS));
    printf("The average waiting time for all economy-class customers is: %.2f seconds.\n", avg_class_wait(ECONOMY_CLASS));

    pthread_attr_destroy(&attr);

    return 0;
}