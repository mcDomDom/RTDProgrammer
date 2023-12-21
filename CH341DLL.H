// 2004.05.28, 2004.10.20, 2005.01.08, 2005.03.25, 2005.04.28, 2005.07.18, 2005.07.28, 2005.09.19, 2007.12.19, 2008.10.15,2022.5
//****************************************
//**  Copyright  (C)  WCH   1999-2022   **
//**  Web:  http://www.winchiphead.com  **
//****************************************
//**  DLL for USB interface chip CH341  **
//**  C, VC5.0                          **
//****************************************
//
// USB���߽ӿ�оƬCH341����Ӧ�ò�ӿڿ� V2.2
// �Ͼ��ߺ�������޹�˾  ����: W.ch 2008.10
// CH341-DLL  V2.2
// ���л���: Windows 98/ME, Windows 2000/XP/7/10/11
// support USB chip: CH341, CH341A�� CH347 SPI/I2C
// USB => Parallel, I2C, SPI, ...
//

#ifndef		_CH341_DLL_H
#define		_CH341_DLL_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN64
#define		mOFFSET( s, m )			( (ULONG_PTR) & ( ( ( s * ) 0 ) -> m ) )	// �����ȡ�ṹ��Ա���ƫ�Ƶ�ַ�ĺ�
#else
#define		mOFFSET( s, m )			( (ULONG) & ( ( ( s * ) 0 ) -> m ) )	// �����ȡ�ṹ��Ա���ƫ�Ƶ�ַ�ĺ�
#endif

#ifndef		max
#define		max( a, b )				( ( ( a ) > ( b ) ) ? ( a ) : ( b ) )	// �ϴ�ֵ
#endif

#ifndef		min
#define		min( a, b )				( ( ( a ) < ( b ) ) ? ( a ) : ( b ) )	// ��Сֵ
#endif

#ifdef		ExAllocatePool
#undef		ExAllocatePool						// ɾ����TAG���ڴ����
#endif

#ifndef		NTSTATUS
typedef		LONG	NTSTATUS;					// ����״̬
#endif


typedef	struct	_USB_SETUP_PKT {				// USB���ƴ���Ľ����׶ε�����������ṹ
	UCHAR			mUspReqType;				// 00H ��������
	UCHAR			mUspRequest;				// 01H �������
	union	{
		struct	{
			UCHAR	mUspValueLow;				// 02H ֵ�������ֽ�
			UCHAR	mUspValueHigh;				// 03H ֵ�������ֽ�
		};
		USHORT		mUspValue;					// 02H-03H ֵ����
	};
	union	{
		struct	{
			UCHAR	mUspIndexLow;				// 04H �����������ֽ�
			UCHAR	mUspIndexHigh;				// 05H �����������ֽ�
		};
		USHORT		mUspIndex;					// 04H-05H ��������
	};
	USHORT			mLength;					// 06H-07H ���ݽ׶ε����ݳ���
} mUSB_SETUP_PKT, *mPUSB_SETUP_PKT;


#define		mCH341_PACKET_LENGTH	32			// CH341֧�ֵ����ݰ��ĳ���
#define		mCH341_PKT_LEN_SHORT	8			// CH341֧�ֵĶ����ݰ��ĳ���


typedef	struct	_WIN32_COMMAND {				// ����WIN32����ӿڽṹ
	union	{
		ULONG		mFunction;					// ����ʱָ�����ܴ�����߹ܵ���
		NTSTATUS	mStatus;					// ���ʱ���ز���״̬
	};
	ULONG			mLength;					// ��ȡ����,���غ������ݵĳ���
	union	{
		mUSB_SETUP_PKT	mSetupPkt;				// USB���ƴ���Ľ����׶ε���������
		UCHAR			mBuffer[ 512];      	// ���ݻ�����,����Ϊ0��255B
	};
} mWIN32_COMMAND, *mPWIN32_COMMAND;


// WIN32Ӧ�ò�ӿ�����
#define		IOCTL_CH341_COMMAND		( FILE_DEVICE_UNKNOWN << 16 | FILE_ANY_ACCESS << 14 | 0x0f34 << 2 | METHOD_BUFFERED )	// ר�ýӿ�

#define		mWIN32_COMMAND_HEAD		mOFFSET( mWIN32_COMMAND, mBuffer )	// WIN32����ӿڵ�ͷ����

#define		mCH341_MAX_NUMBER		16			// ���ͬʱ���ӵ�CH341��

#define		mMAX_BUFFER_LENGTH		0x1000		// ���ݻ�������󳤶�4096

#define		mMAX_COMMAND_LENGTH		( mWIN32_COMMAND_HEAD + mMAX_BUFFER_LENGTH )	// ������ݳ��ȼ�������ṹͷ�ĳ���

#define		mDEFAULT_BUFFER_LEN		0x0400		// ���ݻ�����Ĭ�ϳ���1024

#define		mDEFAULT_COMMAND_LEN	( mWIN32_COMMAND_HEAD + mDEFAULT_BUFFER_LEN )	// Ĭ�����ݳ��ȼ�������ṹͷ�ĳ���


// CH341�˵��ַ
#define		mCH341_ENDP_INTER_UP	0x81		// CH341���ж������ϴ��˵�ĵ�ַ
#define		mCH341_ENDP_INTER_DOWN	0x01		// CH341���ж������´��˵�ĵ�ַ
#define		mCH341_ENDP_DATA_UP		0x82		// CH341�����ݿ��ϴ��˵�ĵ�ַ
#define		mCH341_ENDP_DATA_DOWN	0x02		// CH341�����ݿ��´��˵�ĵ�ַ


// �豸��ӿ��ṩ�Ĺܵ���������
#define		mPipeDeviceCtrl			0x00000004	// CH341���ۺϿ��ƹܵ�
#define		mPipeInterUp			0x00000005	// CH341���ж������ϴ��ܵ�
#define		mPipeDataUp				0x00000006	// CH341�����ݿ��ϴ��ܵ�
#define		mPipeDataDown			0x00000007	// CH341�����ݿ��´��ܵ�

// Ӧ�ò�ӿڵĹ��ܴ���
#define		mFuncNoOperation		0x00000000	// �޲���
#define		mFuncGetVersion			0x00000001	// ��ȡ��������汾��
#define		mFuncGetConfig			0x00000002	// ��ȡUSB�豸����������
#define		mFuncSetTimeout			0x00000009	// ����USBͨѶ��ʱ
#define		mFuncSetExclusive		0x0000000b	// ���ö�ռʹ��
#define		mFuncResetDevice		0x0000000c	// ��λUSB�豸
#define		mFuncResetPipe			0x0000000d	// ��λUSB�ܵ�
#define		mFuncAbortPipe			0x0000000e	// ȡ��USB�ܵ�����������

