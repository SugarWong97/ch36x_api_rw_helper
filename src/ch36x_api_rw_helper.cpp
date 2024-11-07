#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <string.h>

#include "CH367DLL.h"
#include "cmdparser.h"
#include "ch36x_api_rw_helper.H"

#pragma comment(lib,"CH367DLL.LIB")

// ����ϰ��ʹ�õ�ģʽ
#define USING_LITTLE_ENDIAN

//CHAR mCaptionInform[] = "��ʾ";
UINT mIndex = 0; /* �豸��� */
CHAR flag_open = 0; /* �豸���ر�־ */
mPCH367_IO_REG mBaseAddr; /* I/O�ռ�Ĵ��� */
mPCH368_MEM_REG mMemAddr;/*Mem�ռ�Ĵ���*/

char buffer_open[16];
char buffer_close[16];
int CH367=0;
int CH368=0;
int intType;//�ж��ǵ�ƽ�жϣ�1�����Ǳ����жϣ�2��
int Devices[5][2];//���ڴ洢�����е������豸

// CHAR SpiFilePath[256] = ""; /* ������ļ�·�� */

typedef struct _VAR_TYPE {
	union {
		UCHAR cVar;
		UINT iVar;
		USHORT sVar;
		ULONG lVar;
	};
}mVAR_TYPE, *mpVAR_TYPE;


//unsigned char qbyte = 0;
char irq_buf[100] = "";
ULONG mCount = 0;//��ƽ�жϼ���
ULONG mCount1=0;//�����жϼ���
UCHAR PulWidth[]={0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4a,0x4b,0x4c,0x4d,0x4e};//����������
UCHAR CurPulWidth;//��ǰ������

void mShowDevVer(void) //��������汾��
{
    //ULONG ver = 0;
    //char vers[8];
    //ver = CH367GetDrvVersion();
    //_ltoa(ver, vers, 16);
    //if(!SetDlgItemText(hDialog, IDC_EDTDEVVER, vers))
    //{
    //  MessageBox(hDialog, "δ�ܳɹ���������汾��!", "��ʾ", MB_ICONSTOP | MB_OK);
    //}
    ULONG ver = 0;
    //char vers[8];
    ver = CH367GetDrvVersion();
    printf("CH36x Driver Version :%x\n", ver);
}

void AddrRefresh(void) // ȡI/O��ַ�洢����ַ���жϺ�
{
    ULONG oIntLine;
    UCHAR oByte,x;
    ULONG ver = 0;
    if(CH367mGetIntLine(mIndex, &oIntLine))
    {
        //UCHAR sByte[12];
        //sprintf(sByte, "%02X\x0", oIntLine);
        //SetDlgItemText(hDialog, IDC_EDTINTLINE, sByte);
        printf("IntLine %x\n", oIntLine);
    }
    //else
    //{
    //  MessageBox(hDialog, "����жϺ�ʧ��!", "��ʾ", MB_OK | MB_ICONSTOP);
    //  SetDlgItemText(hDialog, IDC_EDTINTLINE, "");
    //}

    if(CH367mGetIoBaseAddr(mIndex, &mBaseAddr)) //ȡI/O��ַ
    {
        //UCHAR sByte[12];
        //sprintf(sByte, "%08x\x0", mBaseAddr);
        //SetDlgItemText(hDialog, IDC_EDTBASEADDR, sByte);
        printf("IO Base Address %p\n", mBaseAddr);
    }
    //else
    //{
    //  MessageBox(hDialog, "���I/O��ַʧ��!", "��ʾ", MB_OK | MB_ICONSTOP);
    //  SetDlgItemText(hDialog, IDC_EDTBASEADDR, "");
    //}
    if(CH368==1)
    {
        if(CH368mGetMemBaseAddr(mIndex, &mMemAddr)) //ȡMEM��ַ
        {
            //CHAR sByte[128]="";
            //sprintf( sByte,"%04p\x0", mMemAddr );
            //SetDlgItemText(hDialog,IDC_EDTMEM,sByte);
            printf("MEM Base Address %p\n", mMemAddr);
        }
        //else
        //{
        //  MessageBox(hDialog,"���MEM�Ĵ�����ַʧ�ܣ�","��ʾ",MB_OK|MB_ICONSTOP);
        //  SetDlgItemText(hDialog,IDC_EDTMEM,"");
        //}

    }
    if(CH367mReadIoByte(mIndex, &mBaseAddr->mCH367Speed,&oByte))//��ʾ��ǰ������
    {
        //char input[10];
        x=oByte&0X0F;
        //sprintf(input,"%d\0",(x+1)*30);
        //SetDlgItemText(mSaveDialogMain,IDC_PULSHOW,input);
        printf("PLUS Width %d\n", (x+1)*30);
    }
    //else
    //{
    //  MessageBox(mSaveDialogMain,"��ǰ�����ȡʧ��","��ʾ",MB_OK|MB_ICONSTOP);
    //}
    if(CH368==1)
    {
        if(!CH367mWriteIoByte(mIndex,&mBaseAddr->mCH367Speed,oByte|0x40))//����CH368֧��32λ��IO���ߴ洢�����ж�д
        {
            //MessageBox(mSaveDialogMain,"32λIO��д����ʧ��","��ʾ",MB_OK);
            printf("Config 32-Bit MEM R/W ERROR\n");
        }else
        {
            printf("Config 32-Bit MEM R/W OK\n");
        }
    }
}

