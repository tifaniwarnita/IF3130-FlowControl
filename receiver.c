/*
	Tifani Warnita	13513055
	Asanilta Fahda	13513079

	File	: receiver.c
	Source	: Paul Krzyzanowski (https://www.cs.rutgers.edu/~pxk/417/notes/sockets/udp.html)
*/

#include <pthread.h>
#include "receiver.h"
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define DELAY 500 // Delay to adjust speed of consuming buffer
#define BUFFSIZE 8 // Define receiver buffer size
#define MAXLIMIT 7

int sockfd; //Socket declaration

Byte rxbuf[BUFFSIZE];
QTYPE rcvq = { 0, 0, 0, BUFFSIZE, rxbuf };
QTYPE *rxq = &rcvq;
Byte c; // Current
Byte sent_xonxoff = XON;
bool send_xon = false,
send_xoff = false;
int recvlen; // # bytes received
int msgcnt = 0;	// Count # of messages we received

/* Functions declaration */
static Byte *rcvchar(int socksockfd, QTYPE *queue);
static Byte *q_get(QTYPE *);
void* consumeBuff(void *);

int main(int argc, char **argv)
/* Argv and argc will be used to define port number */
{	
	struct sockaddr_in myaddr;	// Our address
	struct sockaddr_in remaddr;	// Remote address
	socklen_t addrlen = sizeof(remaddr); // Length of addresses
	Byte *c = NULL;

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

	printf("Binding pada \n");
	/* ifr.ifr_addr.sa_family = AF_INET;
	strncpy(ifr.ifr_name,"eth0", IFNAMSIZ-1);
	ioctl(fd, SIOCGIFADDR, &ifr);
	printf("%s: %d\n", inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr),port); */

	/* Initialize XON/XOFF flags */
	send_xon = true; // Transmitter can send any message now

	/* Create child process */
	pthread_t consumerbuff;
	pthread_create(&consumerbuff, NULL, consumeBuff, NULL);

	/*** IF PARENT PROCESS ***/
	/*while (true) {
		c = *(rcvchar(sockfd, rxq));
		/* Quit on end of file */
		/*if (c == Endfile) {
			exit(0);
		}
	}

	/* now loop, receiving data and printing what we received */
	/*for (;;) {
		printf("waiting on port %d\n", SERVICE_PORT);
		recvlen = recvfrom(sockfd, buf, BUFFSIZE, 0, (struct sockaddr *)&remaddr, &addrlen);
		if (recvlen > 0) {
			buf[recvlen] = 0;
			printf("received message: \"%s\" (%d bytes)\n", buf, recvlen);
		}
		else
			printf("uh oh - something went wrong!\n");
		sprintf(buf, "ack %d", msgcnt++);
		printf("sending response \"%s\"\n", buf);
		if (sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr *)&remaddr, addrlen) < 0)
			perror("sendto");
	}
	/* never exits */
}

void* consumeBuff(void *threadConsumer) {
	/*** CHILD PROCESS ***/
	while (true) {
		/* Call q_get */
		Byte *ch = q_get(rxq);
		if (ch != NULL) { // Consume char
			printf("Mengkonsumsi byte ke-%d: '%c'\n", *ch, rcvq->data[*ch]);
		}
		/* Can introduce some delay here. */
		sleep(DELAY);
	}
}

static Byte *rcvchar(int sockfd, QTYPE *queue) {
/* Read a character from socket and put it to the receive buffer. 
If the number of characters in the receive buffer is above certain 
level, then send XOFF and set a flag to notify transmitter. 
Return a pointer to the buffer where data is put. */
	recvlen = recvfrom(sockfd, rcvq, BUFFSIZE, 0, (struct sockaddr *)&remaddr, &addrlen);
	if (recvlen > 0) {
			rcvq[recvlen] = 0;
			printf("Menerima byte ke-\n", recvlen);
		}
		else
			printf("uh oh - something went wrong!\n");
		sprintf(buf, "ack %d", msgcnt++);
		printf("sending response \"%s\"\n", buf);
		if (sendto(sockfd, rcvq, strlen(rcvq), 0, (struct sockaddr *)&remaddr, addrlen) < 0)
			perror("sendto");
	}
}

static Byte *q_get(QTYPE *queue) {
/* q_get returns a pointer to the buffer where data is read or NULL if 
buffer is empty. */

	Byte *current = NULL;

	/* Nothing in the queue */
	if (!queue->count) {
		return (NULL);
	} 
	/* Retrieve data from buffer, save it to "current" and "data" 
	If the number of characters in the receive buffer is below certain 
	level, then send XON. Increment front index and check for wraparound. */
	else {
		*current = queue->data[queue->count];
		/* Circular buffer handling */
		if(queue->front == 7)
			queue->front = 0;
		else
			queue->front++;
		queue->count--;
		if(queue->count == MAXLIMIT && sent_xonxoff == XOFF) {
			/* Sending XON */
			sent_xonxoff = XON;
			send_xon = true,
			send_xoff = false;
			if (sendto(sockfd, &sent_xonxoff, 1, 0, (struct sockaddr *)&remaddr, slen)==-1) {
				perror("Error sending flag XON");
				exit(1);
			}
		}
	}
	return current;
}
	