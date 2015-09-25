/* 
	Tifani Warnita	13513055
	Asanilta Fahda	13513079

	File	: transmitter.h
*/ 

#ifndef _TRANSMITTER_H_ 
#define _TRANSMITTER_H_ 

#define Endfile 26 /* End of file character */
#define CR 13 /* Carriage Return */
#define LF 10 /* Line Feed */ 

/* XON/XOFF protocol */ 
#define XON (0x11) 
#define XOFF (0x13) 

typedef unsigned char Byte;
void* receiveSignal(void*);

#endif