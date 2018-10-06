#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <sys/iofunc.h>
#include <sys/dispatch.h>
#include <sys/neutrino.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <pthread.h>
#include <signal.h>
#include <time.h>
#include <sys/siginfo.h>
#include "roby.h"

struct MESSAGE //Пересылаемое сообщение send message structure
{
 unsigned char      type; // 0,1,2,3,4,5,6
 unsigned int       buf;// send data
};

void* DisplayXYZ(); // Прототип функции DisplayXYZ().
void handler(int); // Прототип функции handler().
void* DisplayWF(); // Прототип функции DisplayWF().

int chidWF; //Идентификатор канала, которого должен создать поток DisplayWF.
int stateW=0; //Направления движения по координатам W и
int stateF=0; //F соответственно: -1 - назад; 0 - нет движения; +1 - вперёд.
int ActiveW=0, ActiveF=0; //Активные координаты W и F: переменная ActiveW или ActiveF
	//устанавливается в 1, когда нужно заставить робота Roby начать движение
	//по координате W или F соответственно.
unsigned int w_cnt=0, f_cnt=0; //Счётчики положения по W и F соответственно.
unsigned int X=0, Y=0, Z=0; //Эти переменные содержат значения координат X, Y и Z
	//для робота Roby.

int main_pulse_priority=10; //Приоритет импульсов, поступающих от потока main
	//к потоку DisplayWF.
int coid_roby; //Идентификатор соединения, который использует эмулятор робота Roby.

unsigned char DisplayState=0; //Режим отображения состояния датчиков:
	//0 - выключен; 1 - включен.

