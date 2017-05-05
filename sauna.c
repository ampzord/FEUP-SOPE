

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h> /* errno */

#include "type.h"

extern int errno;

void printUsageMessage() {
    printf("\nWrong number of arguments!\n");
    printf("Usage: sauna <number of seats> <time unit system>\n");
    printf("Number of seats : is the total number of orders generated throughout the execution of the program. If that number is reached the program stops.\n\n");
}

int main(int argc, char *argv[]) {

    if (argc != 2) {
        printUsageMessage();
        exit(1);
    }

    return 0;
}