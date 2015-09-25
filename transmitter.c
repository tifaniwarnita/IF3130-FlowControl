/* 
	Tifani Warnita	13513055
	Asanilta Fahda	13513079

	File	: transmitter.c
*/ 

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <pthread.h>
#include "transmitter.h"

Byte rcvd_signal;
struct sockaddr_in myaddr, remaddr;
int sockfd, i, slen=sizeof(remaddr);
int recvlen;

int main(int argc, char* argv[])
{
	char *server = argv[1];
	int service_port = atoi(argv[2]);
	char *filename = argv[3];
	int msgcnt = 0;
	Byte ch;

	printf(">> transmitter %s %d %s\n",server,service_port,filename);

	/* create a socket */

	if ((sockfd=socket(AF_INET, SOCK_DGRAM, 0))==-1)
		printf("Membuat socket untuk koneksi ke %s:%d\n",server,service_port);

	/* bind it to all local addresses and pick any port number */

	/*memset((char *)&myaddr, 0, sizeof(myaddr));
	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	myaddr.sin_port = htons(0);

	if (bind(sockfd, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
		perror("bind failed");
		return 0;
	}  */    

	memset((char *) &remaddr, 0, sizeof(remaddr));
	remaddr.sin_family = AF_INET;
	remaddr.sin_port = htons(service_port);
	if (inet_aton(server, &remaddr.sin_addr)==0) {
		fprintf(stderr, "inet_aton() failed\n");
		exit(1);
	}


	/* Create child process */
	pthread_t signalreceiver;
	pthread_create(&signalreceiver, NULL, receiveSignal, NULL);


	/*Parent Process*/
	FILE *fp;
	fp = fopen(filename,"r");
	
	while (!feof(fp)) {
		if (rcvd_signal != XOFF) {
			ch = fgetc(fp);
			if(sendto(sockfd, &ch, 1, 0, (struct sockaddr *)&remaddr, slen)==-1) {
				perror("sendto");
				exit(1);
			} else {
				msgcnt++;
				if (ch==LF) {
					printf("Mengirim byte ke-%d: 'Line Feed'\n",msgcnt);
				} else if (ch==CR) {
					printf("Mengirim byte ke-%d: 'Carriage Return'\n",msgcnt);
				} else {
					printf("Mengirim byte ke-%d: '%c'-%d\n",msgcnt,ch,ch);
				}
			}
		} else {
			printf("XOFF diterima.\n");
			while (rcvd_signal==XOFF) {
				printf("Menunggu XON...\n");
				sleep(1);
			}
			printf("XON diterima.\n");
		}
	}
	ch = Endfile;
	if(sendto(sockfd, &ch, 1, 0, (struct sockaddr *)&remaddr, slen)==-1) {
	    perror("sendto");
	    exit(1);
	} else {
	    printf("End of file\n");
	}

	close(sockfd);
	return 0;
}

void* receiveSignal(void *threadSignalReceiver) {
	Byte ch;
	for(;;) {
		recvlen = recvfrom(sockfd, &ch, 1, 0, (struct sockaddr *)&remaddr, &slen);
        if (recvlen > 0) {
            rcvd_signal = ch;
    	}
	}
}
