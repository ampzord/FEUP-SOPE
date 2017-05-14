
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h> /* errno */
#include <pthread.h>
#include <time.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/file.h>
#include <stdint.h> /*uint */

#include <sys/time.h>

#include "type.h"
#include "consts.h"

extern int errno;
unsigned int total_orders = 0;
unsigned int max_number_orders;
unsigned int max_usage_time;
FILE* fp_register;
double start_time;

void printUsageMessage() {
    printf("\nWrong number of arguments!\n");
    printf("Usage: gerador <number of orders> <max usage time>\n");
    printf("Number of orders : is the total number of orders generated throughout the execution of the program. If that number is reached the program stops.\n");
    printf("Max usage time : is the maximum time a user can stay inside a sauna.\n\n");
}

void* threadOrders()
{   
    int maxIdDigits = findn(max_number_orders);
    int maxUsageDigits = findn(max_usage_time);
    int fd_order_fifo = open(ORDER_FIFO, O_WRONLY);

    for (size_t i = 0; i < max_number_orders; i++) {

        /* Generate random orders */
        Order* ord = malloc(sizeof(Order));

        if (rand() % 2 == 0) {
            ord->gender = 'M';
        } else {
            ord->gender = 'F';
        }
        ord->time_spent = rand() % max_usage_time + 1;
        ord->serial_number = ++total_orders;
        
        /* Write messages to the Order FIFO */
        write(fd_order_fifo, ord, sizeof(Order));

        /* Write messages to register */
        //pthread_t id = pthread_self();

        gettimeofday(&tv, NULL);
        double end_time = (tv.tv_sec) * 1000000 + (tv.tv_usec);
        double delta_time = (end_time - start_time) / 1000;

        fprintf(fp_register, "%.2f - ", delta_time);
        //change from %d to %ld to remove warning print to long int
        fprintf(fp_register, "%ld - ", gettid());
        fprintf(fp_register, "%*d: ", maxIdDigits, ord->serial_number);
        fprintf(fp_register, "%c ", ord->gender);
        fprintf(fp_register, "%*d ", maxUsageDigits, ord->time_spent);
        fprintf(fp_register, "PEDIDO\n");
        
        /* Cleanup order */
        free(ord);
        ord = NULL;
        
        sleep(1);
    }
    close(fd_order_fifo);
}

pthread_t generateOrders() {
    pthread_t pth;
    pthread_create(&pth,NULL,threadOrders,NULL);
    return pth;
}

int main(int argc, char *argv[]) {

    /* Validates arguments */
    if (argc != 3) {
        printUsageMessage();
        exit(1);
    }

    /* Defines a differend random seed */
    int seed = time(NULL);
    srand(seed);
    
    /* Gets starting time of the program */
    gettimeofday(&tv, NULL);
    start_time = (tv.tv_sec) * 1000000 + (tv.tv_usec);

    /* Parse command-line arguments to global variables */
    max_number_orders = atoi(argv[1]);
    max_usage_time = atoi(argv[2]);

    /* Create Order FIFO */
    char* orderFIFO = ORDER_FIFO;
    mkfifo(orderFIFO, 0660);
    
    /* Create register file */
    char path_reg[16];
    sprintf(path_reg, "/tmp/ger.%d\n", getpid());
    fp_register = fopen(path_reg, "w");

    /* Generate orders */
    pthread_t gen_pth = generateOrders();
    //receiveRejected();

    pthread_join(gen_pth, NULL);

    /* unlink fifos */
    unlink(orderFIFO);

    /* Close register file */
    fclose(fp_register);

    return 0;
}
