

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h> /* errno */
#include <fcntl.h> /* O_RDONLY O_WRONLY */
#include <pthread.h>
#include <semaphore.h>

#include "type.h"
#include "consts.h"

extern int errno;
unsigned int number_seats;
unsigned int occupied_seats;

void printUsageMessage() {
    printf("\nWrong number of arguments!\n");
    printf("Usage: sauna <number of seats>\n");
    printf("Number of seats : is the total number of orders generated throughout the execution of the program. If that number is reached the program stops.\n\n");
}

int readOrder(int fd, Order *ord) {
    int n;
    n = read(fd,ord,sizeof(Order));
    return n;
}

int addThread(pthread_t threads[], size_t size, pthread_t thread) {
    for (int i = 0; i < size; i++) {
        if (threads[i] == NULL) {
            threads[i] = thread;
            return 1;
        }
    }
    return 0;
}

int removeThread(pthread_t threads[], size_t size, pthread_t thread) {
       for (int i = 0; i < size; i++) {
        if (threads[i] == thread) {
            threads[i] = NULL;
            return 1;
        }
    }
    return 0;
}

void runOrder(void *arg) {
    unsigned int time_ms = *((int *) arg);
    while(time_ms--) usleep(1000);
    printf("THREAD ENDED:%d\n", time_ms);
    // TODO: Notify thread ended
    free(arg);
}

pthread_t acceptOrder(Order *ord) {
    pthread_t pth;
    unsigned int *time_ms = malloc(sizeof(time_ms));
    *time_ms = ord->time_spent;
    //printf("PREThread:%d\n", *time_ms);
    pthread_create(&pth,NULL,runOrder,time_ms);
    return pth;
}

void rejectOrder(Order *ord) {
    ord->rejected++;
    // TODO: Send rejected order bacl
}

void processOrder(pthread_t threads[], size_t size, Order *ord) {
    if (occupied_seats < number_seats) {
        occupied_seats++;
        pthread_t pth = acceptOrder(ord);
        if(addThread(threads, size, pth)) {
            printf("Added\n");
        } else {
            printf("Full\n");
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printUsageMessage();
        exit(1);
    }

    /* Parse command-line arguments to global variables */
    number_seats = atoi(argv[1]);
    occupied_seats = 0;

    /* Create Rejected FIFO */
    mkfifo(REJECTED_FIFO, 0660);
   
    /* Array to hold all threads created */ 
    pthread_t seats_threads[number_seats];
    for (int i = 0; i < number_seats; i++) seats_threads[i] = NULL;
        
    int fd;
    do
    {
        printf("Opening FIFO...\n");
        fd=open(ORDER_FIFO ,O_RDONLY);
        if (fd==-1) sleep(1);
    } while (fd==-1);
    printf("FIFO found\n");
    
    Order* ord = malloc(sizeof(Order));
    while(readOrder(fd, ord)) {
        processOrder(seats_threads, number_seats, ord);
    }
    
    /* Waits for all the threads */
    for (int i = 0; i < number_seats; i++)
        if (seats_threads[i] != NULL)
            pthread_join(seats_threads[i], NULL);
     
    unlink(REJECTED_FIFO);
    return 0;
}