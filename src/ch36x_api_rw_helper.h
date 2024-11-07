#ifndef		_DEBUG368_H
#define		_DEBUG368_H
#include <windows.h>
#ifdef __cplusplus
extern "C" {
#endif


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



#define CH367_GPO_SET do{CH367mReadIoByte(mIndex, &mBaseAddr->mCH367IoPort[0xF1], &qbyte); CH367mWriteIoByte(mIndex, &mBaseAddr->mCH367IoPort[0xF1], qbyte |= 0x80);}while(0)
#define CH367_GPO_CLR do{CH367mReadIoByte(mIndex, &mBaseAddr->mCH367IoPort[0xF1], &qbyte); CH367mWriteIoByte(mIndex, &mBaseAddr->mCH367IoPort[0xF1], qbyte &= 0x7F);}while(0)
#define WM_INTNOTIFY (WM_USER + 1)

void CALLBACK InterruptEvent(void); // �豸�ж�֪ͨ��Ϣ 
void AddrRefresh(HWND hDialog); // ����ȡ��ַ
void mSetI2C(HWND hDialog); // I2C��д
void mIoWrite(HWND hDialog,int IoModel,int IoAddr);//IO������
void mIoRead(HWND hDialog,int IoModel,int IoAddr);//IOд����
void mMemRead(HWND hDialog,int MemModel);//MEM������
void mMemWrite(HWND hDialog,int MemModel);//MEM������
void mConRead(HWND hDialog);//���ÿռ��ֽڶ�
void mConWrite(HWND hDialog);//���ÿռ��ֽ�д
void mI2CRead(HWND hDialog);//I2C��
void mI2CWrite(HWND hDialog);//I2Cд
void mInitCheckBox(HWND hDialog); // ��ʼ��Ĭ��ѡ�е�ѡ�� 
void mShowDevVer(HWND hDialog); //��ʾ�����汾��


void mPreVerify(HWND hwnd);

int ch36xCloseDevice(int index);

#ifdef __cplusplus
}
#endif

#endif // _DEBUG367_H
