
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
#include <sys/syscall.h>
#include <sys/time.h>

#define gettid() syscall(SYS_gettid)

#include "type.h"

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

char* createOrderFifo() {
    char* ordFifo = "/tmp/entrada";
    //to change later, depending on what this fifo will do
    mkfifo(ordFifo, 0660);
    return ordFifo;
}

char* receiveRejectedFifo() {
    char* rejectedFifo = "/tmp/rejeitados";
    return rejectedFifo;
}

//Get number of digits of number
int findn(int num)
{
    char snum[100];
    sprintf(snum, "%d", num);
    return strlen(snum);
}

void* threadOrders(void* arg)
{
    int fd_order_fifo;
    char* orderFifo = (char*) arg;
    fd_order_fifo = open(orderFifo, O_WRONLY);
    
    int maxIdDigits = findn(max_number_orders);
    int maxUsageDigits = findn(max_usage_time);

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

        /* Write struct to order fifo */

        //write(fd_order_fifo, &ord, sizeof(ord));

        /* Write messages to register */

        pthread_t id = pthread_self();

        struct timeval tv;
        gettimeofday(&tv, NULL);
        double end_time = (tv.tv_sec) * 1000000 + (tv.tv_usec);
        double delta_time = (end_time - start_time) / 1000;

        fprintf(fp_register, "%.2f - ", delta_time);
        fprintf(fp_register, "%d - ", gettid());
        fprintf(fp_register, "%*d: ", maxIdDigits, ord->serial_number);
        fprintf(fp_register, "%c ", ord->gender);
        fprintf(fp_register, "%*d ", maxUsageDigits, ord->time_spent);
        fprintf(fp_register, "PEDIDO\n");
    }

    close(fd_order_fifo);
}

pthread_t generateOrders(char* orderFifo) {
    pthread_t pth;
    pthread_create(&pth,NULL,threadOrders,&orderFifo);
    return pth;
}

int main(int argc, char *argv[]) {

    if (argc != 3) {
        printUsageMessage();
        exit(1);
    }

    struct timeval tv;
    gettimeofday(&tv, NULL);
    start_time = (tv.tv_sec) * 1000000 + (tv.tv_usec);

    /* Parse command-line arguments to global variables */
    max_number_orders = atoi(argv[1]);
    max_usage_time = atoi(argv[2]);

    char *orderFifo = createOrderFifo();
    char *rejectedFifo = receiveRejectedFifo();
    
    /* TESTING CODE */
    sleep(3);
    printf("Writing FIFO\nw");
    int fd_order_fifo = open(orderFifo, O_WRONLY);

    Order* ord = malloc(sizeof(Order));
    ord->gender = 'M';
    ord->time_spent = 16;
    ord->serial_number = 14;
    ord->rejected = 12;
    write(fd_order_fifo, ord, sizeof(Order));
    //write(fd_order_fifo, "Hello\0", 6);
    close(fd_order_fifo);
    sleep(2);
    /* END OF TESTING CODE */
    
    char path_reg[16];
    sprintf(path_reg, "/tmp/ger.%d\n", getpid());

    fp_register = fopen(path_reg, "w");

    pthread_t gen_pth = generateOrders(orderFifo);
    //receiveRejected();

    pthread_join(gen_pth, NULL);
    //pthread_join(receive)

    /* unlink fifos */
    unlink(orderFifo);

    fclose(fp_register);

    return 0;
}
