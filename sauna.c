

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h> /* errno */
#include <fcntl.h> /* O_RDONLY O_WRONLY */

#include "type.h"


extern int errno;
unsigned int number_seats;

void printUsageMessage() {
    printf("\nWrong number of arguments!\n");
    printf("Usage: sauna <number of seats>\n");
    printf("Number of seats : is the total number of orders generated throughout the execution of the program. If that number is reached the program stops.\n\n");
}

int readline(int fd, char *str) {
    int n;
    do {
        n = read(fd,str,1);
    }
    while (n>0 && *str++ != '\0');
    return (n>0);
} 

int readOrder(int fd, struct Order *ord) {
    int n;

    n = read(fd,ord,sizeof(Order));
    printf("readorder: %d\n", n);
    sleep(1);
    return n;
} 

char* createRejectedFifo() {
    char* rejFifo = "/tmp/rejeitados";
    //to change later, depending on what this fifo will do
    mkfifo(rejFifo, 0660);
    return rejFifo;
}

char* receiveOrderFifo() {
    char* orderFifo = "/tmp/entrada";
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
    
    /* TESTING CODE */
    int fd, messagelen, i;
    char str[100];
    do
    {
        printf("Oppening FIFO...\n");
        fd=open(orderFifo,O_RDONLY | O_NONBLOCK);
        if (fd==-1) sleep(1);
    }
    while (fd==-1);
    printf("FIFO found\n");
    Order* ord = malloc(sizeof(Order));

    //while(!readOrder(fd, ord));
    while(1) {
        int n;
        n = read(fd,ord,sizeof(Order));
        printf("readorder: %d\n", n);

        printf("Order:\nSerialNo: %d\nGender: %c\nTime: %d\nRejected: %d\n", ord->serial_number, ord->gender, ord->time_spent, ord->rejected);
    }



    /* END OF TESTING CODE */
    
    unlink(rejectedFifo);
    return 0;
}