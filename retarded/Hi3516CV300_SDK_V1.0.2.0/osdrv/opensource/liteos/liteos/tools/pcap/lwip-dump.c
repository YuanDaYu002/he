#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netif/etharp.h>
#include <pcap.h>
#include "lwip/ip.h"
#include <errno.h>


#ifdef LOSCFG_SHELL

#include "shcmd.h"

static UINT32 s_lwipDumpTskHandle;
static pcap_t *g_handle = NULL;        /* Session handle */
static void print_help(void)
{
    PRINTK("USAGE:\ntcpdump -i ifname -w \"path\" [-c \"package-count\"] [\"filter expression\"]\nexample1: tcpdump -i eth0 -w /ramfs/cap -c 15\nexample2: tcpdump -i eth0 -w /ramfs/cap -c 15 \"arp or ip\"\nexample3: tcpdump stop\n");
}

static int dump_main(int argc, char **argv)
{
    int i=0,ret;
    pcap_dumper_t *pd;
    char *dev = NULL;        /* Device to sniff on */
    char errbuf[PCAP_ERRBUF_SIZE];    /* Error string */
    struct bpf_program fp;        /* The compiled filter expression */
    char *filter_exp = "";    /* The filter expression */
    bpf_u_int32 mask = 0;        /* The netmask of our sniffing device */
    bpf_u_int32 net = 0;        /* The IP of our sniffing device */
    char *filename = NULL;
    int pcount = 0;
    int promiscuous = 1;

    while(i < argc)
    {
        if(strcmp(argv[i], "-h") == 0)
        {
            print_help();
            return -1;
        }

        if(strcmp(argv[i], "-p") == 0)//Don't  put  the interface into promiscuous mode.
        {
            PRINTK("promiscuous mode off!\n");
            i += 1;
            promiscuous = 0;
            continue;
        }

        if(strcmp(argv[i], "-c") == 0 && ((i + 1) < argc))
        {
            pcount = atoi(argv[i+1]);
            PRINTK("count: %d\n",pcount);
            i += 2;
            continue;
        }

        if(strcmp(argv[i], "-i") == 0 && ((i + 1) < argc))
        {
            dev = (s8_t *)argv[i+1];
            PRINTK("interface: %s\n",dev);
            i += 2;
            continue;
        }

        if(strcmp(argv[i], "-w") == 0 && ((i + 1) < argc))
        {
            filename = (s8_t *)argv[i+1];
            PRINTK("filename: %s\n",filename);
            i += 2;
            continue;
        }

        if((i + 1) == argc)
        {
            filter_exp = argv[i];
            PRINTK("filter: %s\n",filter_exp);
            break;
        }

        print_help();
        return -1;
    }

    if((filename == NULL) || (dev == NULL))
    {
        print_help();
        return -1;
    }

    g_handle = pcap_open_live(dev, BUFSIZ, promiscuous, 1000, errbuf);
    if (g_handle == NULL) {
        PRINTK("Couldn't open device %s: %s\n", dev, errbuf);
        return (-1);
    }

    if (pcap_compile(g_handle, &fp, filter_exp, 0, net) == -1) {
        PRINTK("Couldn't parse filter %s: %s\n", filter_exp, pcap_geterr(g_handle));
        pcap_close(g_handle);
        g_handle = NULL;

        return(-1);
     }
    if (pcap_setfilter(g_handle, &fp) == -1) {
        PRINTK("Couldn't install filter %s: %s\n", filter_exp, pcap_geterr(g_handle));
        pcap_close(g_handle);
        g_handle = NULL;

        return(-1);
    }

    if ((pd = pcap_dump_open(g_handle,filename)) == NULL) {
        /*
         * Print out error message if pcap_dump_open failed. This will
         * be the below message followed by the pcap library error text,
         * obtained by pcap_geterr().
         */
        PRINTK("Error opening savefile \"%s\" for writing: %s\n",
                filename, pcap_geterr(g_handle));
        pcap_close(g_handle);
        g_handle = NULL;

        return(-1);
    }
    // start packet processing loop, just like live capture
    if ((ret=pcap_loop(g_handle, pcount, (pcap_handler)pcap_dump, (char *)pd)) < 0) {
        pcap_dump_close(pd);
        pcap_close(g_handle);
        g_handle = NULL;
        if(ret==PCAP_ERROR_BREAK)
        {
            PRINTK("tcpdump stoped.\n");
        }
        return -1;
    }

    pcap_dump_close(pd);
    pcap_close(g_handle);
    g_handle = NULL;
    PRINTK("tcpdump file saved.\n");
    return 0;
}

static void dump_task(unsigned int p0, unsigned int p1, unsigned int p2, unsigned int p3)
{
    int i = 0;
    unsigned int argc = p0;
    char **argv = (char **)p1;
    dump_main(argc,argv);
    for(i=0;i<argc;i++) free(argv[i]);
    free(argv);
}


static UINT32 osShellCmdDump(UINT32 argc, CHAR **argv)
{
    UINT32 uwRet = 0, i = 0;
    TSK_INIT_PARAM_S os_task_init_param;
    char **args_buf = NULL;

    if(argc == 0)
    {
        print_help();
        return 0;
    }
    if((argc == 1) && (strcmp(argv[0],"stop")==0))
    {
        if(g_handle!=NULL)
            pcap_breakloop(g_handle);
        return 0;
    }

    if(g_handle!=NULL)
    {
        PRINTK("tcpdump is running.\n");
        return 0;
    }

    args_buf = (char**)zalloc((argc)*sizeof(char**));
    if(args_buf == NULL) return -1;
    for(i=0;i<argc;i++) args_buf[i]=strdup(argv[i]);

    memset(&os_task_init_param,0,sizeof(os_task_init_param));
    os_task_init_param.pfnTaskEntry = (TSK_ENTRY_FUNC)dump_task;
    os_task_init_param.uwStackSize  = 0x30000;
    os_task_init_param.pcName       = "tcpdump";
    os_task_init_param.usTaskPrio   = 4;
    os_task_init_param.uwResved   = LOS_TASK_STATUS_DETACHED;
    os_task_init_param.auwArgs[0] = argc;
    os_task_init_param.auwArgs[1] = (UINT32)args_buf;
    uwRet = LOS_TaskCreate(&s_lwipDumpTskHandle, &os_task_init_param);

    return uwRet;

}

SHELLCMD_ENTRY(lwip_dump_shellcmd, CMD_TYPE_EX, "tcpdump", 0, (CMD_CBK_FUNC)osShellCmdDump); /*lint !e19*/

#endif


