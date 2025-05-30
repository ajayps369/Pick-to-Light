/*
 * File:   main.c
 * Author: AJAY PS
 * Date:   18/02/2024
 * Description: Pick to light 
 * Created on 3 February, 2025, 2:59 PM
 */

#include <xc.h>
#include <string.h>
#include "i2c.h"
#include "can.h"
#include "main.h"
#include "pic_to_light.h"
#include "ssd_display.h"
#include "digital_keypad.h"
#include "eeprom.h"
#include "external_interrupt.h"

State_t state;
static unsigned char ssd[MAX_SSD_CNT];
static unsigned char digit[] = {ZERO, ONE, TWO, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, NINE};
unsigned int key_detected, ssd_3, ssd_2, ssd_1, ssd_0, field = 0;
unsigned char key, once_stock = 1, stock_data[4], stock_address = 0x00;
unsigned char once_product = 1, product_data[4], product_address = 0x04;
unsigned char field_select, address, select = 0;
unsigned char received_stock[5], received_product[5], data_receive;

void init_config() {
    init_digital_keypad();
    init_ssd_control();
    init_i2c();
    init_external_interrupt();
    init_can();
    PEIE = 1;
    ADCON1 = 0x0F;
    GIE = 1;
    for (int i = 0; i < 4; i++) {
        write_internal_eeprom(stock_address + i, '0');
        write_internal_eeprom(product_address + i, '0');
    }
    can_transmit();
}

void main(void) {
    init_config();
    state = e_stock;

    while (1) {
        receive_data();
        if(data_receive){
            state = (strcpy(stock_data, received_stock) == 0) && (strcpy(product_data, received_product) == 0) ? e_stock : e_ssd_off;
            //state = (strcpy(product_data, received_product) == 0) ? e_product : e_ssd_off;
            data_receive = 0;
        }
            
        key = read_digital_keypad(STATE_CHANGE);

        if (key_detected) {
            switch (state) {
                case e_stock:

                    if (once_stock) {
                        ssd[3] = 0xCC; //t
                        ssd[2] = 0x6E; //S
                        ssd[1] = 0x40; //_
                        ssd[0] = 0xE5; //U
                        display(ssd);

                        if (key == SWITCH2) {
                            once_stock = 0;
                            read_stock();
                            state = e_modify_stock;
                        } else if (key == SWITCH3) {
                            state = e_product;
                        }
                    }
                    break;

                case e_product:
                    if (once_product) {
                        ssd[3] = 0xE9; //d
                        ssd[2] = 0x21; //I
                        ssd[1] = 0x40; //_
                        ssd[0] = 0x8F; //P
                        display(ssd);

                        if (key == SWITCH2) {
                            once_product = 0;
                            read_product();
                            state = e_modify_product;
                        } else if (key == SWITCH3) {
                            state = e_stock;
                        }
                    }
                    break;

                case e_modify_stock:
                    
                    update_ssd_display(stock_data);
                    if (key == SWITCH1) {
                        if (field == 0)
                            ssd_3 = (ssd_3 + 1) % 10;
                        else if (field == 1)
                            ssd_2 = (ssd_2 + 1) % 10;
                        else if (field == 2)
                            ssd_1 = (ssd_1 + 1) % 10;
                        else if (field == 3)
                            ssd_0 = (ssd_0 + 1) % 10;
                        stock_data[0] = ssd_0 + '0';
                        stock_data[1] = ssd_1 + '0';
                        stock_data[2] = ssd_2 + '0';
                        stock_data[3] = ssd_3 + '0';
                    } else if (key == SWITCH2) {
                        field = (field + 1) % 4;
                    } else if (key == SWITCH3) {
                        stock_data[0] = ssd_0 + '0';
                        stock_data[1] = ssd_1 + '0';
                        stock_data[2] = ssd_2 + '0';
                        stock_data[3] = ssd_3 + '0';
                        save_stock();
                        state = e_ssd_off;
                    }
                    break;

                case e_modify_product:

                    update_ssd_display(product_data);
                    if (key == SWITCH1) {
                        if (field == 0)
                            ssd_3 = (ssd_3 + 1) % 10;
                        else if (field == 1)
                            ssd_2 = (ssd_2 + 1) % 10;
                        else if (field == 2)
                            ssd_1 = (ssd_1 + 1) % 10;
                        else if (field == 3)
                            ssd_0 = (ssd_0 + 1) % 10;
                        product_data[0] = ssd_0 + '0';
                        product_data[1] = ssd_1 + '0';
                        product_data[2] = ssd_2 + '0';
                        product_data[3] = ssd_3 + '0';
                    } else if (key == SWITCH2) {
                        field = (field + 1) % 4;
                    } else if (key == SWITCH3) {
                        product_data[0] = ssd_0 + '0';
                        product_data[1] = ssd_1 + '0';
                        product_data[2] = ssd_2 + '0';
                        product_data[3] = ssd_3 + '0';
                        save_product();
                        state = e_ssd_off;
                    }
                    break;
                    
                case e_ssd_off: ssd_off();
                    key_detected = 0;
                    once_stock = 1;
                    once_product = 1;
                    state = e_stock;
                    break;
            }
        }
    }
}

void ssd_off(void) {
    ssd[3] = BLANK;
    ssd[2] = BLANK; 
    ssd[1] = BLANK; 
    ssd[0] = BLANK; 
    display(ssd);
}

void read_stock(void) {
    for (unsigned int i = 0; i < 4; i++) {
        stock_data[i] = read_internal_eeprom(stock_address + i);
    }
}

void read_product(void) {
    for (unsigned int i = 0; i < 4; i++) {
        product_data[i] = read_internal_eeprom(product_address + i);
    }
}

void update_ssd_display(unsigned char *data) {
    ssd_0 = data[0] - '0';
    ssd_1 = data[1] - '0';
    ssd_2 = data[2] - '0';
    ssd_3 = data[3] - '0';

    ssd[3] = digit[ssd_3] | (field == 0 ? DOT : BLANK);
    ssd[2] = digit[ssd_2] | (field == 1 ? DOT : BLANK);
    ssd[1] = digit[ssd_1] | (field == 2 ? DOT : BLANK);
    ssd[0] = digit[ssd_0] | (field == 3 ? DOT : BLANK);

    display(ssd);
}

void save_stock(void) {
    for (int i = 0; i < 4; i++) {
        write_internal_eeprom(stock_address + i, stock_data[i]);
    }
    can_transmit();
}

void save_product(void) {
    for (int i = 0; i < 4; i++) {
        write_internal_eeprom(product_address + i, product_data[i]);
    }
    can_transmit();
}

void receive_data(void) {
    unsigned char i = 0;
    if (can_receive()) {
        for (i = 0; i < 4; i++) {
            received_stock[i] = can_payload[D0 + i];
            received_product[i] = can_payload[D4 + i];
        }
        received_stock[i] = '\0';
        received_product[i] = '\0';
        data_receive = 1;
    }
}