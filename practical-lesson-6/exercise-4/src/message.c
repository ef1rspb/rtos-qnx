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
	chid = ChannelCreate(0);
	sleep(1);
	rcvd = MsgReceive(chid, &receive_buf, sizeof(receive_buf), NULL);
	printf("Server thread: message <%s> has received \n", &receive_buf);
	strcpy(reply_buf, "Strong answer from Server");
	MsgReply(rcvd, 1500052, &reply_buf, sizeof(reply_buf));
	ChannelDestroy(chid);
	pthread_exit(NULL);
}

void* client()
{
	int coid;
	pid_t PID;
	char send_buf[BUFFER_LENGTH];
	char reply_buf[BUFFER_LENGTH];
	PID = getpid();
	coid = ConnectAttach(0, PID, 1, 0, 0);
	strcpy(send_buf, "It is very simple example");
	MsgSend(coid, &send_buf, sizeof(send_buf), &reply_buf, sizeof(reply_buf));
	printf("Client thread: message <%s> has received \n", &reply_buf);
	ConnectDetach(coid);
	pthread_exit(NULL);
}

int  main()
{
	pthread_t server_tid, client_tid;
	pthread_create(&server_tid, NULL, &server, NULL);
	pthread_create(&client_tid, NULL, &client, NULL);
	pthread_join(client_tid,NULL);
	return EXIT_SUCCESS;
}
