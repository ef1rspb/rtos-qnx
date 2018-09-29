#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/iofunc.h>
#include <sys/dispatch.h>
#include <sys/neutrino.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "roby.h"

#define TIMER_W_SET   1
#define TIMER_W_UNSET 4
#define TIMER_F_SET   2
#define TIMER_F_UNSET 8
#define TIMER_W_COUNT 33
#define TIMER_F_COUNT 65

#define ATTACH_POINT "apu/roby"

// пересылаемое сообщение
struct MESSAGE
{
    // message type
    unsigned char type;
    // send data
    unsigned int buf;
};

int coid; // Идентификатор соединения, который использует эмулятор робота Roby.
int chidWF; // Идентификатор канала, которого должен создать поток DisplayWF.
int stateW = 0; // Направления движения по координатам W и
int stateF = 0; // F соответственно: -1 - назад; 0 - нет движения; +1 - вперёд.

int ActiveW = 0, ActiveF = 0; // Активные координаты W и F: переменная ActiveW или ActiveF
	//устанавливается в 1, когда нужно заставить робота Roby начать движение
	//по координате W или F соответственно.

int main_pulse_priority = 10; //Приоритет импульсов, поступающих от потока main к потоку DisplayWF.

void* DisplayXYZ()
{
    struct MESSAGE msg;
    struct _pulse pulse;
    int chid, x_cnt, y_cnt, z_cnt;
    x_cnt = y_cnt = z_cnt = 0;
    int changed = 0;

    chid = ChannelCreate(0);
    printf("Создан канал %d\n ", chid);

    // Инициализация датчиков координат X,Y,Z
    for(int i = 4; i < 7; i++)
    {
        msg.type = i;
        msg.buf = chid;
        MsgSend(coid, &msg, sizeof(msg), NULL, 0);
    }

    for(;;)
    {
        MsgReceivePulse(chid, &pulse, sizeof(pulse), NULL);

        switch(pulse.code)
        {
        case B_X:                       // Если изменилась координата X.
            x_cnt = pulse.value.sival_int;
            changed = 1;
            break;

        case B_Y:                       // Если изменилась координата Y.
            y_cnt = pulse.value.sival_int;
            changed = 1;
            break;

        case B_Z:                       // Если изменилась координата Z.
            z_cnt = pulse.value.sival_int;
            changed = 1;
            break;

        default:                        // Если изменились другие координаты.
            changed = 0;
            break;
        }

        if(changed)
        {
            // Вывод текущих координат на экран.
                    printf("Roby's coordinates:\r\n\tX=%d\r\n\tY=%d\r\n\tZ=%d\r\n",
                        x_cnt,
                        y_cnt,
                        z_cnt);
        }
    }
}

