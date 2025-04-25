#include <peripherals.h>

void spi_peri_enable(){
	// Enable the SPI master core
	SPI_MASTER.spi.spi_control |= SPI_MASTER_CTRL_EN;
	
	// Set the frame to 9 bits
	SPI_MASTER.spi.spi_control |= (9<<8);
	
	// Set the clock divider 60MHz/4*divider => divider 2 = 001 to have a clock of SPI less than max 10MHz
	SPI_MASTER.spi.spi_control |= (1<<13);
	
}

void spi_peri_select(){
	// Activate the specified slave
	SPI_MASTER.spi.spi_control |= (1<<4);
}

void spi_peri_deselect(){
	// Deactivate all slaves
	SPI_MASTER.spi.spi_control &= ~SPI_MASTER_CTRL_SLAVE;
}

void spi_peri_write(unsigned char message){

    // Check if the SPI master is ready to transmit
    while (SPI_MASTER.spi.spi_status & SPI_MASTER_STAT_FILL);

    // Write the data to be transmitted
    SPI_MASTER.spi.spi_data_out = (int)message;

}