// CH341����ר�õĹ��ܴ���
#define		mFuncSetParaMode		0x0000000f	// ���ò���ģʽ
#define		mFuncReadData0			0x00000010	// �Ӳ��ڶ�ȡ���ݿ�0
#define		mFuncReadData1			0x00000011	// �Ӳ��ڶ�ȡ���ݿ�1
#define		mFuncWriteData0			0x00000012	// �򲢿�д�����ݿ�0
#define		mFuncWriteData1			0x00000013	// �򲢿�д�����ݿ�1
#define		mFuncWriteRead			0x00000014	// �����������
#define		mFuncBufferMode			0x00000020	// �趨�����ϴ�ģʽ����ѯ�������е����ݳ���
#define		mFuncBufferModeDn		0x00000021	// �趨�����´�ģʽ����ѯ�������е����ݳ���
#define		mFuncGetVersionEx		0x00000022	// ��ȡ��������汾�ż�оƬ�ͺ�


// USB�豸��׼�������
#define		mUSB_CLR_FEATURE		0x01
#define		mUSB_SET_FEATURE		0x03
#define		mUSB_GET_STATUS			0x00
#define		mUSB_SET_ADDRESS		0x05
#define		mUSB_GET_DESCR			0x06
#define		mUSB_SET_DESCR			0x07
#define		mUSB_GET_CONFIG			0x08
#define		mUSB_SET_CONFIG			0x09
#define		mUSB_GET_INTERF			0x0a
#define		mUSB_SET_INTERF			0x0b
#define		mUSB_SYNC_FRAME			0x0c

// CH341���ƴ���ĳ���ר����������
#define		mCH341_VENDOR_READ		0xC0		// ͨ�����ƴ���ʵ�ֵ�CH341����ר�ö�����
#define		mCH341_VENDOR_WRITE		0x40		// ͨ�����ƴ���ʵ�ֵ�CH341����ר��д����

// CH341���ƴ���ĳ���ר���������
#define		mCH341_PARA_INIT		0xB1		// ��ʼ������
#define		mCH341_I2C_STATUS		0x52		// ��ȡI2C�ӿڵ�״̬
#define		mCH341_I2C_COMMAND		0x53		// ����I2C�ӿڵ�����

// CH341���ڲ����������
#define		mCH341_PARA_CMD_R0		0xAC		// �Ӳ��ڶ�����0,���ֽ�Ϊ����
#define		mCH341_PARA_CMD_R1		0xAD		// �Ӳ��ڶ�����1,���ֽ�Ϊ����
#define		mCH341_PARA_CMD_W0		0xA6		// �򲢿�д����0,�Ӵ��ֽڿ�ʼΪ������
#define		mCH341_PARA_CMD_W1		0xA7		// �򲢿�д����1,�Ӵ��ֽڿ�ʼΪ������
#define		mCH341_PARA_CMD_STS		0xA0		// ��ȡ����״̬

// CH341A���ڲ����������
#define		mCH341A_CMD_SET_OUTPUT	0xA1		// ���ò������
#define		mCH341A_CMD_IO_ADDR		0xA2		// MEM����ַ��д/�������,�Ӵ��ֽڿ�ʼΪ������
#define		mCH341A_CMD_PRINT_OUT	0xA3		// PRINT���ݴ�ӡ��ʽ���,�Ӵ��ֽڿ�ʼΪ������
#define		mCH341A_CMD_PWM_OUT		0xA4		// PWM��������������,�Ӵ��ֽڿ�ʼΪ������
#define		mCH341A_CMD_SHORT_PKT	0xA5		// �̰�,���ֽ��Ǹ����������������,�ٴ��ֽڼ�֮����ֽ���ԭ�����
#define		mCH341A_CMD_SPI_STREAM	0xA8		// SPI�ӿڵ������,�Ӵ��ֽڿ�ʼΪ������
//#define		mCH341A_CMD_SIO_STREAM	0xA9		// SIO�ӿڵ������,�Ӵ��ֽڿ�ʼΪ������
#define		mCH341A_CMD_I2C_STREAM	0xAA		// I2C�ӿڵ������,�Ӵ��ֽڿ�ʼΪI2C������
#define		mCH341A_CMD_UIO_STREAM	0xAB		// UIO�ӿڵ������,�Ӵ��ֽڿ�ʼΪ������
#define		mCH341A_CMD_PIO_STREAM	0xAE		// PIO�ӿڵ������,�Ӵ��ֽڿ�ʼΪ������

// CH341A���ƴ���ĳ���ר���������
#define		mCH341A_BUF_CLEAR		0xB2		// ���δ��ɵ�����
#define		mCH341A_I2C_CMD_X		0x54		// ����I2C�ӿڵ�����,����ִ��
#define		mCH341A_DELAY_MS		0x5E		// ������Ϊ��λ��ʱָ��ʱ��
#define		mCH341A_GET_VER			0x5F		// ��ȡоƬ�汾

#define		mCH341_EPP_IO_MAX		( mCH341_PACKET_LENGTH - 1 )	// CH341��EPP/MEM��ʽ�µ��ζ�д���ݿ����󳤶�
#define		mCH341A_EPP_IO_MAX		0xFF		// CH341A��EPP/MEM��ʽ�µ��ζ�д���ݿ����󳤶�

#define		mCH341A_CMD_IO_ADDR_W	0x00		// MEM����ַ��д/���������������:д����,λ6-λ0Ϊ��ַ,��һ���ֽ�Ϊ��д����
#define		mCH341A_CMD_IO_ADDR_R	0x80		// MEM����ַ��д/���������������:������,λ6-λ0Ϊ��ַ,��������һ�𷵻�

