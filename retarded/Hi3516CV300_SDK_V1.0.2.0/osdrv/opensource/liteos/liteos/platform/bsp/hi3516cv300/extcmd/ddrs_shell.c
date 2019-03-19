#ifdef LOSCFG_SHELL
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "sys/stat.h"
#include "fcntl.h"
#include "sys/mman.h"
#include "shcmd.h"
#include "ctype.h"

#include "linux/kernel.h"


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#define hi3516cv300

#define OPEN_FILE "/dev/mem"
#define DDR_STAT_SIG SIGUSR1

#define PAGE_SIZE_MASK 0xfffff000

#define TIMER_INTERVAL 1

#define DDRC_BASE_ADDR 0x12060000

#define DDRC_MAP_LENGTH 0x20000

#define DDRC0_ADDR 0x00000
#define DDRC1_ADDR 0x10000

#define SA_RESTART   0x10000000

#ifdef hi3516cv300
#define DDRC_TEST_EN 0x1010
#define DDRC_TEST7 0x1270
#define DDRC_TEST8 0x1380
#define DDRC_TEST9 0x1384
#else
#define DDRC_TEST7 0x260
#define DDRC_TEST8 0x264
#define DDRC_TEST9 0x268
#endif


#define reg_read(addr) (*(volatile unsigned int *)(addr))
#define reg_write(addr, val) (*(volatile unsigned int *)(addr) = (val))

static int fd;

static unsigned char *ddrc_base_addr;
static unsigned char *ddrc0_addr;
static unsigned char *ddrc1_addr;

static unsigned int reg_value;
static unsigned int ddrc_num = 0;
static unsigned int bit_width = 32;
static unsigned int ddrc_freq = 400;
static unsigned int timer_interval = TIMER_INTERVAL;

static unsigned int step = 0;

#undef IO_ADDRESS
#define IO_ADDRESS(addr, off) ((unsigned char *)(addr) + off)

static int ddrc_remap(unsigned int num);
static void ddrc_unmap(unsigned int num);
static void print_usage(void);
static int parse_args(int argc, char *argv[], unsigned int *second,
        unsigned int *ddrc, unsigned int *freq, unsigned int *width);

#ifdef hi3516cv300
#define read_ddrc_reg(addr, tmp1, tmp2) do {\
    do {\
        tmp1 = reg_read(IO_ADDRESS(addr, DDRC_TEST_EN));\
        (void)usleep(50);\
    } while (tmp1 & 0x1);\
    ddr_read = reg_read(IO_ADDRESS(addr, DDRC_TEST8));\
    ddr_write = reg_read(IO_ADDRESS(addr, DDRC_TEST9));\
    tmp1 = reg_value & 0xfffffff;\
    tmp1 *= 16;\
    tmp2 = ddr_read + ddr_write;\
} while (0)
#else
#define read_ddrc_reg(addr, tmp1, tmp2) do {\
    do {\
        tmp1 = reg_read(IO_ADDRESS(addr, DDRC_TEST7));\
        (void)usleep(50);\
    } while (tmp1 & (0x1 << 30));\
    ddr_read = reg_read(IO_ADDRESS(addr, DDRC_TEST8));\
    ddr_write = reg_read(IO_ADDRESS(addr, DDRC_TEST9));\
    tmp1 = reg_value & 0xfffffff;\
    tmp1 *= 2;\
    tmp2 = ddr_read + ddr_write;\
} while (0)
#endif

static void ddr_statistic(int n)
{
    unsigned int ddr_read = 0;
    unsigned int ddr_write = 0;
    unsigned int tmp1 = 0;
    double tmp2 = 0.0;
    double ddrc0_rate = 0.0;
    double ddrc1_rate = 0.0;

    if(2 == step)
    {
        return;
    }
    if (0 == ddrc_num) {
        read_ddrc_reg(ddrc0_addr, tmp1, tmp2);
        ddrc0_rate = (tmp2 / tmp1) * 100;

        if (16 == bit_width)
            ddrc0_rate = ddrc0_rate * 2;

        PRINTK("\nddrc0[%0.2f%%]\n", ddrc0_rate);
        reg_write(IO_ADDRESS(ddrc0_addr, DDRC_TEST7), reg_value);
    } else if (1 == ddrc_num) {
        read_ddrc_reg(ddrc1_addr, tmp1, tmp2);
        ddrc0_rate = (tmp2 / tmp1) * 100;
        if (16 == bit_width)
            ddrc0_rate = ddrc0_rate * 2;
        PRINTK("\nddrc1[%0.2f%%]\n", ddrc0_rate);
        reg_write(IO_ADDRESS(ddrc1_addr, DDRC_TEST7), reg_value);
    } else if (2 == ddrc_num) {
        read_ddrc_reg(ddrc0_addr, tmp1, tmp2);
        ddrc0_rate = (tmp2 / tmp1) * 100;
        read_ddrc_reg(ddrc1_addr, tmp1, tmp2);
        ddrc1_rate = (tmp2 / tmp1) * 100;
        if (16 == bit_width) {
            ddrc0_rate = ddrc0_rate * 2;
            ddrc1_rate = ddrc1_rate * 2;
        }
        PRINTK("\nddrc0[%0.2f%%] ddrc1[%0.2f%%]\n", ddrc0_rate, ddrc1_rate);
        reg_write(IO_ADDRESS(ddrc0_addr, DDRC_TEST7), reg_value);
        reg_write(IO_ADDRESS(ddrc1_addr, DDRC_TEST7), reg_value);

    } else
        ;

#ifdef hi3516cv300
    /* enable ddr bandwidth statistic */
    reg_write(IO_ADDRESS(ddrc0_addr, DDRC_TEST_EN), 0x1);
#endif

    return;
}

