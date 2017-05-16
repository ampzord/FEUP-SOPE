
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
pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER; // Mutex to write register file
double start_time;

int fd_order_fifo;
int rej_fifo_fd;

int generated_orders_M = 0;
int generated_orders_F = 0;
int rejected_received_M = 0;
int rejected_received_F = 0;
int rejected_discarded_M = 0;
int rejected_discarded_F = 0;


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

    /* Send print constraints to Sauna */
    write(fd_order_fifo, &maxIdDigits,sizeof(maxIdDigits));
    write(fd_order_fifo, &maxUsageDigits,sizeof(maxUsageDigits));

    for (size_t i = 0; i < max_number_orders; i++) {
        printf("Generating and sending order\n");
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

        fprintf(fp_register, "%9.2f - %ld - %*d: %c - %*d - PEDIDO\n", delta_time, gettid(), 
                3, ord->serial_number, ord->gender, 6, ord->time_spent);
        /*fprintf(fp_register, "%.2f - ", delta_time);
        fprintf(fp_register, "%ld - ", gettid());
        fprintf(fp_register, "%*d: ", maxIdDigits, ord->serial_number);
        fprintf(fp_register, "%c ", ord->gender);
        fprintf(fp_register, "%*d ", maxUsageDigits, ord->time_spent);
        fprintf(fp_register, "PEDIDO\n");*/
        
        /* Cleanup order */
        free(ord);
        ord = NULL;
    }
    return NULL;
}


void processRejectedOrder(Order* ord) {    
    /* Get Elapsed time */
    printf("Received rejected order\n");
    double delta_time = (getCurrentTime() - start_time) / 1000;

    fprintf(fp_register, "%9.2f - %ld - %*d: %c - %*d - REJEITADO\n", delta_time, gettid(), 
            3, ord->serial_number, ord->gender, 6, ord->time_spent);
    /*fprintf(fp_register, "%.2f - ", delta_time);
    fprintf(fp_register, "%ld - ", gettid());
    fprintf(fp_register, "%*d: ", maxIdDigits, ord->serial_number);
    fprintf(fp_register, "%c ", ord->gender);
    fprintf(fp_register, "%*d ", maxUsageDigits, ord->time_spent);
    fprintf(fp_register, "REJEITADO\n");*/

    if (ord->gender == 'M') {
        rejected_received_M++;
    }
    else if (ord->gender == 'F') {
        rejected_received_F++;
    }

    if (ord->rejected < 3) {
        printf("Sending rejected order\n");
        write(fd_order_fifo, ord, sizeof(Order));
    }
        /* ORDERS TO BE DISCARDED */
    else {
        printf("Discarding rejected order\n");
        /* Get Elapsed time */
        double delta_time = (getCurrentTime() - start_time) / 1000;
        
        fprintf(fp_register, "%9.2f - %ld - %*d: %c - %*d - DESCARTADO\n", delta_time, gettid(), 
                3, ord->serial_number, ord->gender, 6, ord->time_spent);
        /*fprintf(fp_register, "%.2f - ", delta_time);
        fprintf(fp_register, "%ld - ", gettid());
        fprintf(fp_register, "%*d: ", maxIdDigits, ord->serial_number);
        fprintf(fp_register, "%c ", ord->gender);
        fprintf(fp_register, "%*d ", maxUsageDigits, ord->time_spent);
        fprintf(fp_register, "DESCARTADO\n");*/

        if (ord->gender == 'M') {
            rejected_discarded_M++;
        }
        else if (ord->gender == 'F') {
            rejected_discarded_F++;
        }
    }
}


void* rejectedThread()
{
    Order* ord = malloc(sizeof(Order));

    while(readOrder(rej_fifo_fd, ord)) {
        processRejectedOrder(ord);
    }
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

    /* Defines a differend random seed */
    int seed = time(NULL);
    srand(seed);

    /* Parse command-line arguments to global variables */
    max_number_orders = atoi(argv[1]);
   if (max_number_orders < 0 || max_number_orders > MAX_NUMBER_OF_ORDERS) {
        printf("Error on max number of orders, must be between 0 and 30000.\n");
        exit(1);
    }
    max_usage_time = atoi(argv[2]);
    if (max_usage_time < 0 || max_usage_time > MAX_USAGE_TIME) {
        printf("Error on max usage time, must be between 0 and 999999.\n");
        exit(1);
    }

    /* Create Order FIFO */
    printf("Creating Order FIFO\n");
    char* orderFIFO = ORDER_FIFO;
    //mkfifo(orderFIFO, 0660);

    if(mkfifo(ORDER_FIFO, S_IRUSR | S_IWUSR) != 0){
        if (errno != EEXIST) { //file already exists
            perror("Error while creating Order FIFO!\n");
            exit(-1);
        }
    }
    fd_order_fifo = open(ORDER_FIFO, O_WRONLY);
    
    /* Create Rejected FIFO */
    printf("Creating Rejected FIFO\n");
    char* rejectedFIFO = REJECTED_FIFO;
    //mkfifo(rejectedFIFO, 0660);
    if(mkfifo(rejectedFIFO, S_IRUSR | S_IWUSR) != 0){
        if (errno != EEXIST) { //file already exists
            perror("Error while creating Rejected FIFO.\n");
            exit(-1);
        }
    }
    rej_fifo_fd=open(REJECTED_FIFO, O_RDONLY);

    /* Create register file */
    printf("Creating register file\n");
    char path_reg[16];
    sprintf(path_reg, "/tmp/ger.%d\n", getpid());
    fp_register = fopen(path_reg, "w");

    if (fp_register == NULL) {
        perror("Register file wasn't created for gerador.\n");
        exit(1);
    }
    
    /* Write file header */
    fprintf(fp_register, "   inst   – pid –   p: g –   dur  –   tip\n");
    fprintf(fp_register, "------------------------------------------------\n");
    
    /* Gets starting time of the program */
    printf("Starting counting time\n");
    gettimeofday(&tv, NULL);
    start_time = (tv.tv_sec) * 1000000 + (tv.tv_usec);
    
    /* Generate orders */
    printf("Starting generate orders thread\n");
    pthread_t gen_pth = generateOrders();
    
    /* Listens for rejected orders */
    printf("Starting receive rejected thread\n");
    pthread_t rej_pth = receiveRejected();
    
    pthread_join(gen_pth, NULL);
    printf("Generate orders thread ended\n");
    pthread_join(rej_pth, NULL);
    printf("Receive rejecte thread ended\n");

    statsGeneratedGerador();

    /* unlink fifos */
    fclose(fp_register);
    close(fd_order_fifo);
    close(rej_fifo_fd);
    unlink(orderFIFO);
    unlink(rejectedFIFO);
    pthread_exit(NULL);

    return 0;
}