void* DisplayWF()
{
    struct MESSAGE msg; // Посылаемое сообщение.
    //unsigned char rmsg; // Буфер ответного сообщения.
    struct _pulse pulse;
    unsigned int w_cnt = 0, f_cnt = 0; // Счётчики положения по W и F соответственно.

    // Создать канал для передачи импульсов
    chidWF = ChannelCreate(0);
    printf("\nDisplayWF thread: created channel has chidWF = %d\n", chidWF);

    // Создать канальное соединение для двух таймеров по каналу chidWF.
    pid_t PID = getpid();
    int coidW = ConnectAttach(0, PID, chidWF, 0, 0);
    int coidF = ConnectAttach(0, PID, chidWF, 0, 0);

    /*
    Заполнить структуру sigevent для таймеров,
    используя макровызовы SIGEV_PULSE_INIT()
    */
    struct sigevent pulse_event_W, pulse_event_F; //Структуры уведомлений для таймеров по W и по F.

    /*
    Синтаксис для макроса SIGEV_PULSE_INIT()
    SIGEV_PULSE_INIT(eventp, coid, priority, code, value), где
        eventp - указатель на заполняемую структуру sigevent,
        при этом eventp соответствует полю sigev_notify в структуре struct sigevent
        и задаёт способ уведомления (в данном случае SIGEV_PULSE - импульсы);

        coid - идентификатор соединения,
        priority - приоритет, например, значение priority = SIGEV_PULSE_PRIO_INHERIT
        предотвращает изменение приоритета принимающего потока;
        code - код импульса,
        value - значение импульса.
    */

    SIGEV_PULSE_INIT(&pulse_event_W, coidW, SIGEV_PULSE_PRIO_INHERIT, 0, TIMER_W_COUNT);
    SIGEV_PULSE_INIT(&pulse_event_F, coidF, SIGEV_PULSE_PRIO_INHERIT, 0, TIMER_F_COUNT);

    /*
    Создать виртуальные таймеры для координат W и F,
    используя системные вызовы TimerCreate()
    */

    /*
    Синтаксис для timer_create()
    timer_create(clockid_t clock_id, struct sigevent *event, timer_t *timer_id), где
        clock_id - тип таймера: CLOCK_REALTIME, CLOCK_SOFTTIME, CLOCK_MONOTONIC;
        *event - указатель на структуру sigevent;
        *timer_id - указатель на идентификатор таймера.
    */

    timer_t timerW, timerF; // Идентификаторы таймеров по W и F.

    timer_create(CLOCK_REALTIME, &pulse_event_W, &timerW);
    timer_create(CLOCK_REALTIME, &pulse_event_F, &timerF);

    /*
    Послать инициализирующие команды для датчиков крайних положений
    по W и F к Roby, используя глобальную переменную coid
    */

    // Инициализация датчиков крайних положений по координатам W и F

    //Буфер сообщения msg.buf для команд инициализации датчиков содержит номер канала chidWF.
    msg.buf = chidWF;

    // Инициализация датчика координаты W
    msg.type = 7;
    MsgSend(coid, &msg, sizeof(msg), NULL, 0);

    // Инициализация датчика координаты F
    msg.type = 8;
    MsgSend(coid, &msg, sizeof(msg), NULL, 0);

    // Установка значений временных интервалов

    /*
    Новая и старая спецификации интервальных
    таймеров соответственно.
    */
    struct itimerspec itspec, old_itspec;

    /*
    Задержка перед однократной (первой) генерацией
    события - это время (для относительного таймера - относительно
    текущего момента времени), после истечения которого таймер
    запустится на генерирование событий.
    */
    itspec.it_value.tv_sec = 0;

    /*
    Задаётся интервал для каждой циклической перезагрузки таймера.
    Данные значения для структуры itspec соответствуют таймеру с интервалом
    генерации событий, равным 5 миллисекундам.
    */
    itspec.it_interval.tv_sec = 0;
    itspec.it_interval.tv_nsec = 5000000;


    while(1)
    {
        //sleep(1); //Вернуть управление потоку main (на некоторое время).

        // Приём импульсных сообщений по открытому каналу chidWF
        MsgReceivePulse(chidWF, &pulse, sizeof(pulse), NULL);

        // Анализ кода импульса
        switch(pulse.code)
        {
        // Импульс от потока main: запустить таймер по W
        case TIMER_W_SET:
            ActiveW = 0;
            /*
            Таймер по W будет запущен, а команду на его запуск
            (для будущих импульсов) сделать неактивной,
            т. е., когда работает запущенный таймер,
            не нужно его снова инициализировать.
            */

            printf("\nDisplayWF thread: pulse code = %d from main thread: start timer W.", pulse.code);

            /*
            В потоке DisplayWF запускать и останавливать таймеры нужно
            с помощью системных вызовов TimerSettime()
            */

            /*
            timer_settime(timer_t timerid,
                            int flags,
                            struct itimerspec *value,
                            struct itimerspec *oldvalue), где
                timerid - идентификатор таймера;
                flags - флаги;
            */

            itspec.it_value.tv_nsec = 5000000; // 5 миллисекунд

            // При flags == NULL задан относительный таймер по W
            timer_settime(timerW, 0, &itspec, &old_itspec);
            break;

        // Импульс от потока main: запустить таймер по F
        case TIMER_F_SET:
            ActiveF = 0;
            printf("\nDisplayWF thread: pulse code = %d from main thread: start timer F\n", pulse.code);

            itspec.it_value.tv_nsec = 5000000; // 5 миллисекунд
            timer_settime(timerF, 0, &itspec, &old_itspec);
            break;

        // Импульс от таймера по W: изменить значение счётчика w_cnt
        case TIMER_W_COUNT:
            /*
            Инкрементация или декрементация счётчиков положения по W и F определяется
            значениями глобальных переменных stateW и stateF,
            т. е. направлениями движения по W и F (w_cnt+=stateW; f_cnt+=stateF;)
            */

            printf("\nDisplayWF thread: pulse code = %d from timer W: change w_cnt\n", pulse.code);
            w_cnt += stateW;
            break;

        // Импульс от таймера по F: изменить значение счётчика f_cnt
        case TIMER_F_COUNT:
            printf("\nDisplayWF thread: pulse code = %d from timer F: change f_cnt\n", pulse.code);
            f_cnt += stateF;
            break;

        // Импульс от датчика крайнего положения по W
        case W_BEGIN: case W_END:
            printf("\nDisplayWF thread: pulse code = %d from end position on W: stop timer W\n", pulse.code);
            itspec.it_value.tv_nsec = 0; // Для остановки таймера по W
            timer_settime(timerW, 0, &itspec, &old_itspec); // Остановить таймер по W
            stateW = 0; // Нет движения по W
            break;

        // Импульс от датчика крайнего положения по F
        case F_BEGIN: case F_END:
            printf("\nDisplayWF thread: pulse code = %d from end position on F: stop timer F\n", pulse.code);
            itspec.it_value.tv_nsec = 0; // Для остановки таймера по F
            timer_settime(timerF, 0, &itspec, &old_itspec); // Остановить таймер по F
            stateF = 0; // Нет движения по F
            break;

        default:
            // Если код импульса отрицателен, то принят системный импульс.
            if(pulse.code < 0)
            {
                printf("\nDisplayWF thread: pulse code = %d from system.", pulse.code);
            }
        }

        /*
        Вывод значений координат W и F, а также кодов
        и значений принимаемых импульсов
        */
        printf("\nW = %d, F = %d", w_cnt, f_cnt);
    }

    ChannelDestroy(chidWF);
    pthread_exit(NULL);
}