#define		mCH341A_CMD_I2C_STM_STA	0x74		// I2C�ӿڵ�������:������ʼλ
#define		mCH341A_CMD_I2C_STM_STO	0x75		// I2C�ӿڵ�������:����ֹͣλ
#define		mCH341A_CMD_I2C_STM_OUT	0x80		// I2C�ӿڵ�������:�������,λ5-λ0Ϊ����,�����ֽ�Ϊ����,0������ֻ����һ���ֽڲ�����Ӧ��
#define		mCH341A_CMD_I2C_STM_IN	0xC0		// I2C�ӿڵ�������:��������,λ5-λ0Ϊ����,0������ֻ����һ���ֽڲ�������Ӧ��
#define		mCH341A_CMD_I2C_STM_MAX	( min( 0x3F, mCH341_PACKET_LENGTH ) )	// I2C�ӿڵ�������������������������ݵ���󳤶�
#define		mCH341A_CMD_I2C_STM_SET	0x60		// I2C�ӿڵ�������:���ò���,λ2=SPI��I/O��(0=���뵥��,1=˫��˫��),λ1λ0=I2C�ٶ�(00=����,01=��׼,10=����,11=����)
#define		mCH341A_CMD_I2C_STM_US	0x40		// I2C�ӿڵ�������:��΢��Ϊ��λ��ʱ,λ3-λ0Ϊ��ʱֵ
#define		mCH341A_CMD_I2C_STM_MS	0x50		// I2C�ӿڵ�������:������Ϊ��λ��ʱ,λ3-λ0Ϊ��ʱֵ
#define		mCH341A_CMD_I2C_STM_DLY	0x0F		// I2C�ӿڵ�����������������ʱ�����ֵ
#define		mCH341A_CMD_I2C_STM_END	0x00		// I2C�ӿڵ�������:�������ǰ����

#define		mCH341A_CMD_UIO_STM_IN	0x00		// UIO�ӿڵ�������:��������D7-D0
#define		mCH341A_CMD_UIO_STM_DIR	0x40		// UIO�ӿڵ�������:�趨I/O����D5-D0,λ5-λ0Ϊ��������
#define		mCH341A_CMD_UIO_STM_OUT	0x80		// UIO�ӿڵ�������:�������D5-D0,λ5-λ0Ϊ����
#define		mCH341A_CMD_UIO_STM_US	0xC0		// UIO�ӿڵ�������:��΢��Ϊ��λ��ʱ,λ5-λ0Ϊ��ʱֵ
#define		mCH341A_CMD_UIO_STM_END	0x20		// UIO�ӿڵ�������:�������ǰ����


// CH341���ڹ���ģʽ
#define		mCH341_PARA_MODE_EPP	0x00		// CH341���ڹ���ģʽΪEPP��ʽ
#define		mCH341_PARA_MODE_EPP17	0x00		// CH341A���ڹ���ģʽΪEPP��ʽV1.7
#define		mCH341_PARA_MODE_EPP19	0x01		// CH341A���ڹ���ģʽΪEPP��ʽV1.9
#define		mCH341_PARA_MODE_MEM	0x02		// CH341���ڹ���ģʽΪMEM��ʽ
#define		mCH341_PARA_MODE_ECP	0x03		// CH341A���ڹ���ģʽΪECP��ʽ


// I/O��������λ����,ֱ�������״̬�źŵ�λ����,ֱ�������λ���ݶ���
#define		mStateBitERR			0x00000100	// ֻ����д,ERR#��������״̬,1:�ߵ�ƽ,0:�͵�ƽ
#define		mStateBitPEMP			0x00000200	// ֻ����д,PEMP��������״̬,1:�ߵ�ƽ,0:�͵�ƽ
#define		mStateBitINT			0x00000400	// ֻ����д,INT#��������״̬,1:�ߵ�ƽ,0:�͵�ƽ
#define		mStateBitSLCT			0x00000800	// ֻ����д,SLCT��������״̬,1:�ߵ�ƽ,0:�͵�ƽ
#define		mStateBitWAIT			0x00002000	// ֻ����д,WAIT#��������״̬,1:�ߵ�ƽ,0:�͵�ƽ
#define		mStateBitDATAS			0x00004000	// ֻд�ɶ�,DATAS#/READ#��������״̬,1:�ߵ�ƽ,0:�͵�ƽ
#define		mStateBitADDRS			0x00008000	// ֻд�ɶ�,ADDRS#/ADDR/ALE��������״̬,1:�ߵ�ƽ,0:�͵�ƽ
#define		mStateBitRESET			0x00010000	// ֻд,RESET#��������״̬,1:�ߵ�ƽ,0:�͵�ƽ
#define		mStateBitWRITE			0x00020000	// ֻд,WRITE#��������״̬,1:�ߵ�ƽ,0:�͵�ƽ
#define		mStateBitSCL			0x00400000	// ֻ��,SCL��������״̬,1:�ߵ�ƽ,0:�͵�ƽ
#define		mStateBitSDA			0x00800000	// ֻ��,SDA��������״̬,1:�ߵ�ƽ,0:�͵�ƽ


#define		MAX_DEVICE_PATH_SIZE	128			// �豸���Ƶ�����ַ���
#define		MAX_DEVICE_ID_SIZE		64			// �豸ID������ַ���


typedef		VOID	( CALLBACK	* mPCH341_INT_ROUTINE ) (  // �жϷ������
	ULONG			iStatus );  // �ж�״̬����,�ο������λ˵��
// λ7-λ0��ӦCH341��D7-D0����
// λ8��ӦCH341��ERR#����, λ9��ӦCH341��PEMP����, λ10��ӦCH341��INT#����, λ11��ӦCH341��SLCT����


HANDLE	WINAPI	CH341OpenDevice(  // ��CH341�豸,���ؾ��,��������Ч
	ULONG			iIndex );  // ָ��CH341�豸���,0��Ӧ��һ���豸


VOID	WINAPI	CH341CloseDevice(  // �ر�CH341�豸
	ULONG			iIndex );  // ָ��CH341�豸���


ULONG	WINAPI	CH341GetVersion( );  // ���DLL�汾��,���ذ汾��


ULONG	WINAPI	CH341DriverCommand(  // ֱ�Ӵ����������������,�����򷵻�0,���򷵻����ݳ���
	ULONG			iIndex,  // ָ��CH341�豸���,V1.6����DLLҲ�������豸�򿪺�ľ��
	mPWIN32_COMMAND	ioCommand );  // ����ṹ��ָ��
// �ó����ڵ��ú󷵻����ݳ���,������Ȼ��������ṹ,����Ƕ�����,�����ݷ���������ṹ��,
// ���ص����ݳ����ڲ���ʧ��ʱΪ0,�����ɹ�ʱΪ��������ṹ�ĳ���,�����һ���ֽ�,�򷵻�mWIN32_COMMAND_HEAD+1,
// ����ṹ�ڵ���ǰ,�ֱ��ṩ:�ܵ��Ż�������ܴ���,��ȡ���ݵĳ���(��ѡ),����(��ѡ)
// ����ṹ�ڵ��ú�,�ֱ𷵻�:����״̬����,�������ݵĳ���(��ѡ),
//   ����״̬��������WINDOWS����Ĵ���,���Բο�NTSTATUS.H,
//   �������ݵĳ�����ָ���������ص����ݳ���,���ݴ�������Ļ�������,����д����һ��Ϊ0


