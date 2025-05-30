/*
 * File:   main.c
 * Author: AJAY PS
 * Date:   18/02/2024
 * Description: Pick to light 
 * Created on 3 February, 2025, 2:59 PM
 */
#include <xc.h>
#include "main.h"
#include "uart.h"
#include "can.h"

unsigned char ch;
unsigned char stock_data[5], product_data[5], data_received, once_stock = 1, once_product = 1;
unsigned int count, i = 0, j = 0, check_stock = 0, check_product = 0;

void init_config(void) {
    init_uart();
    getch();
    /* Enabling peripheral interrupt */
    PEIE = 1;
    /* Enabling global interrupt */
    GIE = 1;
    /* Initialize CAN module */
    init_can();
    puts("Press any key to continue\n\r");
}

void main() {
    init_config();

    while (1) {
        receive_data();
        if (ch != '\0') {
            if (data_received) {
                puts("\n\r-----------------------\n\r");
                puts("\n\r     Received Data     \n\r");
                puts("\n\r-----------------------\n\r");
                puts("Stock Count\t");
                puts(stock_data);
                puts("\n\rProduct Count\t");
                puts(product_data);
                puts("\n\r-----------------------\n\r");
                data_received = 0;
            }
            if (ch == '\r' && count == 0) {
                if (once_stock) {
                    puts("\n\n\rEnter New Stock data\n\r");
                    once_stock = 0;
                }
                check_stock = 1;
            }
            if (check_stock == 1) {
                if (i < 4) {
                    ch = getche();
                    stock_data[i] = ch;
                    putch(ch);
                    i++;
                }
                else {
                    stock_data[i] = '\0';
                    check_stock = 0;
                    count++;
                }
            }
        }
        else if (ch == '\r' && count == 1) {
            if (once_product) {
                puts("\n\n\rEnter New Product data\n\r");
                once_product = 0;
            }
            check_product = 1;
        }
        if (check_product == 1) {
            if (j < 4) {
                ch = getche();
                product_data[j] = ch;
                putch(ch);
                j++;
            }
            else {
                product_data[j] = '\0';
                check_product = 0;
                count++;
            }
        }
        else if (ch == '\r' && count == 2) {
            can_transmit();
            delay(1000);
            count = 0;
            once_stock = 1;
            once_product = 1;
            i = 0;
            j = 0;
            puts("\n\n\r  Transmission successful  ");
            puts("\n\r.............*****.............\n\r");
        }
        ch = '\0';
    }

}

/* delay 1ms function */
void delay(unsigned short factor) {
    unsigned short i, j;

    for (i = 0; i < factor; i++) {
        for (j = 500; j--;);
    }
}

void receive_data(void) {
    unsigned char i = 0;
    if (can_receive()) {
        for (i = 0; i < 4; i++) {
            stock_data[i] = can_payload[D0 + i];
            product_data[i] = can_payload[D4 + i];
        }
        stock_data[i] = '\0';
        product_data[i] = '\0';
        data_received = 1;
    }
}
