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

#define ATTACH_POINT "apu/roby"
#define COMMANDS_MAX_COUNT 10

// пересылаемое сообщение
struct MESSAGE
{
    // message type
    unsigned char type;
    // send data
    unsigned int buf;
};

int coid;

void *Display() {
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

    pthread_create(NULL, NULL, &Display, NULL);

    do
    {
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
                break;
            case 'f':
                PC ^= C_F_BACK;
                command ='C';
                break;
            case 'W':
                PC ^= C_W_FORWARD;
                command ='C';
                break;
            case 'w':
                PC ^= C_W_BACK;
                command ='C';
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
                MsgSend(coid, &msg, sizeof(msg), NULL, 0);
                break;
            default:
                printf("default last\n");
                    break;
            }

    }  while(1);

}
