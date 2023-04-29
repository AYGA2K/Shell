#include <sys/types.h> /* pid_t */
#include <sys/wait.h>  /* wait */
#include <unistd.h>    /* read, write */
typedef int Semaphore[2];
void Initsem(Semaphore S, int N);
void P(Semaphore S);
void V(Semaphore S);
void attente(int N);
void message(int i, char *s);
