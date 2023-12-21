#include "stdafx.h"
#include "i2c.h"
#ifdef WIN32
#include "CH341DLL_EN.H"
#else
#include "i2c-dev.h"
#endif

#ifdef WIN32
ULONG g_iIndex = 0;
ULONG g_iDevice = 0x4a;	// RTD2662 I2C Address
#else
// I2C Linux device handle
// I2C Linux device handle
int g_i2cFile;
#endif

// open the Linux device
bool InitI2C(int i2cport)
{
#ifdef WIN32
    // Open Device
    HANDLE h = CH341OpenDevice(g_iIndex);
	if (h == INVALID_HANDLE_VALUE) {
		return false;
	}

    // DLL verison
    ULONG dllVersion = CH341GetVersion();
    printf("DLL verison %d\n", dllVersion);

    // Driver version
    ULONG driverVersion = CH341GetDrvVersion();
    printf("Driver verison %d\n", driverVersion);

    // Device Name
    PVOID p = CH341GetDeviceName(g_iIndex);
    printf("Device Name %s\n", (PCHAR)p);

    // IC verison 0x10=CH341,0x20=CH341A,0x30=CH341A3
    ULONG icVersion = CH341GetVerIC(g_iIndex);
    printf("IC version %X\n", icVersion);

    // Reset Device
    BOOL b = CH341ResetDevice(g_iIndex);
    printf("Reset Device %d\n", b);
	if (!b) {
		return false;
	}

    // Set serial stream mode
    // B1-B0: I2C SCL freq. 00=20KHz,01=100KHz,10=400KHz,11=750KHz
    // B2:    SPI I/O mode, 0=D3 CLK/D5 OUT/D7 IMP, 1=D3 CLK/D5&D4 OUT/D7&D6 INP)
    // B7:    SPI MSB/LSB, 0=LSB, 1=MSB
    // ULONG iMode = 0; // SCL = 10KHz
    // ULONG iMode = 1; // SCL = 100KHz
    ULONG iMode = 2; // SCL = 400KHz
    // ULONG iMode = 3; // SCL = 750KHz
    b = CH341SetStream(g_iIndex, iMode);
    printf("Set Stream %d\n", b);
	return b==TRUE;
#else
    char dev[256];
    sprintf(dev, "/dev/i2c-%d", i2cport);
    g_i2cFile = open(dev, O_RDWR);
    if (g_i2cFile < 0)
    {
        perror("i2cOpen");
        return false;
    }
#endif
	return true;
}

// close the Linux device
void CloseI2C()
{
#ifdef WIN32
    CH341CloseDevice(g_iIndex);
#else
    close(g_i2cFile);
#endif
}

void SetI2CAddr(uint8_t address)
{
    printf("Working with device %02x\n",address);
#ifdef WIN32
	g_iDevice = address;
#else
    if (ioctl(g_i2cFile, I2C_SLAVE, address) < 0)
    {
        perror("i2cSetAddress");
        exit(1);
    }
#endif
}

bool WriteBytesToAddr(uint8_t reg, uint8_t* values, uint8_t len)
{
#ifdef WIN32
    // I2C Transfer
    ULONG iTmpWriteLength = len + 2;
    PUCHAR iTmpWriteBuffer = new UCHAR[iTmpWriteLength];

    memcpy(&iTmpWriteBuffer[2], values, len);
    iTmpWriteBuffer[0] = g_iDevice << 1; // SSD1306 I2C Address (But Need Shifted)
    iTmpWriteBuffer[1] = reg; // SSD1306 OLED Write Data
    BOOL b = CH341StreamI2C(g_iIndex, iTmpWriteLength, iTmpWriteBuffer, 0UL, NULL);
#ifdef _DEBUG
	for (int i=1; i<iTmpWriteLength; i++) {
		printf("0x%02X,0x%02X,Write\n", g_iDevice, iTmpWriteBuffer[i]);
	}
#endif

    delete[] iTmpWriteBuffer;
	return b==TRUE;
#else
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
#endif
}

bool ReadBytesFromAddr(uint8_t reg, uint8_t* dest, uint8_t len)
{
#ifdef WIN32
    // I2C Transfer
	uint8_t wr[2] = {g_iDevice<<1, reg};
    BOOL b = CH341StreamI2C(g_iIndex, 2, &wr[0], len, dest);
#ifdef _DEBUG
	for (int i=1; i<2; i++) {
		printf("0x%02X,0x%02X,Write\n", g_iDevice, wr[i]);
	}
	for (int i=0; i<len; i++) {
		printf("0x%02X,0x%02X,Read\n", g_iDevice, dest[i]);
	}
#endif

	return b==TRUE;
#else
    i2c_smbus_read_i2c_block_data(g_i2cFile,reg,len,dest);
#endif
}

uint8_t ReadReg(uint8_t reg)
{
#ifdef WIN32
	uint8_t	data;
#if 1
   //BOOL b = CH341ReadI2C(g_iIndex, g_iDevice, reg, &data);
   ReadBytesFromAddr(reg, &data, 1);
#else
	uint8_t wr[2] = {g_iDevice<<1, reg};
	uint8_t rd[1] = {0};
   BOOL b = CH341StreamI2C(g_iIndex, 2, &wr[0], 1, &rd[0]);
   data = rd[0];
#endif
   return data;
#else
    return i2c_smbus_read_byte_data(g_i2cFile,reg);
#endif
}

bool WriteReg(uint8_t reg, uint8_t value)
{
    //printf("Writing %02x to %02x\n",value,reg);
    return WriteBytesToAddr(reg, &value, 1);
}
