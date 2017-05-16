#include <sys/syscall.h>

#define ORDER_FIFO "/tmp/entrada"
#define REJECTED_FIFO "/tmp/rejeitados"

#define gettid() syscall(SYS_gettid)

#define MAX_NUMBER_OF_ORDERS    30000
#define MAX_USAGE_TIME          999999
#define MAX_NUMBER_OF_SEATS     30000