ULONG	WINAPI	CH341GetDrvVersion( );  // �����������汾��,���ذ汾��,�����򷵻�0


BOOL	WINAPI	CH341ResetDevice(  // ��λUSB�豸
	ULONG			iIndex );  // ָ��CH341�豸���


BOOL	WINAPI	CH341GetDeviceDescr(  // ��ȡ�豸������
	ULONG			iIndex,  // ָ��CH341�豸���
	PVOID			oBuffer,  // ָ��һ���㹻��Ļ�����,���ڱ���������
	PULONG			ioLength );  // ָ�򳤶ȵ�Ԫ,����ʱΪ׼����ȡ�ĳ���,���غ�Ϊʵ�ʶ�ȡ�ĳ���


BOOL	WINAPI	CH341GetConfigDescr(  // ��ȡ����������
	ULONG			iIndex,  // ָ��CH341�豸���
	PVOID			oBuffer,  // ָ��һ���㹻��Ļ�����,���ڱ���������
	PULONG			ioLength );  // ָ�򳤶ȵ�Ԫ,����ʱΪ׼����ȡ�ĳ���,���غ�Ϊʵ�ʶ�ȡ�ĳ���


BOOL	WINAPI	CH341SetIntRoutine(  // �趨�жϷ������
	ULONG			iIndex,  // ָ��CH341�豸���
	mPCH341_INT_ROUTINE	iIntRoutine );  // ָ���жϷ������,ΪNULL��ȡ���жϷ���,�������ж�ʱ���øó���


BOOL	WINAPI	CH341ReadInter(  // ��ȡ�ж�����
	ULONG			iIndex,  // ָ��CH341�豸���
	PULONG			iStatus );  // ָ��һ��˫�ֵ�Ԫ,���ڱ����ȡ���ж�״̬����,������
// λ7-λ0��ӦCH341��D7-D0����
// λ8��ӦCH341��ERR#����, λ9��ӦCH341��PEMP����, λ10��ӦCH341��INT#����, λ11��ӦCH341��SLCT����


BOOL	WINAPI	CH341AbortInter(  // �����ж����ݶ�����
	ULONG			iIndex );  // ָ��CH341�豸���


BOOL	WINAPI	CH341SetParaMode(  // ���ò���ģʽ
	ULONG			iIndex,  // ָ��CH341�豸���
	ULONG			iMode );  // ָ������ģʽ: 0ΪEPPģʽ/EPPģʽV1.7, 1ΪEPPģʽV1.9, 2ΪMEMģʽ


BOOL	WINAPI	CH341InitParallel(  // ��λ����ʼ������,RST#����͵�ƽ����
	ULONG			iIndex,  // ָ��CH341�豸���
	ULONG			iMode );  // ָ������ģʽ: 0ΪEPPģʽ/EPPģʽV1.7, 1ΪEPPģʽV1.9, 2ΪMEMģʽ, >= 0x00000100 ���ֵ�ǰģʽ


BOOL	WINAPI	CH341ReadData0(  // ��0#�˿ڶ�ȡ���ݿ�
	ULONG			iIndex,  // ָ��CH341�豸���
	PVOID			oBuffer,  // ָ��һ���㹻��Ļ�����,���ڱ����ȡ������
	PULONG			ioLength );  // ָ�򳤶ȵ�Ԫ,����ʱΪ׼����ȡ�ĳ���,���غ�Ϊʵ�ʶ�ȡ�ĳ���


BOOL	WINAPI	CH341ReadData1(  // ��1#�˿ڶ�ȡ���ݿ�
	ULONG			iIndex,  // ָ��CH341�豸���
	PVOID			oBuffer,  // ָ��һ���㹻��Ļ�����,���ڱ����ȡ������
	PULONG			ioLength );  // ָ�򳤶ȵ�Ԫ,����ʱΪ׼����ȡ�ĳ���,���غ�Ϊʵ�ʶ�ȡ�ĳ���


BOOL	WINAPI	CH341AbortRead(  // �������ݿ������
	ULONG			iIndex );  // ָ��CH341�豸���


BOOL	WINAPI	CH341WriteData0(  // ��0#�˿�д�����ݿ�
	ULONG			iIndex,  // ָ��CH341�豸���
	PVOID			iBuffer,  // ָ��һ��������,����׼��д��������
	PULONG			ioLength );  // ָ�򳤶ȵ�Ԫ,����ʱΪ׼��д���ĳ���,���غ�Ϊʵ��д���ĳ���


BOOL	WINAPI	CH341WriteData1(  // ��1#�˿�д�����ݿ�
	ULONG			iIndex,  // ָ��CH341�豸���
	PVOID			iBuffer,  // ָ��һ��������,����׼��д��������
	PULONG			ioLength );  // ָ�򳤶ȵ�Ԫ,����ʱΪ׼��д���ĳ���,���غ�Ϊʵ��д���ĳ���


BOOL	WINAPI	CH341AbortWrite(  // �������ݿ�д����
	ULONG			iIndex );  // ָ��CH341�豸���


BOOL	WINAPI	CH341GetStatus(  // ͨ��CH341ֱ���������ݺ�״̬
	ULONG			iIndex,  // ָ��CH341�豸���
	PULONG			iStatus );  // ָ��һ��˫�ֵ�Ԫ,���ڱ���״̬����,�ο������λ˵��
// λ7-λ0��ӦCH341��D7-D0����
// λ8��ӦCH341��ERR#����, λ9��ӦCH341��PEMP����, λ10��ӦCH341��INT#����, λ11��ӦCH341��SLCT����, λ23��ӦCH341��SDA����
// λ13��ӦCH341��BUSY/WAIT#����, λ14��ӦCH341��AUTOFD#/DATAS#����,λ15��ӦCH341��SLCTIN#/ADDRS#����


BOOL	WINAPI	CH341ReadI2C(  // ��I2C�ӿڶ�ȡһ���ֽ�����
	ULONG			iIndex,  // ָ��CH341�豸���
	UCHAR			iDevice,  // ��7λָ��I2C�豸��ַ
	UCHAR			iAddr,  // ָ�����ݵ�Ԫ�ĵ�ַ
	PUCHAR			oByte );  // ָ��һ���ֽڵ�Ԫ,���ڱ����ȡ���ֽ�����


