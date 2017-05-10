

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h> /* errno */

#include "type.h"

extern int errno;
unsigned int number_seats;

void printUsageMessage() {
    printf("\nWrong number of arguments!\n");
    printf("Usage: sauna <number of seats>\n");
    printf("Number of seats : is the total number of orders generated throughout the execution of the program. If that number is reached the program stops.\n\n");
}

char* createRejectedFifo() {
    char* rejFifo = "/tmp/rejeitados";
    //to change later, depending on what this fifo will do
    mkfifo(rejFifo, 0222);
    return rejFifo;
}

char* receiveOrderFifo() {
    char* orderFifo = "tmp/entrada";
    return orderFifo;
}

int main(int argc, char *argv[]) {

    if (argc != 2) {
        printUsageMessage();
        exit(1);
    }

    /* Parse command-line arguments to global variables */
    number_seats = atoi(argv[1]);

    char* orderFifo = receiveOrderFifo();
    char* rejectedFifo = createRejectedFifo();

    return 0;
}