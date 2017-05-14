#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h> /* errno */
#include <fcntl.h> /* O_RDONLY O_WRONLY */
#include <pthread.h>

#include "type.h"
#include "consts.h"

typedef struct {
    pthread_t pth;
    int idx;
} SeatThread;

typedef struct {
    int idx;
    unsigned int time_ms;
} ThreadArg;

extern int errno;
unsigned int number_seats;
pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER; // Mutex to update seat values
pthread_mutex_t mut_add = PTHREAD_MUTEX_INITIALIZER; // Mutex to assign seat
SeatThread *seats_threads = NULL; // Array to hold all the seat threads
char curr_gender; // Char to hold the current type


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

int getEmptySeat() {
    pthread_mutex_lock(&mut);
    for (int i = 0; i < number_seats; i++) {
        if (seats_threads[i].idx < 0) {
            pthread_mutex_unlock(&mut);
            return i;
        }
    }
    pthread_mutex_unlock(&mut);
    return -1;
}

int getEmptySeats() {
    int emptySeats = 0;
    
    pthread_mutex_lock(&mut);
    for (int i = 0; i < number_seats; i++)
        if (seats_threads[i].idx < 0)
            emptySeats++;
    pthread_mutex_unlock(&mut);
     
    return emptySeats;
}

int isEmpty() {
    if (getEmptySeats() == number_seats)
        return 1;
    else
        return 0;
}

void setSeatThread(int idx, SeatThread thread) {
    pthread_mutex_lock(&mut);
    seats_threads[idx] = thread;
    pthread_mutex_unlock(&mut);
}

void removeSeatThread(int idx) {
    pthread_mutex_lock(&mut);
    seats_threads[idx].idx = -1;
    pthread_mutex_unlock(&mut);
}

void runOrder(void *arg) {
    /* Parse arguments */
    ThreadArg targ = *((ThreadArg *) arg);
    int idx = targ.idx;
    unsigned int time_ms = targ.time_ms;
    
    /* Waits for the given ammount of time */
    while(time_ms-- >0) usleep(1000);
    printf("Thread %d ended\n", idx);
    
    /* Thread ended */
    removeSeatThread(idx);
    free(arg);
}

pthread_t acceptOrder(Order *ord, int idx) {
    pthread_t pth;
    unsigned int *time_ms = malloc(sizeof(time_ms));
    ThreadArg *targ = malloc(sizeof(ThreadArg));
    targ->idx = idx;
    targ->time_ms = ord->time_spent;
    
    printf("Starting thread %d during %d\n", idx, targ->time_ms);
    pthread_create(&pth,NULL,runOrder,targ);
    return pth;
}

void rejectOrder(Order *ord) {
    ord->rejected++;
    printf("Rejected because current gender is %c and requested %c\n", curr_gender, ord->gender);
    // TODO: Send rejected order bacl
}

void processOrder(Order *ord) {
    /* Wait for empty seats */
    while (getEmptySeats() == 0) usleep(500*1000);
    
    if (isEmpty()) {
        curr_gender = ord->gender;
    } else if (curr_gender != ord->gender) {
        rejectOrder(ord);
        return;
    }
    /* Assigns the order to a empty seat */
    int idx = getEmptySeat();
    pthread_t pth = acceptOrder(ord, idx);
    SeatThread st;
    st.idx = idx;
    st.pth = pth;
    pthread_mutex_unlock(&mut);
    setSeatThread(idx, st);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printUsageMessage();
        exit(1);
    }

    /* Parse command-line arguments to global variables */
    number_seats = atoi(argv[1]);

    /* Array to hold all threads created */ 
    seats_threads = malloc(number_seats * sizeof(SeatThread));
    for (int i = 0; i < number_seats; i++) seats_threads[i].idx = -1;
    
    /* Create Rejected FIFO */
    mkfifo(REJECTED_FIFO, 0660);
    
    /* Frees mutex from possible previous lock */
    pthread_mutex_unlock(&mut);
    
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
        processOrder(ord);
    }
    
    for (int i = 0; i < number_seats; i++)
        if (seats_threads[i].idx >= 0) {
            printf("Waiting on thread: %d\n",i);
            pthread_join(seats_threads[i].pth, NULL);
        }
    
    /* Cleanup */
    unlink(REJECTED_FIFO);
    free(seats_threads);
    free(ord);
    return 0;
}