BOOL read_manufacturer_device_id(unsigned char *data) // ������/�豸ID
{
    unsigned char buffer[8];
    unsigned long len = 0x04;
    buffer[0] = RMDI;
    buffer[1] = 0x00;
    buffer[2] = 0x00;
    buffer[3] = 0x00;
    if( CH367StreamSPI(mIndex, 0x04, buffer, &len, data) == FALSE ) return FALSE;
    return TRUE;
}

bool ch36xOpenDevice(int index)
{
    char buffer[mMAX_BUFFER_LENGTH*3];
    mIndex = index;
    UCHAR mByte;
    mCount=0;
    mCount1=0;
    if(flag_open)
    {
        ch36xCloseDevice(mIndex);
    }

    //mIndex = (UCHAR)SendDlgItemMessage(hDialog, IDC_DEVICE_NUMBER, CB_GETCURSEL, 0, 0); // ���Ҫ�򿪵��豸��
    printf("Index is %d\n", mIndex);
    if(CH367mOpenDevice(mIndex, FALSE, TRUE, 0x00) == INVALID_HANDLE_VALUE) //  ʹ���ж�, ��ƽ�ж�ģʽ, �͵�ƽ����
    {
        //sprintf(buffer_open, "�޷����豸%d,��ȷ���豸�Ѿ�����", mIndex);
        //MessageBox(hDialog, buffer_open, mCaptionInform, MB_ICONSTOP | MB_OK);
        //return( TRUE );
        return false;
    }

    CH367mReadConfig(mIndex,(PVOID)0x02, &mByte);//��ȡ�豸�����ÿռ�����ȷ����ʲô�豸
    if(mByte==0x34)////0x34��ʾCH368�豸,0x31����0x30��ʾCH367,������������豸��ʶ,���豸�����üĴ����е�02H�ռ䣬�뽫�����и�������ı�ʶ
    {
        CH368=1;
        CH367=0;
        printf("Ch36x Open 368 at %d\n", mIndex);
    }
    else if(mByte==0x31||mByte==0x30)
    {
        CH367=1;
        CH368=0;
        printf("Ch36x Open 367 at %d\n", mIndex);
    }
    else
    {
        //MessageBox(hDialog,"δ��ʶ�������豸,������������豸��ʶ,���豸�����üĴ����е�02H�ռ�,�뽫��������Ӧ�Ĵ������һ��","��ʾ",MB_OK);
        //CH367mCloseDevice(mIndex);
        //return FALSE;
        printf("Ch36x Open Unknown at %d\n", mIndex);
        ch36xCloseDevice(mIndex);
        return false;
    }

    if(CH368==1)//�����CH368�豸�����´�֧�ִ洢����д
    {
        CH367mCloseDevice(mIndex);
        CH367mOpenDevice(mIndex, TRUE, TRUE, 0x00);//���´�CH368�豸������֧��Mem
    }
    flag_open = 0x01;
    mShowDevVer(); // ��ʾ�����汾��
    AddrRefresh();

    //CH367mSetIntRoutine(mIndex, InterruptEvent); // �����жϷ������
    CH367mSetIntRoutine(mIndex, NULL); // �����жϷ������
    //if(CH367==1)
    //{
    //  sprintf(buffer_open, "CH367�豸%d�Ѿ���", mIndex);
    //}
    //if(CH368==1)
    //{
    //  sprintf(buffer_open, "CH368�豸%d�Ѿ���", mIndex);
    //}
    //SetDlgItemText(hDialog, IDC_STATUS, buffer_open);
    //EnableWindow(GetDlgItem(hDialog, IDC_BTNPU), TRUE); // �����ɼ�
    //EnableWindow(GetDlgItem(hDialog, IDC_BTNIOCL), TRUE);
    //EnableWindow(GetDlgItem(hDialog, IDC_BTNIORD), TRUE);
    //EnableWindow(GetDlgItem(hDialog, IDC_BTNIOWR), TRUE);
    //EnableWindow(GetDlgItem(hDialog, IDC_BTNSPIRD), TRUE);
    //EnableWindow(GetDlgItem(hDialog, IDC_BTNSPIWR), TRUE);
    //EnableWindow(GetDlgItem(hDialog, IDC_BTNI2CWR), TRUE);
    //EnableWindow(GetDlgItem(hDialog, IDC_BTNI2CRD), TRUE);
    //EnableWindow(GetDlgItem(hDialog, IDC_BTNI2C_PROGRAM), TRUE);
    //EnableWindow(GetDlgItem(hDialog, IDC_BTNSPICL), TRUE);
    //EnableWindow(GetDlgItem(hDialog, IDC_BTNSPIRD), TRUE);
    //EnableWindow(GetDlgItem(hDialog, IDC_BTNSPIWR), TRUE);
    //EnableWindow(GetDlgItem(hDialog, IDC_BTNCONRD), TRUE);
    //EnableWindow(GetDlgItem(hDialog, IDC_BTNCONWR), TRUE);
    //EnableWindow(GetDlgItem(hDialog, IDC_BTNINT1), TRUE);
    //EnableWindow(GetDlgItem(hDialog, IDC_BTNINT2), TRUE);
    //EnableWindow(GetDlgItem(hDialog, IDC_BTNCLOSE),TRUE);
    //EnableWindow(GetDlgItem(hDialog, IDC_BTNSPISET), TRUE);
    //EnableWindow(GetDlgItem(hDialog, IDC_BTNOPEN), FALSE);
    //EnableWindow(GetDlgItem(hDialog, IDC_DEVICE_NUMBER), FALSE);
    read_manufacturer_device_id((unsigned char*)buffer);//���FLASH�Ƿ�����
    if(buffer[0] == 0xBF && buffer[1] == 0x48 && buffer[2] == 0xBF && buffer[3] == 0x48)
    {
        printf("Flash OK\n");
        //SetDlgItemText(hDialog,IDC_EDITFLASH,"FLASH״̬����");
        //EnableWindow(GetDlgItem(hDialog,IDC_FLASH_READ),TRUE);
        //EnableWindow(GetDlgItem(hDialog,IDC_FLASH_WRITE),TRUE);
        //EnableWindow(GetDlgItem(hDialog,IDC_FLASH_ERASE),TRUE);
    }
    return true;
}