static int ddrc_remap(unsigned int num)
{
    unsigned int phy_addr_in_page = 0;

    phy_addr_in_page = DDRC_BASE_ADDR & PAGE_SIZE_MASK;

    ddrc_base_addr = (unsigned char *)mmap(NULL, DDRC_MAP_LENGTH,
            PROT_READ | PROT_WRITE, MAP_SHARED,
            fd, phy_addr_in_page);

    if (ddrc_base_addr == MAP_FAILED) {
        PRINTK("ddr%d statistic mmap failed.\n", num);
        return -1;
    }

    ddrc0_addr = ddrc_base_addr + DDRC0_ADDR;
    ddrc1_addr = ddrc_base_addr + DDRC1_ADDR;
    return 0;
}



static void print_usage(void)
{
    PRINTK("NAME\n");
    PRINTK("  ddrs - ddr statistic\n\n");
    PRINTK("DESCRIPTION\n");
    PRINTK("  Statistic percentage of occupation of ddr.\n\n");
    PRINTK("  -d, --ddrc\n");
    PRINTK("      which ddrc you want statistic. \"0\" statistic ddrc0,\n");
    PRINTK("      \"1\" statistic ddrc1, \"2\" statistic ddrc0 and ddrc1 at the same time. \"0\" as default.\n");
    PRINTK("      \"0\" as default.\n");
    PRINTK("  -f, --freq\n");
    PRINTK("      one ddcr freq, one chip, please set the freq referring to the chip.\n");
    PRINTK("      \"400\" as default.\n");
    PRINTK("  -w, --width\n");
    PRINTK("      set the bit-width referring to the chip. \"32\" or \"16\".\n");
    PRINTK("      \"32\" as default.\n");
    PRINTK("  -i, --interval\n");
    PRINTK("      the range is 1~3 second, 1 second as default.\n");
    PRINTK("  -h, --help\n");
    PRINTK("      display this help\n");
    PRINTK("  -q, close\n");
    PRINTK("      close and exit\n");
    PRINTK("  eg:\n");
    PRINTK("      $ hiddrs -d 0 -f 400 -i 1\n");
    PRINTK("      or\n");
    PRINTK("      $ hiddrs\n");
    return;
}

#define check_digit() \
    do {\
        for (k = 0; k < strlen(argv[i + 1]); ++k) {\
            if(0 == isdigit(argv[i + 1][k])) {\
                flags = 1;\
                break;\
            }\
        }\
    } while (0)

static int parse_args(int argc, char *argv[], unsigned int *second,
        unsigned int *ddrc, unsigned int *freq, unsigned int *width)
{
    int i = 0;
    unsigned int k = 0;
    int flags = 0;
    int _ddrc = 0;
    int _second = 0;
    int _freq= 0;
    int _width= 0;

    if ((argc % 2) != 0)
        goto ERROR;

    for (i = 0; i < argc; i += 2) {
        flags = 0;
        if (0 == strcmp("-d", argv[i]) ||
                0 == strcmp("--ddrc",  argv[i])) {
            check_digit();
            if (!flags) {
                _ddrc = strtoul(argv[i + 1],0,0);
                if (_ddrc < 0 || _ddrc > 2)
                    goto ERROR;
                *ddrc = _ddrc;
            } else
                goto ERROR;
        } else if (0 == strcmp("-f", argv[i]) ||
                0 == strcmp("--freq", argv[i])) {
            check_digit();
            if (!flags) {
                _freq = strtoul(argv[i + 1],0,0);
                if (_freq < 0)
                    goto ERROR;
                *freq = _freq;
            }
            else
                goto ERROR;
        } else if (0 == strcmp("-w", argv[i]) ||
                0 == strcmp("--width", argv[i])) {
            check_digit();
            if (!flags) {
                _width = strtoul(argv[i + 1],0,0);
                if ((_width != 32) && (_width != 16))
                    goto ERROR;
                *width = _width;
            }
            else
                goto ERROR;
        } else if (0 == strcmp("-i",  argv[i]) ||
                0 == strcmp("--interval", argv[i])) {
            check_digit();
            if (!flags) {
                _second = strtoul(argv[i + 1],0,0);
                if (_second < 0 || _second > 3)
                    goto ERROR;
                *second = _second;
            } else
                goto ERROR;
        } else if (0 == strcmp("-h",  argv[i]) ||
                0 == strcmp("--help", argv[i])) {
            print_usage();
        } else
            goto ERROR;
    }

    return 0;
ERROR:
    print_usage();
    return -1;
}

