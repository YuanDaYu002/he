/*----------------------------------------------------------------------------
 * Copyright (c) <2013-2015>, <Huawei Technologies Co., Ltd>
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 * conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 * of conditions and the following disclaimer in the documentation and/or other materials
 * provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific prior written
 * permission.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *---------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------
 * Notice of Export Control Law
 * ===============================================
 * Huawei LiteOS may be subject to applicable export control laws and regulations, which might
 * include those applicable to Huawei LiteOS of U.S. and the country in which you are located.
 * Import, export and usage of Huawei LiteOS in any manner by you shall be in compliance with such
 * applicable export control laws and regulations.
 *---------------------------------------------------------------------------*/

#include "linux/seq_file.h"
#include "proc_fs.h"
#include "sys/statfs.h"
#include "sys/mount.h"
#include "stdio.h"
#include "disk.h"
#include "mmc_core.h"
#include "himci.h"

#ifdef LOSCFG_FS_PROC

#define MCI_PARENT       "mci"
#define MCI_STATS_PROC   "mci_info"

static struct proc_dir_entry *proc_mci_dir;


static int show(FAR const char *mountpoint, FAR struct statfs *statbuf, FAR void *arg)
{
    struct seq_file *m = (struct seq_file *)arg;
    char *ftype = (char *)NULL;
    char *fname = (char *)NULL;

    switch(statbuf->f_type)
    {
        case PROCFS_MAGIC:
            ftype = "proc";
            fname = "proc";
            break;
        case JFFS2_SUPER_MAGIC:
            ftype = "jffs";
            fname = "jffs";
            break;
        case YAFFS_MAGIC:
            ftype = "yaffs";
            fname = "yaffs";
            break;
        case NFS_SUPER_MAGIC:
            ftype = "nfs";
            fname = "nfs";
            break;
        case RAMFS_MAGIC:
            ftype = "ramfs";
            fname = "ramfs";
            break;
        case MSDOS_SUPER_MAGIC:
            ftype = "vfat";
            fname = "fat";
            break;
        default:
            return 0;
    }
    (void)seq_printf(m,"%s %s %s %s %d %d\n",fname, mountpoint, ftype, 0, 0, 0);

    return 0;
}

#define MAX_CARD_TYPE   4
#define MAX_SPEED_MODE  5


static char *card_type[MAX_CARD_TYPE + 1] = {
    "unknown"
    "MMC card",
    "SD card",
    "SDIO card",
    "SD combo (IO+mem) card",
};
static char *clock_unit[4] = {
    "Hz",
    "KHz",
    "MHz",
    "GHz"
};


static char *mci_get_card_type(unsigned int sd_type)
{
    if (MAX_CARD_TYPE <= sd_type)
        return card_type[0];
    else
        return card_type[sd_type];
}

static unsigned int analyze_clock_scale(unsigned int clock,
        unsigned int *clock_val)
{
    unsigned int scale = 0;
    unsigned int tmp = clock;

    while (1) {
        tmp = tmp / 1000;
        if (0 < tmp) {
            *clock_val = tmp;
            scale++;
        } else {
            break;
        }
    }
    return scale;
}

