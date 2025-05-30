#ifndef MAIN_H
#define MAIN_H

#include <xc.h>

#define _XTAL_FREQ 20000000

/* Defines the data */
#define TRUE			1
#define FALSE			0

void receive_data(void);
void delay(unsigned short factor);

#endif