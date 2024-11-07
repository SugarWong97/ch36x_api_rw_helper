#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <string.h>

#include "CH367DLL.h"
#include "ch36x_api_rw_helper.H"

#pragma comment(lib,"CH367DLL.LIB")

#define WM_INTNOTIFY (WM_USER + 1)

void ch36xGPOSET(struct ch36xDevEntity * dev)
{
    unsigned char qbyte = 0;
    CH367mReadIoByte(dev->devIndex, &dev->mBaseAddr->mCH367IoPort[0xF1], &qbyte);
    CH367mWriteIoByte(dev->devIndex, &dev->mBaseAddr->mCH367IoPort[0xF1], qbyte |= 0x80);
}

void ch36xGPOCLR(struct ch36xDevEntity * dev)
{
    unsigned char qbyte = 0;
    CH367mReadIoByte(dev->devIndex, &dev->mBaseAddr->mCH367IoPort[0xF1], &qbyte);
    CH367mWriteIoByte(dev->devIndex, &dev->mBaseAddr->mCH367IoPort[0xF1], qbyte &= 0x7F);
}

//void CALLBACK InterruptEvent(void); // �豸�ж�֪ͨ��Ϣ

// ����ϰ��ʹ�õĶ���
#define USING_LITTLE_ENDIAN

char irq_buf[100] = "";
//ULONG mCount = 0;//��ƽ�жϼ���
//ULONG mCount1=0;//�����жϼ���
//UCHAR PulWidth[]={0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4a,0x4b,0x4c,0x4d,0x4e};//����������
//UCHAR CurPulWidth;//��ǰ������


void CH36xShowDevVer(void) //��������汾��
{
    ULONG ver = 0;
    ver = CH367GetDrvVersion();
    printf("CH36x Driver Version :%lx\n", ver);
}

void Ch36xAddrRefresh(struct ch36xDevEntity * dev) // ȡI/O��ַ�洢����ַ���жϺ�
{
    unsigned int devIndex = dev->devIndex;
    ULONG oIntLine;
    UCHAR oByte, x;

    if(CH367mGetIntLine(devIndex, &oIntLine))
    {
        printf("IntLine %lx\n", oIntLine);
    }

    if(CH367mGetIoBaseAddr(devIndex, &dev->mBaseAddr)) //ȡI/O��ַ
    {
        printf("IO Base Address %p\n", dev->mBaseAddr);
    }
    if(dev->isCH368==1)
    {
        if(CH368mGetMemBaseAddr(devIndex, &dev->mMemAddr)) //ȡMEM��ַ
        {
            printf("MEM Base Address %p\n", dev->mMemAddr);
        }
    }
    if(CH367mReadIoByte(devIndex, &dev->mBaseAddr->mCH367Speed, &oByte))//��ʾ��ǰ������
    {

        x=oByte&0X0F;
        printf("PLUS Width %d\n", (x+1)*30);
    }

    if(dev->isCH368==1)
    {
        if(!CH367mWriteIoByte(devIndex,&dev->mBaseAddr->mCH367Speed, oByte|0x40))//����CH368֧��32λ��IO���ߴ洢�����ж�д
        {
            printf("Config 32-Bit MEM R/W ERROR\n");
        }else
        {
            printf("Config 32-Bit MEM R/W OK\n");
        }
    }
}


/* �����볣������,�����25F512оƬ˵���� */
#define WREN		0x06 /* дʹ��ָ�� */
#define WRDI		0x04 /* д��ָֹ�� */
#define EWSR		0x50 /* д״̬�Ĵ���ʹ��ָ�� */
#define RDSR		0x05 /* ��״̬�Ĵ���ָ�� */
#define WRSR		0x01 /* д״̬�Ĵ���ָ��,WELΪ0������д */
#define READ		0x03 /* ���ֽ�ָ�� */
#define FAST_READ	0x0B /* ���ָ�� */
#define BP			0x02 /* �ֽ�дָ�� */
#define AAI			0xAF /* ����дָ�� */
#define SE			0x20 /* ��������ָ�� */
#define BE			0xD8 /* �����ָ�� */
//#define CE			0xC7 /* оƬ����ָ��*/
#define CE			0x60 /* оƬ����ָ��*/
#define RMDI        0x90 /* ������/�豸IDָ�� */

#define SPI_BUFFER_LENGTH 0x1000 /* ÿ�ζ�4096���ֽ� */