static void mci_stats_seq_printout(struct seq_file *s)
{
    unsigned int index_mci;
    unsigned int clock;
    unsigned int clock_scale;
    unsigned int clock_value = 0;
    const char *type;
    struct mmc_host *mmc = NULL;
    struct mmc_card  *card = NULL;
    struct himci_host *host = NULL;
    struct mmc_iocfg *iocfg = NULL;
    const char *uhs_bus_speed_mode = "";
    unsigned int speed_class, grade_speed_uhs;
    static const char *const uhs_speeds[] = {
        [SD_MODE_UHS_SDR12] = "SDR12 ",
        [SD_MODE_UHS_SDR25] = "SDR25 ",
        [SD_MODE_UHS_SDR50] = "SDR50 ",
        [SD_MODE_UHS_SDR104] = "SDR104 ",
        [SD_MODE_UHS_DDR50] = "DDR50 ",
    };

    for (index_mci = 0; index_mci < MAX_MMC_NUM; index_mci++) {
        mmc = get_mmc_host(index_mci);
        if (NULL == mmc) {
            seq_printf(s, "\nMCI%d invalid\n", index_mci);
            continue;
        } else {
            seq_printf(s, "\nMCI%d", index_mci);
        }
        card = mmc->card_cur;

        if (!card) {
            seq_printf(s, ": unplugged");
        } else {
            seq_printf(s, ": pluged");

            if (!is_card_present(card)) {
                seq_printf(s, "_disconnected\n");
            } else {
                seq_printf(s, "_connected\n");
                seq_printf(s,
                        "\tType: %s",
                        mci_get_card_type(get_card_type(card)));

                if (is_card_blkaddr(card)) {
                    if (is_card_ext_capacity(card))
                        type = "SDXC";
                    else
                        type = "SDHC";
                }
                seq_printf(s, "(%s)\n", type);
                //iocfg = card->iocfg;
                if (is_card_uhs(card) && (card->bus_speed_mode <  ARRAY_SIZE(uhs_speeds)))
                    uhs_bus_speed_mode = uhs_speeds[card->bus_speed_mode];

                seq_printf(s, "\tMode: %s%s%s%s\n",
                        is_card_uhs(card) ? "UHS" :
                        (is_card_highspeed(card) ? "HS ": ""),
                        (is_card_hs200(card)? "HS200 ": ""),
                        (is_card_ddr_mode(card) ? "DDR": "",
                         uhs_bus_speed_mode));

                speed_class = card->card_reg.ssr.speed_class;
                grade_speed_uhs = card->card_reg.ssr.uhs_speed_grade;
                seq_printf(s, "\tSpeed Class: Class %s\n",
                        (0x00 == speed_class) ? "0":
                        (0x01 == speed_class) ? "2":
                        (0x02 == speed_class) ? "4":
                        (0x03 == speed_class) ? "6":
                        (0x04 == speed_class) ? "10":
                        "Reserved");
                seq_printf(s, "\tUhs Speed Grade: %s\n",
                        (0x00 == grade_speed_uhs)?
                        "Less than 10MB/sec(0h)" :
                        (0x01 == grade_speed_uhs)?
                        "10MB/sec and above(1h)":
                        "Reserved");

                host = mmc->priv;

                clock = host->hclk;
                clock_scale = analyze_clock_scale(clock, &clock_value);
                seq_printf(s, "\tHost work clock: %d%s\n",
                        clock_value, clock_unit[clock_scale]);

                seq_printf(s, "\tCard support clock: %d%s\n",
                        clock_value, clock_unit[clock_scale]);

                seq_printf(s, "\tCard work clock: %d%s\n",
                        clock_value, clock_unit[clock_scale]);

                seq_printf(s, "\tCard error count: %d\n",
                        host->error_count);
            }
        }
    }
}

static int himci_proc_show(struct seq_file *m, void *v)
{
    mci_stats_seq_printout(m);
    return 0;
}

static int himci_proc_open(struct inode *inode_ptr, struct proc_file *pf)
{
    return single_open(pf, himci_proc_show, NULL);
}

static const struct proc_file_operations himci_proc_fops = {
    .open       = himci_proc_open,
    .read       = seq_read,
    .llseek     = seq_lseek,
    .release    = single_release,
    .write      = NULL, /*lint !e64*/
};

int himci_proc_init(void)
{
    struct proc_dir_entry *proc_stats_entry;
    struct proc_dir_entry *pHandle;

    proc_mci_dir = proc_mkdir(MCI_PARENT, NULL);
    if (!proc_mci_dir) {
        PRINT_ERR("creat dir error!\n");
        return 1;
    }

    pHandle = create_proc_entry(MCI_STATS_PROC, 0, (struct proc_dir_entry *)proc_mci_dir);
    if (pHandle == NULL) {
        PRINT_ERR("creat mounts error!\n");
        return 1;
    }

    pHandle->proc_fops = &himci_proc_fops;
    return 0;
}

#else

int himci_proc_init(void)
{
    return 0;
}
#endif
