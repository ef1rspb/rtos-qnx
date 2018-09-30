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

    /*
     В sa_mask задаётся маска сигналов, к
     оторые должны блокироваться (т.е. добавляется к маске сигналов нити,
     в которой вызывается обработчик сигнала)
     при выполнении обработчика сигнала.
     */
    act.sa_mask = set;

    // sa_handler указывает действие, которое должно быть связано с signum;
    act.sa_handler = &handler;

    /*
     Системный вызов sigaction() используется для
     изменения выполняемого процессом действия
     при получении определённого сигнала
     */
    sigaction( SIGUSR1, &act, NULL );

	printf("I am main\n");

	// Посылка сигнала SIGUSR1 от одного процесса другому
    kill( getpid(), SIGUSR1 );

    /* Program will terminate with a SIGUSR2 */
    return EXIT_SUCCESS;
}

void handler(int signo)
{
    static int first = 1;
    printf("I am Signal handler\n");
    printf("Signal code is %i\n", signo);
    if(first)
    {
    	first = 0;
    	// Посылка сигнала SIGUSR1 от одного процесса другому
    	kill( getpid(), SIGUSR1 );  /* Prove signal masked */

    	// Посылка сигнала SIGUSR2 от одного процесса другому
    	kill( getpid(), SIGUSR2 );  /* Prove signal masked */
    }
}