BOOL	WINAPI	CH341WriteI2C(  // ��I2C�ӿ�д��һ���ֽ�����
	ULONG			iIndex,  // ָ��CH341�豸���
	UCHAR			iDevice,  // ��7λָ��I2C�豸��ַ
	UCHAR			iAddr,  // ָ�����ݵ�Ԫ�ĵ�ַ
	UCHAR			iByte );  // ��д����ֽ�����


BOOL	WINAPI	CH341EppReadData(  // EPP��ʽ������: WR#=1, DS#=0, AS#=1, D0-D7=input
	ULONG			iIndex,  // ָ��CH341�豸���
	PVOID			oBuffer,  // ָ��һ���㹻��Ļ�����,���ڱ����ȡ������
	PULONG			ioLength );  // ָ�򳤶ȵ�Ԫ,����ʱΪ׼����ȡ�ĳ���,���غ�Ϊʵ�ʶ�ȡ�ĳ���


BOOL	WINAPI	CH341EppReadAddr(  // EPP��ʽ����ַ: WR#=1, DS#=1, AS#=0, D0-D7=input
	ULONG			iIndex,  // ָ��CH341�豸���
	PVOID			oBuffer,  // ָ��һ���㹻��Ļ�����,���ڱ����ȡ�ĵ�ַ����
	PULONG			ioLength );  // ָ�򳤶ȵ�Ԫ,����ʱΪ׼����ȡ�ĳ���,���غ�Ϊʵ�ʶ�ȡ�ĳ���


BOOL	WINAPI	CH341EppWriteData(  // EPP��ʽд����: WR#=0, DS#=0, AS#=1, D0-D7=output
	ULONG			iIndex,  // ָ��CH341�豸���
	PVOID			iBuffer,  // ָ��һ��������,����׼��д��������
	PULONG			ioLength );  // ָ�򳤶ȵ�Ԫ,����ʱΪ׼��д���ĳ���,���غ�Ϊʵ��д���ĳ���


BOOL	WINAPI	CH341EppWriteAddr(  // EPP��ʽд��ַ: WR#=0, DS#=1, AS#=0, D0-D7=output
	ULONG			iIndex,  // ָ��CH341�豸���
	PVOID			iBuffer,  // ָ��һ��������,����׼��д���ĵ�ַ����
	PULONG			ioLength );  // ָ�򳤶ȵ�Ԫ,����ʱΪ׼��д���ĳ���,���غ�Ϊʵ��д���ĳ���


BOOL	WINAPI	CH341EppSetAddr(  // EPP��ʽ���õ�ַ: WR#=0, DS#=1, AS#=0, D0-D7=output
	ULONG			iIndex,  // ָ��CH341�豸���
	UCHAR			iAddr );  // ָ��EPP��ַ


BOOL	WINAPI	CH341MemReadAddr0(  // MEM��ʽ����ַ0: WR#=1, DS#/RD#=0, AS#/ADDR=0, D0-D7=input
	ULONG			iIndex,  // ָ��CH341�豸���
	PVOID			oBuffer,  // ָ��һ���㹻��Ļ�����,���ڱ���ӵ�ַ0��ȡ������
	PULONG			ioLength );  // ָ�򳤶ȵ�Ԫ,����ʱΪ׼����ȡ�ĳ���,���غ�Ϊʵ�ʶ�ȡ�ĳ���


BOOL	WINAPI	CH341MemReadAddr1(  // MEM��ʽ����ַ1: WR#=1, DS#/RD#=0, AS#/ADDR=1, D0-D7=input
	ULONG			iIndex,  // ָ��CH341�豸���
	PVOID			oBuffer,  // ָ��һ���㹻��Ļ�����,���ڱ���ӵ�ַ1��ȡ������
	PULONG			ioLength );  // ָ�򳤶ȵ�Ԫ,����ʱΪ׼����ȡ�ĳ���,���غ�Ϊʵ�ʶ�ȡ�ĳ���


BOOL	WINAPI	CH341MemWriteAddr0(  // MEM��ʽд��ַ0: WR#=0, DS#/RD#=1, AS#/ADDR=0, D0-D7=output
	ULONG			iIndex,  // ָ��CH341�豸���
	PVOID			iBuffer,  // ָ��һ��������,����׼�����ַ0д��������
	PULONG			ioLength );  // ָ�򳤶ȵ�Ԫ,����ʱΪ׼��д���ĳ���,���غ�Ϊʵ��д���ĳ���


BOOL	WINAPI	CH341MemWriteAddr1(  // MEM��ʽд��ַ1: WR#=0, DS#/RD#=1, AS#/ADDR=1, D0-D7=output
	ULONG			iIndex,  // ָ��CH341�豸���
	PVOID			iBuffer,  // ָ��һ��������,����׼�����ַ1д��������
	PULONG			ioLength );  // ָ�򳤶ȵ�Ԫ,����ʱΪ׼��д���ĳ���,���غ�Ϊʵ��д���ĳ���


BOOL	WINAPI	CH341SetExclusive(  // ���ö�ռʹ�õ�ǰCH341�豸
	ULONG			iIndex,  // ָ��CH341�豸���
	ULONG			iExclusive );  // Ϊ0���豸���Թ���ʹ��,��0���ռʹ��


BOOL	WINAPI	CH341SetTimeout(  // ����USB���ݶ�д�ĳ�ʱ
	ULONG			iIndex,  // ָ��CH341�豸���
	ULONG			iWriteTimeout,  // ָ��USBд�����ݿ�ĳ�ʱʱ��,�Ժ���mSΪ��λ,0xFFFFFFFFָ������ʱ(Ĭ��ֵ)
	ULONG			iReadTimeout );  // ָ��USB��ȡ���ݿ�ĳ�ʱʱ��,�Ժ���mSΪ��λ,0xFFFFFFFFָ������ʱ(Ĭ��ֵ)


BOOL	WINAPI	CH341ReadData(  // ��ȡ���ݿ�
	ULONG			iIndex,  // ָ��CH341�豸���
	PVOID			oBuffer,  // ָ��һ���㹻��Ļ�����,���ڱ����ȡ������
	PULONG			ioLength );  // ָ�򳤶ȵ�Ԫ,����ʱΪ׼����ȡ�ĳ���,���غ�Ϊʵ�ʶ�ȡ�ĳ���


BOOL	WINAPI	CH341WriteData(  // д�����ݿ�
	ULONG			iIndex,  // ָ��CH341�豸���
	PVOID			iBuffer,  // ָ��һ��������,����׼��д��������
	PULONG			ioLength );  // ָ�򳤶ȵ�Ԫ,����ʱΪ׼��д���ĳ���,���غ�Ϊʵ��д���ĳ���


