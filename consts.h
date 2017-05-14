
/*#define ORDER_FIFO     ((const unsigned char *)"/tmp/entrada")
#define REJECTED_FIFO  ((const unsignedchar *)"/tmp/rejeitados")*/

#include <sys/syscall.h>

#define ORDER_FIFO "/tmp/entrada"
#define REJECTED_FIFO "/tmp/rejeitados"

#define gettid() syscall(SYS_gettid)