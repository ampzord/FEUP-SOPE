
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h> /* errno */

#include "type.h"

extern int errno;

void printUsageMessage() {
    printf("\nWrong number of arguments!\n");
    printf("Usage: gerador <number of orders> <max usage time> <time unit system>\n");
    printf("Number of orders : is the total number of orders generated throughout the execution of the program. If that number is reached the program stops.\n");
    printf("Max usage time : is the maximum time a user can stay inside a sauna.\n");
    printf("Time unit system : is the time unit of the given time. 's' - second, 'm' - millisecond, 'u' - microsecond.\n");
}

/* link : http://stackoverflow.com/questions/2784500/how-to-send-a-simple-string-between-two-programs-using-pipes */


char* createOrderFifo() {
    char* myfifo = "/tmp/entrada";

    //to change later, depending on what this fifo will do
    mkfifo(myfifo, 0222);

    return myfifo;
}

char* receiveRejectedFifo() {
    char* rejectedFifo = "tmp/rejeitados";
    return rejectedFifo;
}

int main(int argc, char *argv[]) {

    if (argc != 4) {
        printUsageMessage();
        exit(1);
    }

    int fd_order_fifo, fd_rejected_fifo;
    struct Order ord;

    char *orderFifo = createOrderFifo();
    char *rejectedFifo = receiveRejectedFifo();

    fd_order_fifo = open(orderFifo, O_WRONLY);

    /* unlink myfifo */

    return 0;
}
