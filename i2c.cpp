#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include "i2c.h"
#include "i2c-dev.h"

// I2C Linux device handle
// I2C Linux device handle
int g_i2cFile;

// open the Linux device
void InitI2C(int i2cport)
{
    char dev[256];
    sprintf(dev, "/dev/i2c-%d", i2cport);
    g_i2cFile = open(dev, O_RDWR);
    if (g_i2cFile < 0)
    {
        perror("i2cOpen");
        exit(1);
    }
}

// close the Linux device
void CloseI2C()
{
    close(g_i2cFile);
}

void SetI2CAddr(uint8_t address)
{
    printf("Working with device %02x\n",address);

    if (ioctl(g_i2cFile, I2C_SLAVE, address) < 0)
    {
        perror("i2cSetAddress");
        exit(1);
    }
}

bool WriteBytesToAddr(uint8_t reg, uint8_t* values, uint8_t len)
{

        uint8_t buf[64];
        if (len > 63)
        {
            len = 63;
        }
        uint8_t buflen = len + 1;
        buf[0] = reg;

        for(int idx = 1; idx < buflen; idx++)
        {
            buf[idx] = values[idx-1];
        }
        /*
            for(int idx = 0; idx < buflen; idx++)
            {
                printf("buf[%i] = %02x ",idx,buf[idx]);
            }
            printf("\n");
        */
        if (write(g_i2cFile, buf, buflen) != buflen)
        {
            printf("Failed to write to the i2c bus\n");
            return 0;
        }
        else
        {
            return 1;
        }


/*
    if (i2c_smbus_write_i2c_block_data(g_i2cFile, reg, len, values) != len)
    {
        printf("Failed to write to the i2c bus\n");
        return 0;
    }
    else
    {
        return 1;
    }
*/
}

bool ReadBytesFromAddr(uint8_t reg, uint8_t* dest, uint8_t len)
{

    i2c_smbus_read_i2c_block_data(g_i2cFile,reg,len,dest);

}

uint8_t ReadReg(uint8_t reg)
{
    return i2c_smbus_read_byte_data(g_i2cFile,reg);
}

bool WriteReg(uint8_t reg, uint8_t value)
{
    //printf("Writing %02x to %02x\n",value,reg);
    return WriteBytesToAddr(reg, &value, 1);
}
