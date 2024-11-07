#ifndef		_DEBUG368_H
#define		_DEBUG368_H
#include <windows.h>
#include "CH367DLL.h"
#ifdef __cplusplus
extern "C" {
#endif

#define EEPROM_INFO_SIZE 0x20

/*============================= MEM读写 ==============================*/
enum CH36xMemModel
{
     CH36xMemModel_BYTE = 0,
     CH36xMemModel_DWORD,
};

struct ch36xDevEntity {
    unsigned devIndex; /* 设备序号 */
    char isOpened; /* 设备开关标志, 0关 1开 */
    mPCH367_IO_REG mBaseAddr; /* I/O空间寄存器 */
    mPCH368_MEM_REG mMemAddr;/*Mem空间寄存器*/

    // 判断是CH367还是CH368
    int isCH367;
    int isCH368;

    //判断是电平中断（1）还是边沿中断（2）
    int intType;

    unsigned char manufacturerDeviceID[mMAX_BUFFER_LENGTH*3];

    char isReadEERPROM;
    unsigned short vid;// VID(厂商标识 : Vendor ID)
    unsigned short did;// DID(设备标识: Device ID)
    unsigned short rid/* 只有一个字节*/;// RID(芯片版本 : Revision ID)
    unsigned short svid;// SVID(子系统厂商标识 : Subsystem Vendor ID)
    unsigned short sid;// SID(子系统标识 : Subsystem ID)
};


void CH36xShowDevVer(void); //获得驱动版本号

// 指定index打开ch36x设备
bool ch36xOpenDevice(struct ch36xDevEntity * dev, unsigned int devIndex);

// 关闭ch36x设备
int ch36xCloseDevice(struct ch36xDevEntity * dev);

// 配置ch36x设备是否允许32位读写（ch368默认打开此项,ch367不支持此功能）
int ch36xMemConfig32BitRW(struct ch36xDevEntity * dev, int enable);

// 写入ch36x的内存
void ch36xMemWrite(struct ch36xDevEntity * dev, enum CH36xMemModel MemModel, ULONG addr,  ULONG len, unsigned char *writeBuff);//MEM写操作
// 读取ch36x的内存
void ch36xMemRead(struct ch36xDevEntity * dev, enum CH36xMemModel MemModel, ULONG addr,  ULONG len, unsigned char *recvBuff);//MEM读操作

// 目前这个接口写值是原封不动的，因为不能都写0, 因此没有开放写入ID的操作，但测试过是可以正常读写的
void ch36xEEPRomWrite(struct ch36xDevEntity * dev);
int ch36xEEPRomRead(struct ch36xDevEntity * dev);


#ifdef __cplusplus
}
#endif

#endif // _DEBUG367_H