//-----Определения функций-----------------------------------------------------------
int main()
{
unsigned char PA=0, PC=0; //Эти переменные хранят значения регистров PA и PC
	//робота Roby.
 char ch[2]; //Символ, введённый пользователем с клавиатуры.
char Help[]="Unknown command. Please, use next keys: <F1>-<F12>, <Enter>, <+/=> or <Esc> for exit."; //Справка.
char command; //Тип команды.

int status;
struct MESSAGE msg; //Посылаемое сообщение.
unsigned char rmsg; //Буфер ответного сообщения.
//char command[10]; //Буфер для команд оператора.
int coid; //Идентификатор соединения, который использует эмулятор робота Roby.

pthread_t DisplayXYZ_tid; //TID (идентификатор потока) для потока DisplayXYZ.
pthread_t DisplayWF_tid; //TID для потока DisplayWF.

pthread_create(&DisplayXYZ_tid, NULL, &DisplayXYZ, NULL); //Создать поток-сын DisplayXYZ
pthread_create(&DisplayWF_tid, NULL, &DisplayWF, NULL); //и поток-сын DisplayWF.
//pthread_create(&tid, &attr, &Display, NULL);

//....Вставлены операторы.
coid_roby=coid=name_open("apu/roby", NULL); //Определить coid (идентификатор соединения)
	//по имени "apu/roby".
//int name_open( const char * name,
//               int flags );
//...........
printf("apu/roby has coid=%d\n\n",coid);

sleep(5); //Приостановить поток main, чтобы позволить потокам DisplayXYZ и DisplayWF
	//подготовиться к работе (например, инициализировать датчики движения по X, Y и Z
	//и датчики крайних положений по W и F эмулятора робота roby).

pid_t PID=getpid(); int coid_DisplayWF=ConnectAttach(0, PID, chidWF, 0, 0); //Создать
	//соединение с каналом chidWF (для запуска таймеров).

/*****Syntax for ConnectAttach()*****************
 * int ConnectAttach(int nd, //Node Descriptor - дескриптор узла в сети (nd=0, если
 *				//локальное соединение);
 *		pid_t pid, //PID (Process ID);
 *		int chid, //CHID (Channel ID);
 *		unsigned index,
 *		int flags); //Флаги.
 ************************************************/

printf("\nPult1_var2: main thread: please, press any key.");

do { //Бесконечный цикл.
		sleep(1); //Передать управление потоку DisplayXYZ или
			//потоку DisplayWF (на некоторое время).

//.....Интерфейс с пользователем.
	printf("\n>"); //Приглашение выбрать команду с помощью клавиши (для пользователя).
	//scanf("%s",command); //Считать команду оператора с клавиатуры.
	scanf("%s", ch); //Считать символ, введённый с клавиатуры.

	switch(ch[0]){ //Анализ ASCII-кода нажатой клавиши.
	case 'e': case 'E': //Нажата клавиша <Esc>.
		printf("\nExit..."); //Завершение программы.
		command='E'; //Тип команды.
		return 0;
	case 'I': //Нажата клавиша <Enter>.
		printf("\nMove to start position on all coordinates"
			"\nand clear position values.");
		//Двигаться в начальное положение по всем координатам
		//и сбросить значения датчиков положений.
		//command=...; //Тип команды.
		PA=0; PC=0; //Для этого сбросить в 0 переменные PA и PC.
		command='C'; //Подать команду записи в регистр PC роботу Roby.
		break;
	case '+': case '=': //Нажата клавиша <+/=>.
		//Переключить режим отображения состояния датчиков.
		DisplayState^=0x01; //Поразрядное исключающее ИЛИ (XOR)
			//для переменной DisplayState сбрасывает её в 0
			//либо устанавливает в 1.
		printf("\n<+/=> - display state = %d.", DisplayState);
		//command=...; //Тип команды.
		break;
	case '0': //Нажата управляющая или функциональная клавиша.
		scanf("%s", ch);
		switch(ch[0]){ //Это требует дополнительного анализа.
		case '1': //Нажата клавиша <F1>.
			//Начать/остановить движение по X вперёд.
			printf("\n<F1> - toggle moving on X forward.");
			PA^=A_X_FORWARD; //Переключить бит A_X_FORWARD в регистре PA.
			command='A'; //Тип команды.
			break;
		case '2': //Нажата клавиша <F2>.
			//Начать/остановить движение по X назад.
			printf("\n<F2> - toggle moving on X backward.");
			PA^=A_X_BACK; //Переключить бит A_X_BACK в регистре PA.
			command='A'; //Тип команды.
			break;
		case '3': //Нажата клавиша <F3>.
			//Начать/остановить движение по Y вперёд.
			printf("\n<F3> - toggle moving on Y forward.");
			PA^=A_Y_FORWARD; //Переключить бит A_Y_FORWARD в регистре PA.
			command='A'; //Тип команды.
			break;
		case '4': //Нажата клавиша <F4>.
			//Начать/остановить движение по Y назад.
			printf("\n<F4> - toggle moving on Y backward.");
			PA^=A_Y_BACK; //Переключить бит A_Y_BACK в регистре PA.
			command='A'; //Тип команды.
			break;
		case '5': //Нажата клавиша <F5>.
			//Начать/остановить движение по Z вперёд.
			printf("\n<F5> - toggle moving on Z forward.");
			PA^=A_Z_FORWARD; //Переключить бит A_Z_FORWARD в регистре PA.
			command='A'; //Тип команды.
			break;
		case '6': //Нажата клавиша <F6>.
			//Начать/остановить движение по Z назад.
			printf("\n<F6> - toggle moving on Z backward.");
			PA^=A_Z_BACK; //Переключить бит A_Z_BACK в регистре PA.
			command='A'; //Тип команды.
			break;
		case '7': //Нажата клавиша <F7>.
			//Начать/остановить движение по F вперёд.
			printf("\n<F7> - toggle rotating on F forward.");
			PC^=C_F_FORWARD; //Переключить бит C_F_FORWARD в регистре PC.
			command='C'; //Тип команды.
			ActiveF=1; //F - активная координата.
			if(stateF==1) //Если Roby движется вперёд по F,
				stateF=0; //то стоп по F.
			else //Иначе
				stateF=1; //запустить движение вперёд по F.
			break;
		case '8': //Нажата клавиша <F8>.
			//Начать/остановить движение по F назад.
			printf("\n<F8> - toggle rotating on F backward.");
			PC^=C_F_BACK; //Переключить бит C_F_BACK в регистре PC.
			command='C'; //Тип команды.
			ActiveF=1; //F - активная координата.
			if(stateF==-1) //Если Roby движется назад по F,
				stateF=0; //то стоп по F.
			else //Иначе
				stateF=-1; //запустить движение назад по F.
			break;
		case '9': //Нажата клавиша <F9>.
			//Начать/остановить движение по W вперёд.
			printf("\n<F9> - toggle rotating head on W forward.");
			PC^=C_W_FORWARD; //Переключить бит C_W_FORWARD в регистре PC.
			command='C'; //Тип команды.
			ActiveW=1; //W - активная координата.
			if(stateW==1) //Если Roby двигался вперёд по W,
				stateW=0; //то остановить его по W.
			else //Иначе
				stateW=1; //запустить движение вперёд по W.
			break;
		case 68: //Нажата клавиша <F10>.
			//Начать/остановить движение по W назад.
			printf("\n<F10> - toggle rotating head on W backward.");
			PC^=C_W_BACK; //Переключить бит C_W_BACK в регистре PC.
			command='C'; //Тип команды.
			ActiveW=1; //W - активная координата.
			if(stateW==-1) //Если Roby двигался назад по W,
				stateF=0; //то остановить его по W.
			else //Иначе
				stateF=-1; //запустить движение назад по W.
			break;
		case 133: //Нажата клавиша <F11>.
			//Включить/выключить схват S.
			printf("\n<F11> - toggle S.");
			PA^=A_S; //Переключить бит A_S в регистре PA.
			command='A'; //Тип команды.
			break;
		case 134: //Нажата клавиша <F12>.
			//Включить/выключить дрель D.
			printf("\n<F12> - toggle drill work.");
			PA^=A_D; //Переключить бит A_D в регистре PA.
			command='A'; //Тип команды.
			break;
		default: //Нажата другая (управляющая или функциональная) клавиша.
			printf("\n%s", Help);
			command='U'; //Неизвестный тип команды.
		} //End of switch().
		break;
	default: //Нажата другая клавиша.
		printf("\n%s", Help); //puts(Help);
		command='U'; //Неизвестный тип команды.
	} //End of switch().
	if(DisplayState==1){ //Если включен режим отображения состояния датчиков, то
		//command='b'; //Команда чтения из порта PB роботу Roby.
		command='c'; //Команда чтения из порта PC роботу Roby.
	} //End of if().
//...............................

//.....Анализ типа команды command.
	switch(command) { //Анализировать тип команды, заданный символом command.
	//switch(command[0]) { //Проверить символ, с которого начинается команда.

        	case 'A':       //Write port A.
	msg.type=0; //Тип сообщения.
	msg.buf=PA; //Записать в буфер сообщения msg.buf значение переменной PA.
		//sscanf(command+1,"%X", &msg.buf); //Преобразовать строку (command+1)
		//из 16-ичного вида и сохранить результат в msg.buf.
//.....Вставлены операторы.
	printf("Write port A.");
	status=MsgSend(coid, &msg, sizeof(msg), &rmsg, sizeof(rmsg));
//int MsgSend( int coid, //Послать сообщение, где coid - идентификатор соединения;
//             const void* smsg, //smsg - указатель на посылаемое сообщение;
//             int sbytes, //sbytes - размер (в байтах) посылаемого сообщения;
//             void* rmsg, //rmsg - указатель на буфер принимаемого ответа от roby;
//             int rbytes ); //rbytes - размер (в байтах) буфера для принимаемого ответа.
	break;

        	case 'C':       //Write port C.
	msg.type=1; //Тип сообщения.
//.....Вставлены операторы.
	msg.buf=PC; //Записать в буфер сообщения msg.buf значение переменной PC.
		//sscanf(command+1,"%X", &msg.buf); //Считать 16-ичную строку из (command+1)
		//в буфер msg.buf.
	printf("Write port C.");

		if(ActiveW) //Если стала активной координата W, то
			MsgSendPulse(coid_DisplayWF, main_pulse_priority,
				 TIMER_W_SET, NULL); //запустить таймер по W.
		if(ActiveF) //Если стала активной координата F, то
			MsgSendPulse(coid_DisplayWF, main_pulse_priority,
				 TIMER_F_SET, NULL); //запустить таймер по F.
	//.......................................

	status=MsgSend(coid, &msg, sizeof(msg), &rmsg, sizeof(rmsg)); //Послать
		//сообщение msg к roby.
	break;

		case 'c':       //Read port C.
	msg.type=2; //Тип сообщения.
//......Вставлены операторы.
	//Буфер сообщения msg.buf при чтении порта C заполнять не требуется.
		//sscanf(command+1,"%X", &msg.buf); //*(command+1) => msg.buf
	printf("Read port C.");
	status=MsgSend(coid, &msg, sizeof(msg), &rmsg, sizeof(rmsg)); //Послать
		//сообщение msg к roby и принять ответ rmsg от roby.
	printf("Port C: 0x%X", rmsg); //Вывод на экран (в поток stdout) ответ rmsg
		//в 16-ичном виде.
	break;

		case 'b':       //Read port B.
	msg.type=3; //Тип сообщения.
//........Вставлены операторы.
	//Буфер сообщения msg.buf при чтении порта B заполнять не требуется.
		//sscanf(command+1,"%X", &msg.buf); //*(command+1) => msg.buf
	printf("Read port B.");
	status=MsgSend(coid, &msg, sizeof(msg), &rmsg, sizeof(rmsg)); //Послать
		//сообщение msg к roby и принять ответ rmsg от roby.
	printf("Port B: 0x%X", rmsg); //Вывод на экран (в поток stdout) ответ rmsg
		//в 16-ичном виде.
	break;

		case 'E': //Выход из программы.
	return 0;       //Return from main().

		default:	//Другой символ.
	printf("Unknown command\n");
	//break;
	}
} while(1);
}