PVOID	WINAPI	CH341GetDeviceName(  // ����ָ��CH341�豸���ƵĻ�����,�����򷵻�NULL
	ULONG			iIndex );  // ָ��CH341�豸���,0��Ӧ��һ���豸


ULONG	WINAPI	CH341GetVerIC(  // ��ȡCH341оƬ�İ汾,����:0=�豸��Ч,0x10=CH341,0x20=CH341A
	ULONG			iIndex );  // ָ��CH341�豸���
#define		IC_VER_CH341A		0x20
#define		IC_VER_CH341A3		0x30


BOOL	WINAPI	CH341FlushBuffer(  // ���CH341�Ļ�����
	ULONG			iIndex );  // ָ��CH341�豸���


BOOL	WINAPI	CH341WriteRead(  // ִ������������,�����������
	ULONG			iIndex,  // ָ��CH341�豸���
	ULONG			iWriteLength,  // д����,׼��д���ĳ���
	PVOID			iWriteBuffer,  // ָ��һ��������,����׼��д��������
	ULONG			iReadStep,  // ׼����ȡ�ĵ�����ĳ���, ׼����ȡ���ܳ���Ϊ(iReadStep*iReadTimes)
	ULONG			iReadTimes,  // ׼����ȡ�Ĵ���
	PULONG			oReadLength,  // ָ�򳤶ȵ�Ԫ,���غ�Ϊʵ�ʶ�ȡ�ĳ���
	PVOID			oReadBuffer );  // ָ��һ���㹻��Ļ�����,���ڱ����ȡ������


BOOL	WINAPI	CH341SetStream(  // ���ô�����ģʽ
	ULONG			iIndex,  // ָ��CH341�豸���
	ULONG			iMode );  // ָ��ģʽ,������
// λ1-λ0: I2C�ӿ��ٶ�/SCLƵ��, 00=����/20KHz,01=��׼/100KHz(Ĭ��ֵ),10=����/400KHz,11=����/750KHz
// λ2:     SPI��I/O��/IO����, 0=���뵥��(D3ʱ��/D5��/D7��)(Ĭ��ֵ),1=˫��˫��(D3ʱ��/D5��D4��/D7��D6��)
// λ7:     SPI�ֽ��е�λ˳��, 0=��λ��ǰ, 1=��λ��ǰ
// ��������,����Ϊ0


BOOL	WINAPI	CH341SetDelaymS(  // ����Ӳ���첽��ʱ,���ú�ܿ췵��,������һ��������֮ǰ��ʱָ��������
	ULONG			iIndex,  // ָ��CH341�豸���
	ULONG			iDelay );  // ָ����ʱ�ĺ�����


BOOL	WINAPI	CH341StreamI2C(  // ����I2C������,2�߽ӿ�,ʱ����ΪSCL����,������ΪSDA����(׼˫��I/O),�ٶ�Լ56K�ֽ�
	ULONG			iIndex,  // ָ��CH341�豸���
	ULONG			iWriteLength,  // ׼��д���������ֽ���
	PVOID			iWriteBuffer,  // ָ��һ��������,����׼��д��������,���ֽ�ͨ����I2C�豸��ַ����д����λ
	ULONG			iReadLength,  // ׼����ȡ�������ֽ���
	PVOID			oReadBuffer );  // ָ��һ��������,���غ��Ƕ��������


typedef	enum	_EEPROM_TYPE {					// EEPROM�ͺ�
	ID_24C01,
	ID_24C02,
	ID_24C04,
	ID_24C08,
	ID_24C16,
	ID_24C32,
	ID_24C64,
	ID_24C128,
	ID_24C256,
	ID_24C512,
	ID_24C1024,
	ID_24C2048,
	ID_24C4096
} EEPROM_TYPE;


BOOL	WINAPI	CH341ReadEEPROM(  // ��EEPROM�ж�ȡ���ݿ�,�ٶ�Լ56K�ֽ�
	ULONG			iIndex,  // ָ��CH341�豸���
	EEPROM_TYPE		iEepromID,  // ָ��EEPROM�ͺ�
	ULONG			iAddr,  // ָ�����ݵ�Ԫ�ĵ�ַ
	ULONG			iLength,  // ׼����ȡ�������ֽ���
	PUCHAR			oBuffer );  // ָ��һ��������,���غ��Ƕ��������


BOOL	WINAPI	CH341WriteEEPROM(  // ��EEPROM��д�����ݿ�
	ULONG			iIndex,  // ָ��CH341�豸���
	EEPROM_TYPE		iEepromID,  // ָ��EEPROM�ͺ�
	ULONG			iAddr,  // ָ�����ݵ�Ԫ�ĵ�ַ
	ULONG			iLength,  // ׼��д���������ֽ���
	PUCHAR			iBuffer );  // ָ��һ��������,����׼��д��������


BOOL	WINAPI	CH341GetInput(  // ͨ��CH341ֱ���������ݺ�״̬,Ч�ʱ�CH341GetStatus����
	ULONG			iIndex,  // ָ��CH341�豸���
	PULONG			iStatus );  // ָ��һ��˫�ֵ�Ԫ,���ڱ���״̬����,�ο������λ˵��
// λ7-λ0��ӦCH341��D7-D0����
// λ8��ӦCH341��ERR#����, λ9��ӦCH341��PEMP����, λ10��ӦCH341��INT#����, λ11��ӦCH341��SLCT����, λ23��ӦCH341��SDA����
// λ13��ӦCH341��BUSY/WAIT#����, λ14��ӦCH341��AUTOFD#/DATAS#����,λ15��ӦCH341��SLCTIN#/ADDRS#����


BOOL	WINAPI	CH341SetOutput(  // ����CH341��I/O����,��ͨ��CH341ֱ���������
/* ***** ����ʹ�ø�API, ��ֹ�޸�I/O����ʹ�������ű�Ϊ������ŵ����������������֮���·����оƬ ***** */
	ULONG			iIndex,  // ָ��CH341�豸���
	ULONG			iEnable,  // ������Ч��־,�ο������λ˵��
// λ0Ϊ1˵��iSetDataOut��λ15-λ8��Ч,�������
// λ1Ϊ1˵��iSetDirOut��λ15-λ8��Ч,�������
// λ2Ϊ1˵��iSetDataOut��7-λ0��Ч,�������
// λ3Ϊ1˵��iSetDirOut��λ7-λ0��Ч,�������
// λ4Ϊ1˵��iSetDataOut��λ23-λ16��Ч,�������
	ULONG			iSetDirOut,  // ����I/O����,ĳλ��0���Ӧ����Ϊ����,ĳλ��1���Ӧ����Ϊ���,���ڷ�ʽ��Ĭ��ֵΪ0x000FC000,�ο������λ˵��
	ULONG			iSetDataOut );  // �������,���I/O����Ϊ���,��ôĳλ��0ʱ��Ӧ��������͵�ƽ,ĳλ��1ʱ��Ӧ��������ߵ�ƽ,�ο������λ˵��
