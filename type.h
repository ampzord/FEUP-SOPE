
typedef struct {
    unsigned int serial_number;
    char gender;
    unsigned int time_spent;
    unsigned int rejected;
} Order;

struct timeval tv;

/* Get number of digits of number */
int findn(int num)
{
    char snum[100];
    sprintf(snum, "%d", num);
    return strlen(snum);
}