BOOL ch36xReadanufacturerDeviceID(struct ch36xDevEntity * dev) // ������/�豸ID
{
    unsigned char buffer[8];
    unsigned long len = 0x04;
    buffer[0] = RMDI;
    buffer[1] = 0x00;
    buffer[2] = 0x00;
    buffer[3] = 0x00;
    if( CH367StreamSPI(dev->devIndex, 0x04, buffer, &len, dev->manufacturerDeviceID) == FALSE ) return FALSE;
    return TRUE;
}

bool ch36xOpenDevice(struct ch36xDevEntity * dev, unsigned int devIndex)
{
    UCHAR mByte;

    //mCount=0;
    //mCount1=0;
    if(dev->isOpened)
    {
        ch36xCloseDevice(dev);
    }

    printf("Index is %d\n", devIndex);
    if(CH367mOpenDevice(dev->devIndex, FALSE, TRUE, 0x00) == INVALID_HANDLE_VALUE) //  ʹ���ж�, ��ƽ�ж�ģʽ, �͵�ƽ����
    {
        return false;
    }
    dev->devIndex = devIndex;

    CH367mReadConfig(devIndex, (PVOID)0x02, &mByte);//��ȡ�豸�����ÿռ�����ȷ����ʲô�豸
    if(mByte==0x34)////0x34��ʾCH368�豸,0x31����0x30��ʾCH367,������������豸��ʶ,���豸�����üĴ����е�02H�ռ䣬�뽫�����и�������ı�ʶ
    {
        dev->isCH368=1;
        dev->isCH367=0;
        printf("Ch36x Open 368 at %d\n", devIndex);
    }
    else if(mByte==0x31||mByte==0x30)
    {
        dev->isCH367=1;
        dev->isCH368=0;
        printf("Ch36x Open 367 at %d\n", devIndex);
    }
    else
    {
        printf("Ch36x Open Unknown at %d\n", devIndex);
        ch36xCloseDevice(dev);
        return false;
    }

    if(dev->isCH368==1)//�����CH368�豸�����´�֧�ִ洢����д
    {
        CH367mCloseDevice(devIndex);
        CH367mOpenDevice(devIndex, TRUE, TRUE, 0x00);//���´�CH368�豸������֧��Mem
    }
    dev->isOpened = 0x01;

    CH36xShowDevVer(); // ��ʾ�����汾��
    Ch36xAddrRefresh(dev);

#if 0
    //CH367mSetIntRoutine(devIndex, InterruptEvent); // �����жϷ������
#else
    CH367mSetIntRoutine(devIndex, NULL); // �����жϷ������
#endif
    //if(dev->isCH367==1)
    //{
    //  sprintf(buffer_open, "CH367�豸%d�Ѿ���", devIndex);
    //}
    //if(dev->isCH368==1)
    //{
    //  sprintf(buffer_open, "CH368�豸%d�Ѿ���", devIndex);
    //}

    ch36xReadanufacturerDeviceID(dev); //���FLASH�Ƿ�����
    if(dev->manufacturerDeviceID[0] == 0xBF && dev->manufacturerDeviceID[1] == 0x48 && dev->manufacturerDeviceID[2] == 0xBF && dev->manufacturerDeviceID[3] == 0x48)
    {
        printf("Flash OK\n");
    }
    return true;
}

int ch36xCloseDevice(struct ch36xDevEntity * dev)
{
    CH367mCloseDevice(dev->devIndex);
    dev->isOpened = false;
    return true;
}

int ch36xMemConfig32BitRW(struct ch36xDevEntity * dev, int enable)
{
    UCHAR oByte;
    if(dev->isCH368==1)
    {
        CH367mReadIoByte(dev->devIndex, &dev->mBaseAddr->mCH367Speed,&oByte);//����IO�Ĵ�����speedλʵ��32λ֧��
        if(enable)
        {
            CH367mWriteIoByte(dev->devIndex,&dev->mBaseAddr->mCH367Speed,oByte|0x40);
            //SendDlgItemMessage(hDialog,IDC_CBMEMMODEL,CB_INSERTSTRING,1,"˫��");
            printf("Mem 32-Bit Config Enable\n");
        }
        else
        {
            CH367mWriteIoByte(dev->devIndex,&dev->mBaseAddr->mCH367Speed,oByte&0x3F);
            printf("Mem 32-Bit Config Disable\n");
        }
        return true;
    }
    return false;
}