int ch36xCloseDevice(int index)
{
    mIndex = index;
    CH367mCloseDevice(mIndex);
    return true;
}


int ch36xMemConfig32BitRW(int enable)
{
    if(CH368==1)
    {
        UCHAR oByte;
        CH367mReadIoByte(mIndex, &mBaseAddr->mCH367Speed,&oByte);//����IO�Ĵ�����speedλʵ��32λ֧��
        if(enable)//IsDlgButtonChecked(hDialog,IDC_CB32)==BST_CHECKED)
        {
            CH367mWriteIoByte(mIndex,&mBaseAddr->mCH367Speed,oByte|0x40);
            //SendDlgItemMessage(hDialog,IDC_CBMEMMODEL,CB_INSERTSTRING,1,"˫��");
            printf("Mem 32-Bit Config Enable\n");
        }
        else
        {
            CH367mWriteIoByte(mIndex,&mBaseAddr->mCH367Speed,oByte&0x3F);
            printf("Mem 32-Bit Config Disable\n");
            //SendDlgItemMessage(hDialog,IDC_CBMEMMODEL,CB_DELETESTRING,1,0);
        }
        return true;
    }
    return false;
}


/*============================= MEM��д ==============================*/
enum CH36xMemModel
{
     CH36xMemModel_BYTE = 0,
     CH36xMemModel_DWORD,
};

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


