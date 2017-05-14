#include <sys/syscall.h>

#define ORDER_FIFO "/tmp/entrada"
#define REJECTED_FIFO "/tmp/rejeitados"

#define gettid() syscall(SYS_gettid)