static UINT32 hiddrs_config(void)
{
    unsigned int ret = 0;
    unsigned int perf_prd;
    ret = ddrc_remap(ddrc_num);
    if (ret)
        goto ERR1;

#ifdef hi3516cv300
    perf_prd = ddrc_freq * 1000000 / 16;
#else
    perf_prd = ddrc_freq * 1000000 / 4;
#endif

    reg_value = timer_interval * perf_prd;
    reg_value &= 0x0fffffff;

#ifdef hi3516cv300
    /* for 3535 set single trigger,for other chip choose channel */
    reg_value |= 0x10000000;
#else
    reg_value |= 0xd0000000;
#endif

#ifdef hi3516cv300
    /* enable ddr bandwidth statistic */
    reg_write(IO_ADDRESS(ddrc0_addr, DDRC_TEST_EN), 0x0);
#endif
    if (0 == ddrc_num)
        reg_write(IO_ADDRESS(ddrc0_addr, DDRC_TEST7), reg_value);
    else if (1 == ddrc_num)
        reg_write(IO_ADDRESS(ddrc1_addr, DDRC_TEST7), reg_value);
    else if (2 == ddrc_num) {
        reg_write(IO_ADDRESS(ddrc0_addr, DDRC_TEST7), reg_value);
        reg_write(IO_ADDRESS(ddrc1_addr, DDRC_TEST7), reg_value);
    } else
        ;

#ifdef hi3516cv300
    /* enable ddr bandwidth statistic */
    reg_write(IO_ADDRESS(ddrc0_addr, DDRC_TEST_EN), 0x1);
#endif
ERR1:
    return ret;
}

static void hiddrs(void)
{

    PRINTK("===== ddr statistic =====\n");

    while (1){
        (void)sleep(timer_interval);
        if (2 == step)
            break;
        ddr_statistic(0);
    }
    PRINTK("hiddrs end. \n");
    step = 0;
}

/*lint -e528*/
static int cmd_hiddrs(int argc , char* argv[])
{
    static unsigned int hiddrs_taskid;
    UINT32 ret = 0;
    TSK_INIT_PARAM_S hiddrsTask;
    memset(&hiddrsTask, 0, sizeof(TSK_INIT_PARAM_S));

    if((0 == strcmp("close", argv[0])) || \
        (0 == strcmp("-q", argv[0]))) {
        if(1 == step){
            step = 2;

            //(void)LOS_TaskDelete(hiddrs_taskid);
            //step = 0;
        }
    }
    else {
        //second = TIMER_INTERVAL;
        //ddrc_freq = 400;
        ret = (UINT32)parse_args(argc, &argv[0], &timer_interval, &ddrc_num, &ddrc_freq, &bit_width);
        if (ret)
            return ret;
        ret = hiddrs_config();
        if(ret)
            return ret;

        if( 0 == step ){
            hiddrsTask.pfnTaskEntry = (TSK_ENTRY_FUNC)hiddrs;
            hiddrsTask.uwStackSize  = LOSCFG_BASE_CORE_TSK_DEFAULT_STACK_SIZE;
            hiddrsTask.pcName = "hiddrs_task";
            hiddrsTask.usTaskPrio = 8;
            hiddrsTask.uwResved   = LOS_TASK_STATUS_DETACHED;
            ret = LOS_TaskCreate(&hiddrs_taskid, &hiddrsTask);
            PRINTK("hiddrs begin. \n");

            if (LOS_OK != ret ) {
                PRINTK("hiddrs_task create failed ! \n");
                return -1;
            }
            step = 1;
        }
    }
    return ret;
}
SHELLCMD_ENTRY(hiddrs_shellcmd, CMD_TYPE_EX,"hiddrs",0,(CMD_CBK_FUNC)cmd_hiddrs);/*lint !e19 */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
#endif