void* DisplayXYZ()
{
    struct sigaction act;
    sigset_t set; // Набор сигналов.
    siginfo_t info; // Информация о сигнале.

    int changed = 0;
    int coid = coid_roby;
    int x_cnt, y_cnt, z_cnt;
    x_cnt = y_cnt = z_cnt = 0;

    struct MESSAGE msg;

    sigemptyset(&set); // Очистить набор сигналов set.
    sigaddset(&set, SIGUSR1); // Добавить в набор set сигнал SIGUSR1.

    act.sa_mask = set; // Маска сигналов.
    act.sa_handler = &handler; // Указать обработчик сигналов.
    /*
    Если в sa_flags указан SA_SIGINFO,
    то sa_sigaction (вместо sa_handler) задаёт функцию обработки сигнала signum.
    В первом аргументе функция принимает номер сигнала
     */
    act.sa_flags = SA_SIGINFO;

    /* Установить связь между получением
    данным процессом сигнала SIGUSR1 и действием act,
    которое предусматривает вызов обработчика сигналов handler()
    */
    sigaction(SIGUSR1, &act, NULL);


    // Инициализация датчиков перемещения по X, Y и Z робота roby
    msg.buf = 0;
    msg.type = 4;
    MsgSend(coid, &msg, sizeof(msg), NULL, 0);

    msg.type = 5;
    MsgSend(coid, &msg, sizeof(msg), NULL, 0);

    msg.type = 6;
    MsgSend(coid, &msg, sizeof(msg), NULL, 0);

    while(1)
    {
        //sleep(1); //Вернуть управление потоку main (на некоторое время).

        // Приём СИГНАЛА SIGUSR1 от roby
        // Ожидать СИГНАЛ из набора set.
        sigwaitinfo(&set, &info);

        // Анализ сигнала по коду
        switch(info.si_code)
        {
        case -B_X:
            x_cnt = info.si_value.sival_int;
            changed = 1;
            break;
        case -B_Y:
            y_cnt = info.si_value.sival_int;
            changed = 1;
            break;
        case -B_Z:
            z_cnt = info.si_value.sival_int;
            changed = 1;
            break;
        default:
            changed = 0;
            printf("\nDisplay thread: unknown signal code: %i", info.si_code);
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

    // Завершить работу потока Display
    pthread_exit(NULL);
}

void handler(int signo)
{
    // Обработчик сигналов SIGUSR1.
}

void* DisplayWF()
{
    struct MESSAGE msg; // Посылаемое сообщение.
    struct _pulse pulse;
    unsigned int w_cnt = 0, f_cnt = 0; // Счётчики положения по W и F соответственно.
    int coid = coid_roby;

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
