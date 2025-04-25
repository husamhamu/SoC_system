#include <stdio.h>

void i2c_peri_enable(void);

int i2c_peri_read(unsigned char device_addr, unsigned char reg_addr, unsigned char *data, int len);

int i2c_peri_write(unsigned char device_addr, unsigned char reg_addr, unsigned char *data, int len);