int main()
{
    unsigned char PA=0, PC=0; //Эти переменные хранят значения регистров PA и PC
    struct MESSAGE msg; // буфер посылаемого сообщения
    unsigned char rmsg; // Буфер ответного сообщения
    char command, ch[3];

    coid = name_open(ATTACH_POINT, 0);
    if(coid == -1)
    {
        return EXIT_FAILURE;
    }

    printf("apu/roby has coid = %d\n", coid);

    pthread_create(NULL, NULL, &DisplayXYZ, NULL);
    pthread_create(NULL, NULL, &DisplayWF, NULL);

    /*
    Приостановить поток main, чтобы позволить потокам DisplayXYZ и DisplayWF
    подготовиться к работе (например, инициализировать датчики движения по X, Y и Z
    и датчики крайних положений по W и F эмулятора робота roby).
     */
    sleep(5);

    pid_t PID = getpid();
    // Создать соединение с каналом chidWF (для запуска таймеров).
    int coid_DisplayWF = ConnectAttach(0, PID, chidWF, 0, 0);

    do
    {
        sleep(1); //Передать управление потоку DisplayXYZ или потоку DisplayWF (на некоторое время)

        printf("Enter command>\n");
        scanf("%s", ch);

        switch(ch[0])
        {
        case 'I':
            // Двигаться в начальное положение по всем координатам
            // и сбросить значения датчиков положений.
            PA = 0; PC = 0; //Для этого сбросить в 0 переменные PA и PC.
            command ='C'; //Подать команду записи в регистр PC роботу Roby.
            break;

        case '+': case '=':
                command = 'c';
                break;
        case '!':
                command = 'B';
                break;

        case 'Y':
            scanf("%s", ch);
            switch(ch[0])
            {
            case 'X':
                PA ^= A_X_FORWARD;
                command ='A';
                break;
            case 'x':
                PA ^= A_X_BACK;
                command ='A';
                break;
            case 'Y':
                PA ^= A_Y_FORWARD;
                command ='A';
                break;
            case 'y':
                PA ^= A_Y_BACK;
                command ='A';
                break;
            case 'Z':
                PA ^= A_Z_FORWARD;
                command ='A';
                break;
            case 'z':
                PA ^= A_Z_BACK;
                command ='A';
                break;
            case 'F':
                PC ^= C_F_FORWARD;
                command ='C';
                ActiveF = 1; // F - активная координата.
                if(stateF == 1) //Если Roby движется вперёд по F,
                    stateF = 0; //то стоп по F.
                else
                    stateF = 1; // Иначе запустить движение вперёд по F.
                break;
            case 'f':
                PC ^= C_F_BACK;
                command ='C';
                ActiveF = 1; // F - активная координата.
                if(stateF == -1) // Если Roby движется назад по F,
                    stateF = 0; //то стоп по F.
                else
                    stateF = -1; // Иначе запустить движение назад по F.
                break;
            case 'W':
                PC ^= C_W_FORWARD;
                command ='C';
                ActiveW = 1; // W - активная координата.
                if(stateW == 1 ) // Если Roby двигался вперёд по W,
                    stateW = 0; // то остановить его по W.
                else
                    stateW = 1; // Иначе запустить движение вперёд по W.
                break;
            case 'w':
                PC ^= C_W_BACK;
                command ='C';
                ActiveW = 1; //W - активная координата.
                if(stateW == -1) // Если Roby двигался назад по W,
                	stateW = 0; // то остановить его по W.
                else
                	stateW = -1; // Иначе запустить движение назад по W.
                break;
            case 'S':
                PA ^= A_S;
                command ='A';
                break;
            case 'D':
                PA ^= A_D;
                command ='A';
                break;
            }

        }

        switch(command)
            {
            case 'A':
                msg.buf = PA;
                msg.type = 0;
                MsgSend(coid, &msg, sizeof(msg), NULL, 0);
                break;
            case 'B':
                msg.type = 3;
                MsgSend(coid, &msg, sizeof(msg), &rmsg, sizeof(rmsg));
                printf("Port B: 0x%X\n", rmsg);
                break;
            case 'c':
                msg.type = 2;
                MsgSend(coid, &msg, sizeof(msg), &rmsg, sizeof(rmsg));
                printf("Port C: 0x%X\n", rmsg);
                break;
            case 'C':
                msg.type = 1;
                msg.buf = PC;

                /*
                Анализ входной комманды оператора:
                определить координату движения (W или F); (stateW или stateF уже установлены)
                - послать импульсные сообщения для запуска таймеров
                потоку DisplayWF, используя MsgSendPulse()
                */

                if(ActiveW) // Если стала активной координата W, то
                    MsgSendPulse(coid_DisplayWF, main_pulse_priority, TIMER_W_SET, 0); // запустить таймер по W
                if(ActiveF) // Если стала активной координата F, то
                    MsgSendPulse(coid_DisplayWF, main_pulse_priority, TIMER_F_SET, 0); // запустить таймер по F

                MsgSend(coid, &msg, sizeof(msg), NULL, 0);
                break;
            default:
                printf("default last\n");
                    break;
            }

    }  while(1);

}