// λ7-λ0��ӦCH341��D7-D0����
// λ8��ӦCH341��ERR#����, λ9��ӦCH341��PEMP����, λ10��ӦCH341��INT#����, λ11��ӦCH341��SLCT����
// λ13��ӦCH341��WAIT#����, λ14��ӦCH341��DATAS#/READ#����,λ15��ӦCH341��ADDRS#/ADDR/ALE����
// ��������ֻ�����,������I/O����: λ16��ӦCH341��RESET#����, λ17��ӦCH341��WRITE#����, λ18��ӦCH341��SCL����, λ29��ӦCH341��SDA����


BOOL	WINAPI	CH341Set_D5_D0(  // ����CH341��D5-D0���ŵ�I/O����,��ͨ��CH341��D5-D0����ֱ���������,Ч�ʱ�CH341SetOutput����
/* ***** ����ʹ�ø�API, ��ֹ�޸�I/O����ʹ�������ű�Ϊ������ŵ����������������֮���·����оƬ ***** */
	ULONG			iIndex,  // ָ��CH341�豸���
	ULONG			iSetDirOut,  // ����D5-D0�����ŵ�I/O����,ĳλ��0���Ӧ����Ϊ����,ĳλ��1���Ӧ����Ϊ���,���ڷ�ʽ��Ĭ��ֵΪ0x00ȫ������
	ULONG			iSetDataOut );  // ����D5-D0�����ŵ��������,���I/O����Ϊ���,��ôĳλ��0ʱ��Ӧ��������͵�ƽ,ĳλ��1ʱ��Ӧ��������ߵ�ƽ
// �������ݵ�λ5-λ0�ֱ��ӦCH341��D5-D0����


BOOL	WINAPI	CH341StreamSPI3(  // ��API��ʧЧ,����ʹ��
	ULONG			iIndex,
	ULONG			iChipSelect,
	ULONG			iLength,
	PVOID			ioBuffer );


BOOL	WINAPI	CH341StreamSPI4(  // ����SPI������,4�߽ӿ�,ʱ����ΪDCK/D3����,���������ΪDOUT/D5����,����������ΪDIN/D7����,Ƭѡ��ΪD0/D1/D2,�ٶ�Լ68K�ֽ�
/* SPIʱ��: DCK/D3����Ϊʱ�����, Ĭ��Ϊ�͵�ƽ, DOUT/D5������ʱ��������֮ǰ�ĵ͵�ƽ�ڼ����, DIN/D7������ʱ���½���֮ǰ�ĸߵ�ƽ�ڼ����� */
	ULONG			iIndex,  // ָ��CH341�豸���
	ULONG			iChipSelect,  // Ƭѡ����, λ7Ϊ0�����Ƭѡ����, λ7Ϊ1�������Ч: λ1λ0Ϊ00/01/10�ֱ�ѡ��D0/D1/D2������Ϊ�͵�ƽ��ЧƬѡ
	ULONG			iLength,  // ׼������������ֽ���
	PVOID			ioBuffer );  // ָ��һ��������,����׼����DOUTд��������,���غ��Ǵ�DIN���������


BOOL	WINAPI	CH341StreamSPI5(  // ����SPI������,5�߽ӿ�,ʱ����ΪDCK/D3����,���������ΪDOUT/D5��DOUT2/D4����,����������ΪDIN/D7��DIN2/D6����,Ƭѡ��ΪD0/D1/D2,�ٶ�Լ30K�ֽ�*2
/* SPIʱ��: DCK/D3����Ϊʱ�����, Ĭ��Ϊ�͵�ƽ, DOUT/D5��DOUT2/D4������ʱ��������֮ǰ�ĵ͵�ƽ�ڼ����, DIN/D7��DIN2/D6������ʱ���½���֮ǰ�ĸߵ�ƽ�ڼ����� */
	ULONG			iIndex,  // ָ��CH341�豸���
	ULONG			iChipSelect,  // Ƭѡ����, λ7Ϊ0�����Ƭѡ����, λ7Ϊ1�������Ч: λ1λ0Ϊ00/01/10�ֱ�ѡ��D0/D1/D2������Ϊ�͵�ƽ��ЧƬѡ
	ULONG			iLength,  // ׼������������ֽ���
	PVOID			ioBuffer,  // ָ��һ��������,����׼����DOUTд��������,���غ��Ǵ�DIN���������
	PVOID			ioBuffer2 );  // ָ��ڶ���������,����׼����DOUT2д��������,���غ��Ǵ�DIN2���������


BOOL	WINAPI	CH341BitStreamSPI(  // ����SPIλ������,4��/5�߽ӿ�,ʱ����ΪDCK/D3����,���������ΪDOUT/DOUT2����,����������ΪDIN/DIN2����,Ƭѡ��ΪD0/D1/D2,�ٶ�Լ8Kλ*2
	ULONG			iIndex,  // ָ��CH341�豸���
	ULONG			iLength,  // ׼�����������λ��,һ�����896,���鲻����256
	PVOID			ioBuffer );  // ָ��һ��������,����׼����DOUT/DOUT2/D2-D0д��������,���غ��Ǵ�DIN/DIN2���������
/* SPIʱ��: DCK/D3����Ϊʱ�����, Ĭ��Ϊ�͵�ƽ, DOUT/D5��DOUT2/D4������ʱ��������֮ǰ�ĵ͵�ƽ�ڼ����, DIN/D7��DIN2/D6������ʱ���½���֮ǰ�ĸߵ�ƽ�ڼ����� */
/* ioBuffer�е�һ���ֽڹ�8λ�ֱ��ӦD7-D0����, λ5�����DOUT, λ4�����DOUT2, λ2-λ0�����D2-D0, λ7��DIN����, λ6��DIN2����, λ3���ݺ��� */
/* �ڵ��ø�API֮ǰ,Ӧ���ȵ���CH341Set_D5_D0����CH341��D5-D0���ŵ�I/O����,���������ŵ�Ĭ�ϵ�ƽ */


BOOL	WINAPI	CH341SetBufUpload(  // �趨�ڲ������ϴ�ģʽ
	ULONG			iIndex,  // ָ��CH341�豸���,0��Ӧ��һ���豸
	ULONG			iEnableOrClear );  // Ϊ0���ֹ�ڲ������ϴ�ģʽ,ʹ��ֱ���ϴ�,��0�������ڲ������ϴ�ģʽ������������е���������
