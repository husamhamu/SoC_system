#include <stdio.h>

#include <peripherals.h>

void i2c_peri_enable(void) {
    // Enable the I2C master core and interrupts
    I2C_MASTER.ctrl = I2C_CTRL_EN | I2C_CTRL_INTR_EN | 299;
    
}

int i2c_peri_read(unsigned char slave_addr, unsigned char reg_addr, unsigned char *data, int length) {
	
    // Send the slave address with write bit
    I2C_MASTER.data[0] = slave_addr;
    // Send the register address
    I2C_MASTER.data[1] = reg_addr;
    I2C_MASTER.cmd = I2C_CMD_STA | (2 << 3);
    while (I2C_MASTER.stat & I2C_STA_TIP);
    //while (I2C_MASTER.stat & I2C_STA_NO_ACK);
    //printf("i2c_peri_read: slave_adress reg_addr was sent\n");


    // Send repeated start condition and slave address with read bit
    I2C_MASTER.data[0] = slave_addr | 1;
    I2C_MASTER.cmd = I2C_CMD_STA | I2C_CMD_RD | I2C_CMD_STO |((length +1)<< 3);
    while (I2C_MASTER.stat & I2C_STA_TIP);
    
    // Read the data
    for (int i = 0; i < length; i++) {
        data[i] = I2C_MASTER.data[i + 1];
    }

    return 0;
}

int i2c_peri_write(unsigned char slave_addr, unsigned char reg_addr, unsigned char *data, int length) {
    if (length > 8) return -1; // Check for maximum data length

    // Send the slave address with write bit
    I2C_MASTER.data[0] = slave_addr;
    // Send the register address
    I2C_MASTER.data[1] = reg_addr;
    // Send the data
    I2C_MASTER.data[2] = *data;
    I2C_MASTER.cmd = I2C_CMD_STA | I2C_CMD_STO |(3 << 3);
    while (I2C_MASTER.stat & I2C_STA_TIP);
    return 0;
}
