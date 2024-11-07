#ifndef		_DEBUG368_H
#define		_DEBUG368_H
#include <windows.h>
#include "CH367DLL.h"
#ifdef __cplusplus
extern "C" {
#endif

#define EEPROM_INFO_SIZE 0x20

/*============================= MEM��д ==============================*/
enum CH36xMemModel
{
     CH36xMemModel_BYTE = 0,
     CH36xMemModel_DWORD,
};

struct ch36xDevEntity {
    unsigned devIndex; /* �豸��� */
    char isOpened; /* �豸���ر�־, 0�� 1�� */
    mPCH367_IO_REG mBaseAddr; /* I/O�ռ�Ĵ��� */
    mPCH368_MEM_REG mMemAddr;/*Mem�ռ�Ĵ���*/

    // �ж���CH367����CH368
    int isCH367;
    int isCH368;

    //�ж��ǵ�ƽ�жϣ�1�����Ǳ����жϣ�2��
    int intType;

    unsigned char manufacturerDeviceID[mMAX_BUFFER_LENGTH*3];

    char isReadEERPROM;
    unsigned short vid;// VID(���̱�ʶ : Vendor ID)
    unsigned short did;// DID(�豸��ʶ: Device ID)
    unsigned short rid/* ֻ��һ���ֽ�*/;// RID(оƬ�汾 : Revision ID)
    unsigned short svid;// SVID(��ϵͳ���̱�ʶ : Subsystem Vendor ID)
    unsigned short sid;// SID(��ϵͳ��ʶ : Subsystem ID)
};


void CH36xShowDevVer(void); //��������汾��

// ָ��index��ch36x�豸
bool ch36xOpenDevice(struct ch36xDevEntity * dev, unsigned int devIndex);

// �ر�ch36x�豸
int ch36xCloseDevice(struct ch36xDevEntity * dev);

// ����ch36x�豸�Ƿ�����32λ��д��ch368Ĭ�ϴ򿪴���,ch367��֧�ִ˹��ܣ�
int ch36xMemConfig32BitRW(struct ch36xDevEntity * dev, int enable);

// д��ch36x���ڴ�
void ch36xMemWrite(struct ch36xDevEntity * dev, enum CH36xMemModel MemModel, ULONG addr,  ULONG len, unsigned char *writeBuff);//MEMд����
// ��ȡch36x���ڴ�
void ch36xMemRead(struct ch36xDevEntity * dev, enum CH36xMemModel MemModel, ULONG addr,  ULONG len, unsigned char *recvBuff);//MEM������

// Ŀǰ����ӿ�дֵ��ԭ�ⲻ���ģ���Ϊ���ܶ�д0, ���û�п���д��ID�Ĳ����������Թ��ǿ���������д��
void ch36xEEPRomWrite(struct ch36xDevEntity * dev);
int ch36xEEPRomRead(struct ch36xDevEntity * dev);


#ifdef __cplusplus
}
#endif

#endif // _DEBUG367_H