// ��������ڲ������ϴ�ģʽ,��ôCH341�������򴴽��߳��Զ�����USB�ϴ����ݵ��ڲ�������,ͬʱ����������е���������,��Ӧ�ó������CH341ReadData���������ػ������е���������


LONG	WINAPI	CH341QueryBufUpload(  // ��ѯ�ڲ��ϴ��������е��������ݰ�����,�ɹ��������ݰ�����,��������-1
	ULONG			iIndex );  // ָ��CH341�豸���,0��Ӧ��һ���豸


BOOL	WINAPI	CH341SetBufDownload(  // �趨�ڲ������´�ģʽ
	ULONG			iIndex,  // ָ��CH341�豸���,0��Ӧ��һ���豸
	ULONG			iEnableOrClear );  // Ϊ0���ֹ�ڲ������´�ģʽ,ʹ��ֱ���´�,��0�������ڲ������´�ģʽ������������е���������
// ��������ڲ������´�ģʽ,��ô��Ӧ�ó������CH341WriteData�󽫽����ǽ�USB�´����ݷŵ��ڲ�����������������,����CH341�������򴴽����߳��Զ�����ֱ�����


LONG	WINAPI	CH341QueryBufDownload(  // ��ѯ�ڲ��´��������е�ʣ�����ݰ�����(��δ����),�ɹ��������ݰ�����,��������-1
	ULONG			iIndex );  // ָ��CH341�豸���,0��Ӧ��һ���豸


BOOL	WINAPI	CH341ResetInter(  // ��λ�ж����ݶ�����
	ULONG			iIndex );  // ָ��CH341�豸���


BOOL	WINAPI	CH341ResetRead(  // ��λ���ݿ������
	ULONG			iIndex );  // ָ��CH341�豸���


BOOL	WINAPI	CH341ResetWrite(  // ��λ���ݿ�д����
	ULONG			iIndex );  // ָ��CH341�豸���


typedef		VOID	( CALLBACK	* mPCH341_NOTIFY_ROUTINE ) (  // �豸�¼�֪ͨ�ص�����
	ULONG			iEventStatus );  // �豸�¼��͵�ǰ״̬(�����ж���): 0=�豸�γ��¼�, 3=�豸�����¼�

#define		CH341_DEVICE_ARRIVAL		3		// �豸�����¼�,�Ѿ�����
#define		CH341_DEVICE_REMOVE_PEND	1		// �豸��Ҫ�γ�
#define		CH341_DEVICE_REMOVE			0		// �豸�γ��¼�,�Ѿ��γ�


BOOL	WINAPI	CH341SetDeviceNotify(  // �趨�豸�¼�֪ͨ����
	ULONG					iIndex,  // ָ��CH341�豸���,0��Ӧ��һ���豸
	PCHAR					iDeviceID,  // ��ѡ����,ָ���ַ���,ָ������ص��豸��ID,�ַ�����\0��ֹ
	mPCH341_NOTIFY_ROUTINE	iNotifyRoutine );  // ָ���豸�¼��ص�����,ΪNULL��ȡ���¼�֪ͨ,�����ڼ�⵽�¼�ʱ���øó���


BOOL	WINAPI	CH341SetupSerial(  // �趨CH341�Ĵ�������,��APIֻ�����ڹ����ڴ��ڷ�ʽ��CH341оƬ
	ULONG					iIndex,  // ָ��CH341�豸���,0��Ӧ��һ���豸
	ULONG					iParityMode,  // ָ��CH341���ڵ�����У��ģʽ: NOPARITY/ODDPARITY/EVENPARITY/MARKPARITY/SPACEPARITY
	ULONG					iBaudRate );  // ָ��CH341���ڵ�ͨѶ������ֵ,������50��3000000֮�������ֵ

/*  ����API�������ڹ����ڴ��ڷ�ʽ��CH341оƬ,����֮���APIһ��ֻ�����ڲ��ڷ�ʽ��CH341оƬ
	CH341OpenDevice
	CH341CloseDevice
	CH341SetupSerial
	CH341ReadData
	CH341WriteData
	CH341SetBufUpload
	CH341QueryBufUpload
	CH341SetBufDownload
	CH341QueryBufDownload
	CH341SetDeviceNotify
	CH341GetStatus
//  ��������ҪAPI,�����Ǵ�ҪAPI
	CH341GetVersion
	CH341DriverCommand
	CH341GetDrvVersion
	CH341ResetDevice
	CH341GetDeviceDescr
	CH341GetConfigDescr
	CH341SetIntRoutine
	CH341ReadInter
	CH341AbortInter
	CH341AbortRead
	CH341AbortWrite
	CH341ReadI2C
	CH341WriteI2C
	CH341SetExclusive
	CH341SetTimeout
	CH341GetDeviceName
	CH341GetVerIC
	CH341FlushBuffer
	CH341WriteRead
	CH341ResetInter
	CH341ResetRead
	CH341ResetWrite
*/
HANDLE	WINAPI	CH341OpenDeviceEx(   // ��CH341�豸,���ؾ��,��������Ч
    ULONG			iIndex );        // ָ��CH341�豸���,0��Ӧ����ĵ�һ���豸,1��Ӧ����ĵڶ����豸,Ϊ��Լ�豸�豸�����Դ,�����Ҫ�ر��豸

VOID	WINAPI	CH341CloseDeviceEx(  // �ر�CH341�豸
	ULONG			iIndex );        // ָ��CH341�豸���

PCHAR	WINAPI	CH341GetDeviceNameEx(   // ����ָ��CH341�豸���ƵĻ�����,�����򷵻�NULL
	ULONG			iIndex );           // ָ��CH341�豸���,0��Ӧ��һ���豸

BOOL	WINAPI	CH341SetDeviceNotifyEx(       // �趨�豸�¼�֪ͨ����
	ULONG					iIndex,           // ָ��CH341�豸���,0��Ӧ��һ���豸
	PCHAR					iDeviceID,        // ��ѡ����,ָ���ַ���,ָ������ص��豸��ID,�ַ�����\0��ֹ
	mPCH341_NOTIFY_ROUTINE	iNotifyRoutine ); // ָ���豸�¼��ص�����,ΪNULL��ȡ���¼�֪ͨ,�����ڼ�⵽�¼�ʱ���øó���


#ifdef __cplusplus
}
#endif

#endif		// _CH341_DLL_H