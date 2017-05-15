
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


int fd_order_fifo;
int rej_fifo_fd;

int generated_orders_M;
int generated_orders_F;
int rejected_received_M;
int rejected_received_F;
int rejected_discarded_M;
int rejected_discarded_F;

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
    fd_order_fifo = open(ORDER_FIFO, O_WRONLY);

    /* Send print constraints to Sauna */
    write(fd_order_fifo, &maxIdDigits,sizeof(maxIdDigits));
    write(fd_order_fifo, &maxUsageDigits,sizeof(maxUsageDigits));

    for (size_t i = 0; i < max_number_orders; i++) {

        /* Generate random orders */
        Order* ord = malloc(sizeof(Order));

        if (rand() % 2 == 0) {
            ord->gender = 'M';
            generated_orders_M++;
        } else {
            ord->gender = 'F';
            generated_orders_F++;
        }

        ord->time_spent = rand() % max_usage_time + 1;
        ord->serial_number = ++total_orders;
        
        /* Write messages to the Order FIFO */
        write(fd_order_fifo, ord, sizeof(Order));

        /* Write messages to register */

        /* Get Elapsed time */
        double delta_time = (getCurrentTime() - start_time) / 1000;

        fprintf(fp_register, "%.2f - ", delta_time);
        fprintf(fp_register, "%ld - ", gettid());
        fprintf(fp_register, "%*d: ", maxIdDigits, ord->serial_number);
        fprintf(fp_register, "%c ", ord->gender);
        fprintf(fp_register, "%*d ", maxUsageDigits, ord->time_spent);
        fprintf(fp_register, "PEDIDO\n");
        
        /* Cleanup order */
        free(ord);
        ord = NULL;
    }
    return NULL;
}


void processRejectedOrder(Order* ord) {

    /* Get Elapsed time */
    double delta_time = (getCurrentTime() - start_time) / 1000;

    int maxIdDigits = findn(max_number_orders);
    int maxUsageDigits = findn(max_usage_time);

    fprintf(fp_register, "%.2f - ", delta_time);
    fprintf(fp_register, "%ld - ", gettid());
    fprintf(fp_register, "%*d: ", maxIdDigits, ord->serial_number);
    fprintf(fp_register, "%c ", ord->gender);
    fprintf(fp_register, "%*d ", maxUsageDigits, ord->time_spent);
    fprintf(fp_register, "REJEITADO\n");

    if (ord->gender == 'M') {
        rejected_received_M++;
    }
    else if (ord->gender == 'F') {
        rejected_received_F++;
    }

    if (ord->rejected < 3) {
        write(fd_order_fifo, ord, sizeof(Order));
    }
        /* ORDERS TO BE DISCARDED */
    else {
        /* Get Elapsed time */
        double delta_time = (getCurrentTime() - start_time) / 1000;

        fprintf(fp_register, "%.2f - ", delta_time);
        fprintf(fp_register, "%ld - ", gettid());
        fprintf(fp_register, "%*d: ", maxIdDigits, ord->serial_number);
        fprintf(fp_register, "%c ", ord->gender);
        fprintf(fp_register, "%*d ", maxUsageDigits, ord->time_spent);
        fprintf(fp_register, "DESCARTADO\n");

        if (ord->gender == 'M') {
            rejected_discarded_M++;
        }
        else if (ord->gender == 'F') {
            rejected_discarded_F++;
        }
    }
    sleep(1);
}


void* rejectedThread()
{
    Order* ord = malloc(sizeof(Order));

    while(readOrder(rej_fifo_fd, ord)) {
        processRejectedOrder(ord);
        //usleep(500*1000);
        //sleep(1);
    }

    //sleep(1);
    usleep(500*1000);
    free(ord);
    return NULL;
}

pthread_t generateOrders() {
    pthread_t pth;
    pthread_create(&pth,NULL,threadOrders,NULL);
    return pth;
}

pthread_t receiveRejected() {
    pthread_t pth;
    pthread_create(&pth,NULL,rejectedThread,NULL);
    return pth;
}


void statsGeneratedGerador() {
    printf("\n-------- FINAL STATS GENERATED FOR GERADOR -----------\n");
    printf("TOTAL ORDERS GENERATED : %d, MALE : %d, FEMALE : %d\n", generated_orders_F+generated_orders_M, generated_orders_M, generated_orders_F);
    printf("TOTAL ORDERS REJECTED  : %d, MALE : %d, FEMALE : %d\n", rejected_received_F+rejected_received_M, rejected_received_M, rejected_received_F);
    printf("TOTAL ORDERS DISCARDED : %d, MALE : %d, FEMALE : %d\n", rejected_discarded_F+rejected_discarded_M, rejected_discarded_M, rejected_discarded_F);
    printf("------------------------------------------------------\n");
}

int main(int argc, char *argv[]) {

    /* Validates arguments */
    if (argc != 3) {
        printUsageMessage();
        exit(1);
    }

    generated_orders_M = 0;
    generated_orders_F = 0;
    rejected_received_M = 0;
    rejected_received_F = 0;
    rejected_discarded_M = 0;
    rejected_discarded_F = 0;

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

    do
    {
        printf("Opening Rejected FIFO...\n");
        rej_fifo_fd=open(REJECTED_FIFO ,O_RDONLY);
        if (rej_fifo_fd == -1) sleep(1);
    } while (rej_fifo_fd == -1);
    printf("REJECTED FIFO OPENED\n");

    pthread_t rej_pth = receiveRejected();

    pthread_join(gen_pth, NULL);
    pthread_join(rej_pth, NULL);

    statsGeneratedGerador();

    /* unlink fifos */
    unlink(orderFIFO);
    fclose(fp_register);
    close(rej_fifo_fd);
    close(fd_order_fifo);
    pthread_exit(NULL);

    return 0;
}
