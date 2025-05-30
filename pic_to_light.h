 #ifndef XC_HEADER_TEMPLATE_H
#define	XC_HEADER_TEMPLATE_H

#include <xc.h> 

typedef enum {
    e_stock,  e_product, e_configure, e_modify_product, e_modify_stock, e_ssd_off
} State_t;


extern State_t state; 

void read_stock(void);
void read_product(void);
void update_ssd_display(unsigned char *data);
void save_stock(void);
void save_product(void);
void ssd_off(void);
void receive_data(void);

#endif