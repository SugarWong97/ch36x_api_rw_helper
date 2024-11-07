# if 0
#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
#else

#include "ch36x_api_rw_helper.H"
#include "cmdparser.h"

struct iniConfig
{
    int index;
    char *action;
    char *outputFile;
    char *reg;
    char *val;
} arg;

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
    sscanf(p, "%lx", &ret);
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

    struct ch36xDevEntity dev;

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
    ch36xOpenDevice(&dev, index);
    Sleep(5);
    ch36xMemConfig32BitRW(&dev, true);
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
        printf("val is %lx\n", val);
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
        ch36xMemWrite(&dev, CH36xMemModel_DWORD/*CH36xMemModel_BYTE*/,
            addr, len, buff);

    } else if(strcmp(arg.action, "mr") ==0)
    {
        // CH36X ���õ��Ǵ����ͨ��
        ch36xMemRead(&dev, CH36xMemModel_DWORD/*CH36xMemModel_BYTE*/,
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
        ch36xEEPRomRead(&dev);
        if(strcmp(arg.action, "ew") ==0)
        {
            printf("Do ew: reg[%08x] write val[%04x]\n", addr, val);
            if(reg == 0x00)
            {
                dev.vid = (unsigned short)val;
            }else if(reg == 0x01)
            {
                dev.did = (unsigned short)val;
            }else if(reg == 0x02)
            {
                dev.rid = (unsigned char)(val);
            }else if(reg == 0x03)
            {
                dev.svid = (unsigned short)val;
            }else if(reg == 0x04)
            {
                dev.sid = (unsigned short)val;
            }
            ch36xEEPRomWrite(&dev);
        } else if(strcmp(arg.action, "er") ==0)
        {
            if(reg == 0x00)
            {
                val = dev.vid;
            }else if(reg == 0x01)
            {
                val = dev.did;
            }else if(reg == 0x02)
            {
                val = (unsigned char)(dev.rid & 0xff);
            }else if(reg == 0x03)
            {
                val = dev.svid;
            }else if(reg == 0x04)
            {
                val = dev.sid;
            }
            printf("Do er: reg[%08x] read val[%08x]\n", addr, val);
            printValToFile(arg.outputFile, val);
        }
    }

    ch36xCloseDevice(&dev);


    return CMDP_ACT_OK;
}

#endif
