#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <sys/neutrino.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>

#define BUFFER_LENGTH 25

void* server()
{
	int chid;
	int rcvd;
	char receive_buf[BUFFER_LENGTH];
	char reply_buf[BUFFER_LENGTH];
	// Создать канал для обмена сообщениями
	chid = ChannelCreate(0);

	// Заснуьб на 1 секунду для того, чтобы клиент успел инициализироваться
	sleep(1);
	// Ожидать сообщение в канале chid, поместить его в строку receive_buf
	rcvd = MsgReceive(chid, &receive_buf, sizeof(receive_buf), NULL);

	// Вывод сообщения в консоль
	printf("Server thread: message <%s> has received \n", &receive_buf);

	// Создать ответ для клиента
	strcpy(reply_buf, "Strong answer from Server");

	// Ответить клиенту с идентификатором rcvd, ответ - строка reply_buf
	MsgReply(rcvd, 1500052, &reply_buf, sizeof(reply_buf));

	// Закрыть канал
	ChannelDestroy(chid);

	// Завершение работы потока
	pthread_exit(NULL);
}

void* client()
{
	int coid;
	pid_t PID;
	char send_buf[BUFFER_LENGTH];
	char reply_buf[BUFFER_LENGTH];

	// Получить идентификатор текущего процесса.
	PID = getpid();

	// Установить соединение между процессом c идентификатором PID и каналом с идентификатором 1
	coid = ConnectAttach(0, PID, 1, 0, 0);
	// Поместить текст для отправки на сервер в строку send_buf
	strcpy(send_buf, "It is very simple example");

	// Отправка сообщения send_buf по соединению с идетификатором coid, ответ хранить в reply_buf
	MsgSend(coid, &send_buf, sizeof(send_buf), &reply_buf, sizeof(reply_buf));
	// Вывод ответа в консоль
	printf("Client thread: message <%s> has received \n", &reply_buf);

	// Разорвать соединение
	ConnectDetach(coid);

	// Завершение работы потока
	pthread_exit(NULL);
}

int  main()
{
	pthread_t server_tid, client_tid;
	pthread_create(&server_tid, NULL, &server, NULL);
	pthread_create(&client_tid, NULL, &client, NULL);

	// Функция pthread_join() блокирует вызывающий поток, пока указанный поток client_tid не завершится.
	pthread_join(client_tid,NULL);
	return EXIT_SUCCESS;
}
