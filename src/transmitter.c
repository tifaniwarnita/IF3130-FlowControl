/* 
	Tifani Warnita	13513055
	Asanilta Fahda	13513079

	File	: transmitter.c
	Source	: Paul Krzyzanowski (https://www.cs.rutgers.edu/~pxk/417/notes/sockets/udp.html)
*/ 

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <pthread.h>
#include "transmitter.h"

Byte rcvd_signal;
struct sockaddr_in remaddr;
int sockfd, slen=sizeof(remaddr);
int recvlen;

int main(int argc, char* argv[])
{
	char *server = argv[1]; //IP address
	int service_port = atoi(argv[2]); //Port
	char *filename = argv[3]; //Text file's name
	int msgcnt = 0;
	Byte ch = Startfile;

	printf(">> transmitter %s %d %s\n",server,service_port,filename); //Transmitter information

	/* Create socket */
	if ((sockfd=socket(AF_INET, SOCK_DGRAM, 0))==-1) {
		perror("Pembuatan socket gagal\n");
		return 0;
	} else {
		printf("Membuat socket untuk koneksi ke %s:%d\n",server,service_port);   
	}

	memset((char *) &remaddr, 0, sizeof(remaddr));
	remaddr.sin_family = AF_INET;
	remaddr.sin_port = htons(service_port);
	if (inet_aton(server, &remaddr.sin_addr)==0) {
		fprintf(stderr, "inet_aton() failed\n");
		exit(1);
	}
	if(sendto(sockfd, &ch, 1, 0, (struct sockaddr *)&remaddr, slen)==-1) {
		perror("sendto");
		exit(1);
	}

	/* Create child process */
	pthread_t signalreceiver;
	pthread_create(&signalreceiver, NULL, receiveSignal, NULL);


	/*Parent Process*/
	FILE *fp;
	fp = fopen(filename,"r");
	
	/* Read file until EOF */
	while (!feof(fp)) {
		if (rcvd_signal != XOFF) { // Sending character only when XON
			ch = fgetc(fp);
			if (ch!=255) {
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
						printf("Mengirim byte ke-%d: '%c'\n",msgcnt,ch);
					}
				}
			}
			sleep(1);
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
/* Receives XON/XOFF signal from server */
	Byte ch;
	for(;;) {
		recvlen = recvfrom(sockfd, &ch, 1, 0, (struct sockaddr *)&remaddr, &slen);
        if (recvlen > 0) {
            rcvd_signal = ch;
    	}
	}
}