BOOL mCheckWord(USHORT mWord, int dFlag) //����ַ�Ƿ��ܴ洢�ֻ�˫��
{
    if(dFlag == 4)
    {
        if(((USHORT)mWord % 2)  == 0)
        {
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
    else if(dFlag == 8)
    {
        if(((USHORT)mWord % 4)  == 0)
        {
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
    return FALSE;
}

void ch36xMemRead(struct ch36xDevEntity * dev, enum CH36xMemModel MemModel, ULONG addr,  ULONG len, unsigned char *recvBuff)//MEM������
{
    unsigned int i;
    ULONG mLen=0,mAddr;
    UCHAR mBuf[mMAX_BUFFER_LENGTH]="";
    unsigned int devIndex = dev->devIndex;
    mAddr = addr;
    mLen = len;

    if(mLen >= 0x8000)
    {
        //MessageBox(hDialog,"������С�ڵ���0x8000�ĳ���!","��ʾ",MB_OK|MB_ICONERROR);
        printf("Len too long %lx > 0x8000\n", mLen);
        return;
    }
    if(mAddr + mLen > 0x7FFF)
    {
        //MessageBox(hDialog,"������ĵ�ַ+����>0x7FFF��������,�����CH367mAccessBlock�����˵��","��ʾ",MB_OK|MB_ICONSTOP);
        printf("MEM Read addr + len = %lx > 0x7FFF, chcek CH367mAccessBlock", mAddr + mLen);
        return;
    }

    if(MemModel == CH36xMemModel_BYTE)
    {
        printf("Read Byte\n");
        if(!CH367mAccessBlock(devIndex,mFuncReadMemByte,&dev->mMemAddr->mCH368MemPort[mAddr],mBuf,mLen))
        {
            return;
        }

    }else
    {
        if(!mCheckWord(mAddr, 8)||!mCheckWord(mLen, 8))
        {
            //MessageBox(hDialog, "�������ܴ洢˫�ֵ���ʼ��ַ�����ݳ���Ϊ4�ı���", "��ʾ", MB_OK | MB_ICONSTOP);
            return;
        }
        printf("Read 4-Byte\n");
        if(!CH367mAccessBlock(devIndex, mFuncReadMemDword, &dev->mMemAddr->mCH368MemPort[mAddr], mBuf, mLen))
        {
            return;
        }
    }

    for(i = 0; i < mLen; i++)
    {
        if(recvBuff!=NULL)
        {
            recvBuff[i] = mBuf[i];
        }
    }
}

void ch36xMemWrite(struct ch36xDevEntity * dev, enum CH36xMemModel MemModel, ULONG addr,  ULONG len, unsigned char *writeBuff)//MEM������
{
    //unsigned int i;
    ULONG mLen=0,mAddr;
    UCHAR buffer[mMAX_BUFFER_LENGTH]="";

    mAddr = addr;
    mLen = len;

    if(mLen >= 0x8000)
    {
        //MessageBox(hDialog,"������С�ڵ���0x8000�ĳ���!","��ʾ",MB_OK|MB_ICONERROR);
        printf("Len too long %lx > 0x8000\n", mLen);
        return;
    }
    if(mAddr + mLen > 0x7FFF)
    {
        //MessageBox(hDialog,"������ĵ�ַ+����>0x7FFF��������,�����CH367mAccessBlock�����˵��","��ʾ",MB_OK|MB_ICONSTOP);
        printf("MEM Read addr + len = %lx > 0x7FFF, chcek CH367mAccessBlock", mAddr + mLen);
        return;
    }

    memcpy(buffer, writeBuff,mLen);
    if(MemModel==0) //���ֽڵķ�ʽ��дMEM
    {
        if(!CH367mAccessBlock(dev->devIndex, mFuncWriteMemByte, &dev->mMemAddr->mCH368MemPort[mAddr],buffer,mLen))
        {
            return;
        }
    }
    else//��˫�ֵķ�ʽ��дMEM
    {
        if(!mCheckWord(mAddr, 8))
        {
            //MessageBox(hDialog, "�������ܴ洢˫�ֵ���ʼ��ַ�����ݳ���Ϊ4�ı���", "��ʾ", MB_OK | MB_ICONSTOP);
            return;
        }
        if(!CH367mAccessBlock(dev->devIndex, mFuncWriteMemDword, &dev->mMemAddr->mCH368MemPort[mAddr],buffer,mLen))
        {
            return;
        }
    }
}

// ��2�����ֽ���ϳ�Ϊ16λ��
//   0xaa 0xbb --> 0xaabb
#define   halfWord16From2Bytes(h,l)           ((((unsigned short)(h)) << 8) | (unsigned char)(l))

int ch36xEEPRomRead(struct ch36xDevEntity * dev) // ��EEPROM
{
    UCHAR i, data[32]; // sByte[8]

    for(i = 0; i < EEPROM_INFO_SIZE; i++)
    {
        if(!CH367mReadI2C(dev->devIndex, 0x50, i, &data[i]))
        {
            //MessageBox(mSaveDialogI2c, "дEEPROMʧ��,����EEPROM�𻵻�û������", mCaptionInform, MB_OK);
            printf("Read EEPROM Failed!\n");
            return -1;
        }
    }
    dev->isReadEERPROM = 1;
    // VID(���̱�ʶ : Vendor ID)
    dev->vid  = halfWord16From2Bytes(data[5], data[4]);
    // DID(�豸��ʶ: Device ID)
    dev->did  = halfWord16From2Bytes(data[7], data[6]);
    // RID(оƬ�汾 : Revision ID)
    dev->rid  = halfWord16From2Bytes(0x00, data[8]); // ֻ��һ���ֽ�
    // SVID(��ϵͳ���̱�ʶ : Subsystem Vendor ID)
    dev->svid = halfWord16From2Bytes(data[13], data[12]);
    // SID(��ϵͳ��ʶ : Subsystem ID)
    dev->sid  = halfWord16From2Bytes(data[15], data[14]);

    printf("VID  = %04x\n", dev->vid);
    printf("DID  = %04x\n", dev->did);
    printf("RID  = %04x\n", dev->rid);
    printf("SVID = %04x\n", dev->svid);
    printf("SID  = %04x\n", dev->sid);

    printf("Read EEPROM OK\n");
    return 0;
}

// Ŀǰ����ӿ�дֵ��ԭ�ⲻ���ģ���Ϊ���ܶ�д0, ���û�п���д��ID�Ĳ����������Թ��ǿ���������д��
void ch36xEEPRomWrite(struct ch36xDevEntity * dev) //�޸�VID,DID...
{
    UCHAR i, buffer[32]; //  data[8],

    if (dev->isReadEERPROM == 0)
    {
        printf("Read Frist\n");
        ch36xEEPRomRead(dev);
    }

    buffer[0] = 0x78;
    buffer[1] = 0x00;
    buffer[2] = 0x00;
    buffer[3] = 0x00;

    buffer[4] = dev->vid & 0xFF;
    buffer[5] = dev->vid >> 8;
    buffer[6] = dev->did & 0xFF;
    buffer[7] = dev->did >> 8;
    buffer[8]  = dev->rid & 0xFF;
    buffer[9]  = 0x00;
    buffer[10] = 0x00;
    buffer[11] = 0x10;
    buffer[12] = dev->svid & 0xFF;
    buffer[13] = dev->svid >> 8;
    buffer[14] = dev->sid & 0xFF;
    buffer[15] = dev->sid >> 8;
    for(i = 0x10; i < EEPROM_INFO_SIZE; i++)
    {
        buffer[i] = 0x00;
    }

    for(i = 0; i < EEPROM_INFO_SIZE; i++)
    {
        printf("Write EEPROM at [%02d] : %02x\n", i, buffer[i]);
#if 0
        if(CH367mWriteI2C(devIndex, 0x50, i, buffer[i]))
        {
            Sleep(20); // ���Ҫ����ʱ
        }
        else
        {
            //MessageBox(mSaveDialogI2c, "дEEPROMʧ��,����EEPROM�𻵻�û������", mCaptionInform, MB_OK);
            printf("Write EEPROM Failed!\n");
            return;
        }
#else
        printf("!! DEBUG : Skip Real-write\n");
#endif
    }
    printf("Write EEPROM OK\n");
}

int testApiFull(void)
{
    int index = 0;

    struct ch36xDevEntity dev;

    if(ch36xOpenDevice(&dev, index))
    {
        return -1;
    }

    ch36xMemConfig32BitRW(&dev, true);

    Sleep(100);

#if 0
    unsigned char buff[256];
    unsigned int i;
    ULONG addr = 0x4;
    ULONG len = 4;
    ch36xMemRead(CH36xMemModel_DWORD/*CH36xMemModel_BYTE*/,
                addr, len, buff);
    for(i = 0; i < len; i++)
    {

        printf("%x\n", buff[i]);
    }

    for(i = 0; i < len; i++)
    {

        buff[i] = 0xff;
    }
    ch36xMemWrite(index, CH36xMemModel_DWORD/*CH36xMemModel_BYTE*/,
                addr, len, buff);

    Sleep(100);
    ch36xMemRead(index, CH36xMemModel_DWORD/*CH36xMemModel_BYTE*/,
                addr, len, buff);
    for(i = 0; i < len; i++)
    {

        printf("%x\n", buff[i]);
    }
#endif

    //ch36xEEPRomRead(&dev);
    //ch36xEEPRomWrite(&dev);

    ch36xCloseDevice(&dev);

    return 0;
}