void ch36xMemRead(enum CH36xMemModel MemModel, ULONG addr,  ULONG len, unsigned char *recvBuff)//MEM������
{
    unsigned int i;
    ULONG mLen=0,mAddr;
    UCHAR mBuf[mMAX_BUFFER_LENGTH]="";

    mAddr = addr;
    mLen = len;

    if(mLen >= 0x8000)
    {
        //MessageBox(hDialog,"������С�ڵ���0x8000�ĳ���!","��ʾ",MB_OK|MB_ICONERROR);
        printf("Len too long %x > 0x8000\n", mLen);
        return;
    }
    if(mAddr + mLen > 0x7FFF)
    {
        //MessageBox(hDialog,"������ĵ�ַ+����>0x7FFF��������,�����CH367mAccessBlock�����˵��","��ʾ",MB_OK|MB_ICONSTOP);
        printf("MEM Read addr + len = %x > 0x7FFF, chcek CH367mAccessBlock", mAddr + mLen);
        return;
    }


    if(MemModel == CH36xMemModel_BYTE)
    {
        printf("Read Byte\n");
        if(!CH367mAccessBlock(mIndex,mFuncReadMemByte,&mMemAddr->mCH368MemPort[mAddr],mBuf,mLen))
        {
            //MessageBox(hDialog,"MEM��ȡʧ��","��ʾ",MB_OK|MB_ICONERROR);
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
        if(!CH367mAccessBlock(mIndex, mFuncReadMemDword, &mMemAddr->mCH368MemPort[mAddr], mBuf, mLen))
        {
            //MessageBox(hDialog,"MEM��ȡʧ��","��ʾ",MB_OK|MB_ICONERROR);
            return;
        }
    }

    for(i = 0; i < mLen; i++)
    {
        if(recvBuff!=NULL)
        {
            recvBuff[i] = mBuf[i];
        }
        //printf("%x\n", mBuf[i]);
    }
}

void ch36xMemWrite(enum CH36xMemModel MemModel, ULONG addr,  ULONG len, unsigned char *writeBuff)//MEM������
{
    //unsigned int i;
    ULONG mLen=0,mAddr;
    UCHAR buffer[mMAX_BUFFER_LENGTH]="";

    mAddr = addr;
    mLen = len;

    if(mLen >= 0x8000)
    {
        //MessageBox(hDialog,"������С�ڵ���0x8000�ĳ���!","��ʾ",MB_OK|MB_ICONERROR);
        printf("Len too long %x > 0x8000\n", mLen);
        return;
    }
    if(mAddr + mLen > 0x7FFF)
    {
        //MessageBox(hDialog,"������ĵ�ַ+����>0x7FFF��������,�����CH367mAccessBlock�����˵��","��ʾ",MB_OK|MB_ICONSTOP);
        printf("MEM Read addr + len = %x > 0x7FFF, chcek CH367mAccessBlock", mAddr + mLen);
        return;
    }

    //memcpy(mBuf,bufferFilter,mLen*2);
    memcpy(buffer, writeBuff,mLen);
    if(MemModel==0) //���ֽڵķ�ʽ��дMEM
    {
        if(!CH367mAccessBlock(mIndex,mFuncWriteMemByte,&mMemAddr->mCH368MemPort[mAddr],buffer,mLen))
        {
            //MessageBox(NULL,"дʧ��","��ʾ",MB_OK|MB_ICONERROR);
            return;
        }
        else
        {
            //MessageBox(NULL,"д�ɹ�","��ʾ",MB_OK);
        }
    }
    else//��˫�ֵķ�ʽ��дMEM
    {
        if(!mCheckWord(mAddr, 8))
        {
            //MessageBox(hDialog, "�������ܴ洢˫�ֵ���ʼ��ַ�����ݳ���Ϊ4�ı���", "��ʾ", MB_OK | MB_ICONSTOP);
            return;
        }
        if(!CH367mAccessBlock(mIndex,mFuncWriteMemDword,&mMemAddr->mCH368MemPort[mAddr],buffer,mLen))
        {
            //MessageBox(NULL,"дʧ��","��ʾ",MB_OK|MB_ICONERROR);
            return;
        }
        else
        {
            //MessageBox(NULL,"д�ɹ�","��ʾ",MB_OK);
        }
    }
}

// ��2�����ֽ���ϳ�Ϊ16λ��
//   0xaa 0xbb --> 0xaabb
#define   halfWord16From2Bytes(h,l)           ((((unsigned short)(h)) << 8) | (unsigned char)(l))
static unsigned short vid, did, rid, svid, sid;
static char isReadEERPROM = 0;
#define EEPROM_INFO_SIZE 0x20
void ch36xEEPRomRead(void) // ��EEPROM
{
    UCHAR i, data[32]; // sByte[8]

    for(i = 0; i < EEPROM_INFO_SIZE; i++)
    {
        if(!CH367mReadI2C(mIndex, 0x50, i, &data[i]))
        {
            //MessageBox(mSaveDialogI2c, "дEEPROMʧ��,����EEPROM�𻵻�û������", mCaptionInform, MB_OK);
            //EndDialog(mSaveDialogI2c,2);
            printf("Read EEPROM Failed!\n");
            return;
        }
    }
    isReadEERPROM = 1;
    // VID(���̱�ʶ : Vendor ID)
    vid  = halfWord16From2Bytes(data[5], data[4]);
    // DID(�豸��ʶ: Device ID)
    did  = halfWord16From2Bytes(data[7], data[6]);
    // RID(оƬ�汾 : Revision ID)
    rid  = halfWord16From2Bytes(0x00, data[8]); // ֻ��һ���ֽ�
    // SVID(��ϵͳ���̱�ʶ : Subsystem Vendor ID)
    svid = halfWord16From2Bytes(data[13], data[12]);
    // SID(��ϵͳ��ʶ : Subsystem ID)
    sid  = halfWord16From2Bytes(data[15], data[14]);

    printf("VID  = %04x\n", vid);
    printf("DID  = %04x\n", did);
    printf("RID  = %04x\n", rid);
    printf("SVID = %04x\n", svid);
    printf("SID  = %04x\n", sid);

    //sprintf(sByte, "%02X%02X\x0", data[5], data[4]);
    //SetDlgItemText(mSaveDialogI2c, IDC_VID, sByte); // ���VID
    //sprintf(sByte, "%02X%02X\x0", data[7], data[6]);
    //SetDlgItemText(mSaveDialogI2c, IDC_DID, sByte); // ���DID
    //sprintf(sByte, "%02X\x0", data[8]);
    //SetDlgItemText(mSaveDialogI2c, IDC_RID, sByte); // ���RID
    //sprintf(sByte, "%02X%02X\x0", data[13], data[12]);
    //SetDlgItemText(mSaveDialogI2c, IDC_SVID, sByte); // ���SVID
    //sprintf(sByte, "%02X%02X\x0", data[15], data[14]);
    //SetDlgItemText(mSaveDialogI2c, IDC_SID, sByte); // ���SID
    printf("Read EEPROM OK\n");
}

void ch36xEEPRomWrite(void) //�޸�VID,DID...
{
    //mVAR_TYPE mVarType;
    UCHAR i, buffer[32]; //  data[8],
    //UINT Len = 0;
    //USHORT Value = 0;

    if (isReadEERPROM == 0)
    {
        printf("Read Frist\n");
        ch36xEEPRomRead();
    }

    buffer[0] = 0x78;
    buffer[1] = 0x00;
    buffer[2] = 0x00;
    buffer[3] = 0x00;
    //Len = GetDlgItemText(mSaveDialogI2c, IDC_VID, data, 5);
    //if(!mCheckInput(data))
    //{
    //  MessageBox(NULL,"��������ַ�����������0~9,a~f,A~F֮���ʮ��������!","��ʾ",MB_OK|MB_ICONERROR);
    //  return;
    //}
    //mVarType = mCharToVar(data, Len, 0); // ȡVID
    //Value = mVarType.sVar;
    buffer[4] = vid & 0xFF;
    buffer[5] = vid >> 8;
    //Len = GetDlgItemText(mSaveDialogI2c, IDC_DID, data, 5);
    //if(!mCheckInput(data))
    //{
    //  MessageBox(NULL,"��������ַ�����������0~9,a~f,A~F֮���ʮ��������!","��ʾ",MB_OK|MB_ICONERROR);
    //  return;
    //}
    //mVarType = mCharToVar(data, Len, 0); // ȡDID
    //Value = mVarType.sVar;
    buffer[6] = did & 0xFF;
    buffer[7] = did >> 8;
    //Len = GetDlgItemText(mSaveDialogI2c, IDC_RID, data, 3);
    //if(!mCheckInput(data))
    //{
    //  MessageBox(NULL,"��������ַ�����������0~9,a~f,A~F֮���ʮ��������!","��ʾ",MB_OK|MB_ICONERROR);
    //  return;
    //}
    //mVarType = mCharToVar(data, Len, 3); // ȡRID
    buffer[8]  = rid; //mVarType.cVar;
    buffer[9]  = 0x00;
    buffer[10] = 0x00;
    buffer[11] = 0x10;
    //Len = GetDlgItemText(mSaveDialogI2c, IDC_SVID, data, 5);
    //if(!mCheckInput(data))
    //{
    //  MessageBox(NULL,"��������ַ�����������0~9,a~f,A~F֮���ʮ��������!","��ʾ",MB_OK|MB_ICONERROR);
    //  return;
    //}
    //mVarType = mCharToVar(data, Len, 0); // ȡSVID
    //Value = mVarType.sVar;
    buffer[12] = svid & 0xFF;
    buffer[13] = svid >> 8;
    //Len = GetDlgItemText(mSaveDialogI2c, IDC_SID, data, 5);
    //if(!mCheckInput(data))
    //{
    //  MessageBox(NULL,"��������ַ�����������0~9,a~f,A~F֮���ʮ��������!","��ʾ",MB_OK|MB_ICONERROR);
    //  return;
    //}
    //mVarType = mCharToVar(data, Len, 0); // ȡSID
    //Value = mVarType.sVar;
    buffer[14] = sid & 0xFF;
    buffer[15] = sid >> 8;
    for(i = 0x10; i < EEPROM_INFO_SIZE; i++)
    {
        buffer[i] = 0x00;
    }

    for(i = 0; i < EEPROM_INFO_SIZE; i++)
    {
        printf("Write EEPROM at [%02d] : %02x\n", i, buffer[i]);
#if 0
        if(CH367mWriteI2C(mIndex, 0x50, i, buffer[i]))
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
    //MessageBox(mSaveDialogI2c, "дEEPROM���", mCaptionInform, MB_OK);
    printf("Write EEPROM OK\n");
}

int testApiFull(int argc, char *argv[])
{
    int index = 0;
    unsigned int i;
    //ULONG addr, len;
    unsigned char buff[256];
    ch36xOpenDevice(index);

    ch36xMemConfig32BitRW(true);

    Sleep(100);
#if 0
    addr = 0x4;
    len = 4;
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
    ch36xMemWrite(CH36xMemModel_DWORD/*CH36xMemModel_BYTE*/,
                addr, len, buff);

    Sleep(100);
    ch36xMemRead(CH36xMemModel_DWORD/*CH36xMemModel_BYTE*/,
                addr, len, buff);
    for(i = 0; i < len; i++)
    {

        printf("%x\n", buff[i]);
    }
#endif

    ch36xEEPRomRead();
    ch36xEEPRomWrite();

    ch36xCloseDevice(index);


    return 0;
}

static struct
{
    int index;
    char *action;
    char *outputFile;
    char *reg;
    char *val;
} arg = {0};

static cmdp_action_t callback_function(cmdp_process_param_st *params);
int main(int argc, char **argv)
{
    static cmdp_command_st g_command;

    cmdp_option_st cmd_opts[] = {
        {'i', "Index", "Device ID", CMDP_TYPE_INT4, &arg.index},
        {'a', "Action", "Action Option(mw/mr, er/ew)", CMDP_TYPE_STRING_PTR, &arg.action},
        {'f', "OutputFile", "output result to file", CMDP_TYPE_STRING_PTR, &arg.outputFile},
        {'r', "Reg", "Reg for r/w", CMDP_TYPE_STRING_PTR, &arg.reg},
        {'v', "Val", "Val for w", CMDP_TYPE_STRING_PTR, &arg.val},
        {0},
    };

    g_command.doc = "A helper for CH36x PCIe Access.\n"
           "URL: https://github.com/SugarWong97/ch36x_api_rw_helper\n"
           "\n"
           "Memory R/W example:  \n"
           "    xx.exe -i 0 -f result.txt -a mr -r 0x0000\n"
           "    xx.exe -i 0 -f result.txt -a mw -r 0x0000  -v 0x00000012\n"
           "\n"
           "EEPROM R/W example:  \n"
           " : when reg 0x00 :   vid(16 Bit)   \n"
           " : when reg 0x01 :   did(16 Bit)   \n"
           " : when reg 0x02 :   rid( 8 Bit)   \n"
           " : when reg 0x04 :  svid(16 Bit)   \n"
           " : when reg 0x04 :   sid(16 Bit)   \n"
           "    xx.exe -i 0 -f result.txt -a er -r 0x00\n"
           "    xx.exe -i 0 -f result.txt -a ew -r 0x00  -v 0x00000012\n"
           "\n";
    g_command.options = cmd_opts;

    g_command.fn_process = callback_function;
    return cmdp_run(argc - 1, argv + 1, &g_command, NULL);
}

unsigned long hexStrToValue(const char *p)
{
    unsigned long ret;
    sscanf(p, "%x", &ret);
    return ret;
}

int printValToFile(const char *fileName, unsigned int val)
{
    FILE *file;

    file = fopen(fileName, "w"); // ���ļ���д��
    if (file == NULL) {
        //printf("�ļ���ʧ�ܣ�\n");
        return 1;
    }

    // ����ʽ�����ı�д���ļ�
    fprintf(file, "%08x\n", val);

    fclose(file); // �ر��ļ�

    return 0;
}

// ��ȡ������bit(��ȡ�����Ĳ�������λ)
#define   getBits(x, end, start)   ( (x & ~(~(0U)<<((end)-(start) + 1))<<(start)) >> (start) )
// ��4�����ֽ���ϳ�Ϊ32λ��
//   0xaa 0xbb 0xcc 0xdd --> 0xaabbccdd
#define   word32From4Bytes(one,two,three,four) (((unsigned int)(one) << 24) | ((unsigned int)(two) << 16) | ((unsigned short)(three) << 8) | ((unsigned char)(four)))

static cmdp_action_t callback_function(cmdp_process_param_st *params)
{
    unsigned long reg, val;
    int index = 0;
    // unsigned int i;
    ULONG addr, len;
    unsigned char buff[256];


    len = 4;


    if (params->opts == 0)
    {
        return CMDP_ACT_SHOW_HELP;
    }
    //for (i = 0; i < params->argc; i++)
    //{
    //    printf("argv[%d]: %s\n", i, params->argv[i]);
    //}
    index = arg.index;

    if(arg.action == NULL)
    {
        printf("Action is NULL\n");
        return CMDP_ACT_SHOW_HELP;
    }
    if(arg.outputFile == NULL)
    {
        printf("outputFile is NULL\n");
        return CMDP_ACT_SHOW_HELP;
    }
    if(arg.reg == NULL)
    {
        printf("reg is NULL\n");
        return CMDP_ACT_SHOW_HELP;
    }

    printf("Device Index : %d\n", index);
    ch36xOpenDevice(index);
    Sleep(5);
    ch36xMemConfig32BitRW(true);
    Sleep(5);

    reg = hexStrToValue(arg.reg);

    if (arg.action[1] == 'w')
    {
        if(arg.val == NULL)
        {
            printf("! Write without Val!\n");
            return CMDP_ACT_SHOW_HELP;
        }

        val = hexStrToValue(arg.val);
        printf("val is %x\n", val);
    }


    addr = reg;

    if(strcmp(arg.action, "mw") ==0)
    {
        printf("Do mw : reg[%08x] write val[%08x]\n", addr, val);
#ifdef USING_LITTLE_ENDIAN // ���ֽ��ڵ͵�ַ 11 22 33 44 -> 44 33 22 11
        buff[3] = getBits(val, 31, 24);
        buff[2] = getBits(val, 23, 16);
        buff[1] = getBits(val, 15,  8);
        buff[0] = getBits(val,  7,  0);
#else             // ���ֽ��ڸߵ�ַ
        buff[0] = getBits(val, 31, 24);
        buff[1] = getBits(val, 23, 16);
        buff[2] = getBits(val, 15,  8);
        buff[3] = getBits(val,  7,  0);
#endif
        // CH36X ���õ��Ǵ����ͨ��
        ch36xMemWrite(CH36xMemModel_DWORD/*CH36xMemModel_BYTE*/,
            addr, len, buff);

    } else if(strcmp(arg.action, "mr") ==0)
    {
        // CH36X ���õ��Ǵ����ͨ��
        ch36xMemRead(CH36xMemModel_DWORD/*CH36xMemModel_BYTE*/,
                    addr, len, buff);
#ifdef USING_LITTLE_ENDIAN // ���ֽ��ڵ͵�ַ
        val = word32From4Bytes(buff[3], buff[2], buff[1], buff[0]);
#else
        val = word32From4Bytes(buff[0], buff[1], buff[2], buff[3]);
#endif
        printf("Do mr : reg[%08x] read val[%08x]\n", addr, val);
        printValToFile(arg.outputFile, val);
    }

    if (arg.action[0] == 'e')
    {
        ch36xEEPRomRead();
        if(strcmp(arg.action, "ew") ==0)
        {
            printf("Do ew: reg[%08x] write val[%04x]\n", addr, val);
            if(reg == 0x00)
            {
                vid = (unsigned short)val;
            }else if(reg == 0x01)
            {
                did = (unsigned short)val;
            }else if(reg == 0x02)
            {
                rid = (unsigned char)(val);
            }else if(reg == 0x03)
            {
                svid = (unsigned short)val;
            }else if(reg == 0x04)
            {
                sid = (unsigned short)val;
            }
            ch36xEEPRomWrite();
        } else if(strcmp(arg.action, "er") ==0)
        {
            if(reg == 0x00)
            {
                val = vid;
            }else if(reg == 0x01)
            {
                val = did;
            }else if(reg == 0x02)
            {
                val = (unsigned char)(rid & 0xff);
            }else if(reg == 0x03)
            {
                val = svid;
            }else if(reg == 0x04)
            {
                val = sid;
            }
            printf("Do er: reg[%08x] read val[%04x]\n", addr, val);
            printValToFile(arg.outputFile, val);
        }
    }

    ch36xCloseDevice(index);


    return CMDP_ACT_OK;
}
