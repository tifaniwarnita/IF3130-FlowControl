/*
	Tifani Warnita	13513055
	Asanilta Fahda	13513079

	File	: receiver.c
	Source	: Paul Krzyzanowski (https://www.cs.rutgers.edu/~pxk/417/notes/sockets/udp.html)
*/

#include <arpa/inet.h>
#include <pthread.h>
#include "receiver.h"
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>

#define DELAY 2500 // Delay to adjust speed of consuming buffer
#define BUFFSIZE 8 // Define receiver buffer size
#define UPPERLIMIT 8 // Max limit of buffer size before sending XOFF
#define LOWERLIMIT 2 // Lower limit of buffer size before sending XON

int sockfd; //Socket declaration

Byte rxbuf[BUFFSIZE]; // Data of Circular Buffer
QTYPE rcvq = { 0, 0, 0, BUFFSIZE, rxbuf }; //Circular Buffer
QTYPE *rxq = &rcvq;
Byte xonxoff = XON;
bool xon_active = false;
int recvlen; // # bytes received
int msgcnt = 0;	// Count # of messages we received
struct sockaddr_in myaddr;	// Our address
struct sockaddr_in remaddr;	// Remote address
socklen_t addrlen = sizeof(remaddr); // Length of addresses

/* Functions declaration */
static Byte *rcvchar(int socksockfd, QTYPE *queue);
static Byte *q_get(QTYPE *);
void* consumeBuff(void *);

int main(int argc, char **argv)
/* Argv and argc will be used to define port number */
{	
	Byte c;
	struct ifreq ifr;

	/* Create a socket and bind it to a specified port according argument initialization */ 
		/* Create a UDP socket */
		if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
			perror("Cannot create socket\n");
			return 0;
		}

		/* Bind the socket to any valid IP address and a specific port */
		memset((char *)&myaddr, 0, sizeof(myaddr));
		myaddr.sin_family = AF_INET; // Address family used when set up the socket
		myaddr.sin_addr.s_addr = htonl(INADDR_ANY); // Address for socket
		myaddr.sin_port = htons(atoi(argv[1])); // Port number of socket
		
		if (bind(sockfd, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
			perror("bind failed");
			return 0;
		}

	ifr.ifr_addr.sa_family = AF_INET; //Type of address to retrieve
	strncpy(ifr.ifr_name,"eth0", IFNAMSIZ-1); //Copy the interface name in the ifreq structure
	ioctl(sockfd, SIOCGIFADDR, &ifr); //Get the IP address
	printf("Binding pada %s:%s\n", inet_ntoa(( (struct sockaddr_in *)&ifr.ifr_addr )->sin_addr), argv[1]);
	
	/* Initialize XON/XOFF flags */
	xon_active = true; // Transmitter can send any message now

	/* Create child process */
	pthread_t consumerbuff;
	pthread_create(&consumerbuff, NULL, &consumeBuff, NULL);

	/*** IF PARENT PROCESS ***/
	while (true) {
		//if (xon_active) {
			c = *(rcvchar(sockfd, rxq));
			//                     printf("ISI C: %c\n", c);
			/* Quit on end of file */
			if (c == Endfile) {
				while (rcvq.count>0) {

				}
				printf("End of file character has been received and the buffer is already empty.\n");
				exit(0);
			}
		//}
	}
}

void* consumeBuff(void *threadConsumer) {
	/*** CHILD PROCESS ***/
	int i = 1;
	while (true) {
		/* Call q_get */
		//printf("MULAI KONSUMSI\n");
		Byte *ch = q_get(rxq);
		if (ch != NULL && ((*ch>=32) || (*ch==CR) || (*ch==LF) || (*ch==Endfile))) { // Consume char 	
			printf("Mengkonsumsi byte ke-%d: '%c' - %d\n", i, *ch, *ch);
			++i;
			sleep(3);
		}
		/* Can introduce some delay here. */
	}
}

static Byte *rcvchar(int sockfd, QTYPE *queue) {
/* Read a character from socket and put it to the receive buffer. 
If the number of characters in the receive buffer is above certain 
level, then send XOFF and set a flag to notify transmitter. 
Return a pointer to the buffer where data is put. */
	Byte ch;
	unsigned int idx;
	//printf("Keadaan QUEUE: {front: %d, rear: %d, count: %d}\n", queue->front, queue->rear, queue->count);

		recvlen = recvfrom(sockfd, &ch, 1, 0, (struct sockaddr *)&remaddr, &addrlen);
		if (recvlen > 0) {
			msgcnt++;
			// Circular buffer handling
			queue->data[queue->rear++] = ch;
			queue->rear %= 8;
			queue->count++;
			//printf("%c\n", ch );
			ch = 0;
			printf("Menerima byte ke-%d\n", msgcnt);
			if (queue->count >= UPPERLIMIT && xon_active) {
				xon_active = false; // XOFF active
				xonxoff = XOFF;
				printf("Reached upper limit. Sending XOFF...\n");
				if (sendto(sockfd, &xonxoff, 1, 0, (struct sockaddr *)&remaddr, addrlen) < 0)
					perror("Failed send response");
			}
			
			//printf("	IDX %d\n", idx);
			//              printf("ISI QUEUE REAR-1 %c sedangkan REAR %c\n", queue->data[(queue->rear)-1], queue->data[queue->rear]);
			
		}
		else {
			printf("Something went wrong with receiver!\n");
		}
		idx = queue->rear-1;
		idx %= 8;
		return &queue->data[idx];

}

static Byte *q_get(QTYPE *queue) {
/* q_get returns a pointer to the buffer where data is read or NULL if 
buffer is empty. */

	Byte *current = NULL;
	//    printf("Keadaan QUEUE: {front: %d, rear: %d, count: %d}\n", queue->front, queue->rear, queue->count);


	//                     printf("Jumlah di queue %d\n", queue->count);
	/* Nothing in the queue */
	if (!queue->count) {
		//printf("QUEUE KOSONG\n");
	} 
	/* Retrieve data from buffer, save it to "current" 
	If the number of characters in the receive buffer is below certain 
	level, then send XON. Increment front index and check for wraparound. */
	else {
		//printf("QUEUE ADAAN\n");
		//printf("%c\n", queue->data[queue->front]);
		current = &queue->data[queue->front];
		//printf("Current: %d\n", *current);
		/* Circular buffer handling */
		queue->front++;
		queue->front %= 8;
		queue->count--;
		if(queue->count <= LOWERLIMIT && !xon_active) {
			/* Sending XON */
			xon_active = true;
			xonxoff = XON;
			printf("Found more room in buffer. Sending XON...\n");
			if (sendto(sockfd, &xonxoff, 1, 0, (struct sockaddr *)&remaddr, addrlen) < 0) {
				perror("Failed send response");
				exit(1);
			}
		}
	}
	return current;
}
	