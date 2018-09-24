#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

int main()
{
    extern void handler();
    struct sigaction act;
    sigset_t set;

    sigemptyset( &set );
    sigaddset( &set, SIGUSR1 );
    sigaddset( &set, SIGUSR2 );

    /*
     * Define a handler for SIGUSR1 such that when
     * entered both SIGUSR1 and SIGUSR2 are masked.
     */
    act.sa_flags = 0;
    act.sa_mask = set;
    act.sa_handler = &handler;
    sigaction( SIGUSR1, &act, NULL );
	printf("I am main\n");
    kill( getpid(), SIGUSR1 );

    /* Program will terminate with a SIGUSR2 */
    return EXIT_SUCCESS;
}

void handler( signo )
{
    static int first = 1;
    printf("I am Signal handler\n");
    if(first)
    {
    	first = 0;
    	kill( getpid(), SIGUSR1 );  /* Prove signal masked */
    	kill( getpid(), SIGUSR2 );  /* Prove signal masked */
    }
}
