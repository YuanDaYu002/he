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

#include "fcntl.h"
#include "sched.h"
#include "errno.h"
#include "assert.h"
#include "sys/ioctl.h"
#ifdef CONFIG_FILE_MODE
#include "stdarg.h"
#endif
#include "stdlib.h"
#include "stdio.h"
#include "local.h"
#include "inode/inode.h"
#include "console.h"
#include "linux/wait.h"

#include "uart.h"

extern UINT32           g_uwShellTask;
/*lint -e534*/
/* acquire uart driver function and filep of /dev/console,
 * then store uart driver function in *pst_filep_ops
 * and store filep of /dev/console in *pst_filep.
 */
int get_filep_ops(struct file *filep, struct file **pst_filep, const struct file_operations_vfs **pst_filep_ops )
{
    int ret = 0;

    if (!filep || !(filep->f_inode) || !(filep->f_inode->i_private))
    {
        ret = ENOMEM;
        return -ret;
    }

    /* to find console device's filep(now it is *pst_filep) throught i_private */
    *pst_filep = (struct file *)filep->f_inode->i_private;

    if (!((*pst_filep)->f_inode) || !((*pst_filep)->f_inode->u.i_ops))
    {
        ret = ENOMEM;
        return -ret;
    }

    /*to find uart driver operation function throutht u.i_opss */
    *pst_filep_ops = (*pst_filep)->f_inode->u.i_ops;

    return ENOERR;
}

static int console_open(struct file *filep)
{
    int ret = 0;
    struct file *pstfilep = NULL;
    const struct file_operations_vfs *pstfops = NULL;

    ret = get_filep_ops(filep, &pstfilep, &pstfops);
    if (ret != ENOERR)
    {
        return -EPERM;
    }
    if (!pstfops->open)
    {
        ret = ENOMEM;
        return -ret;
    }

    /* adopt uart open function to open pstfilep (pstfilep is
     * corresponding to filep of /dev/console)
     */
    ret = pstfops->open(pstfilep);
    if (ret != 0)
    {
        return -EPERM;
    }
    return ENOERR;
}

static int console_close(struct file *filep)
{
    return ENOERR;
}

static ssize_t console_read(struct file *filep, char *buffer, size_t buflen)
{
    int ret = 0;
    int len = 0;
    struct file *pstfilep = NULL;
    const struct file_operations_vfs *pstfops = NULL;

    ret = get_filep_ops(filep, &pstfilep, &pstfops);
    if (ret != ENOERR)
    {
        return -EPERM;
    }

    if (NULL == pstfops->read)
    {
        return -ENOMEM;
    }
    /* adopt uart open function to read data from pstfilep
     * and write data to buffer (pstfilep is
     * corresponding to filep of /dev/console)
     */
    /* if the user is reading data,uw_ShellTask is Suspend */
#ifdef LOSCFG_SHELL
    LOS_TaskSuspend(g_uwShellTask);
#endif
    FLOCKFILE(stdin);
    while (len < (int)buflen)
    {
        pstfops->read(pstfilep, &buffer[len], 1);
        if ('\r' == buffer[len] || '\n' == buffer[len])
        {
            buffer[len] = '\r';
        }
        pstfops->write(pstfilep, &buffer[len], 1);

        if ('\r' == buffer[len])
        {
            buffer[len] = '\n';
            pstfops->write(pstfilep, &buffer[len], 1);
            FUNLOCKFILE(stdin);
#ifdef LOSCFG_SHELL
            LOS_TaskResume(g_uwShellTask);
#endif
            return len + 1;
        }
        len++;
    }
    FUNLOCKFILE(stdin);
#ifdef LOSCFG_SHELL
    LOS_TaskResume(g_uwShellTask);
#endif
    return len;
}

static ssize_t console_write(struct file *filep, const char *buffer, size_t buflen)
{
    int ret = 0;
    int cnt = 0;
    struct file *pstfilep = NULL;
    const struct file_operations_vfs *pstfops = NULL;

    ret = get_filep_ops(filep, &pstfilep, &pstfops);
    if (ret != ENOERR)
    {
        return -EPERM;
    }
    if (!pstfops->write)
    {
        ret = ENOMEM;
        return -ret;
    }
    /* adopt uart open function to read data from buffer
     * and write data to pstfilep (pstfilep is
     * corresponding to filep of /dev/console)
     */
    while (cnt < (int)buflen)
    {
        if ('\n' == buffer[cnt])
        {
            pstfops->write(pstfilep, "\r", 1);
        }
        pstfops->write(pstfilep, &buffer[cnt], 1);
        cnt++;
    }
    return cnt;
}

static ssize_t console_ioctl(struct file *filep, int cmd, unsigned long arg)
{
    int ret = 0;
    struct file *pstfilep = NULL;
    const struct file_operations_vfs *pstfops = NULL;

    ret = get_filep_ops(filep, &pstfilep, &pstfops);
    if (ret != ENOERR)
    {
        return -EPERM;
    }

    if (NULL == pstfops->ioctl)
    {
        return -ENOMEM;
    }

    ret = pstfops->ioctl(pstfilep, cmd, arg);
    return ret;
}

/* console device driver function structure */
static const struct file_operations_vfs  console_dev_ops =
{
    console_open,                 /* open */
    console_close,                /* close */
    console_read,                 /* read */
    console_write,                /* write */
    NULL,
    console_ioctl,
#ifndef CONFIG_DISABLE_POLL
    NULL,
#endif
    NULL
};

/* Initialized console control platform so that when we operate /dev/console
 * as if we are operating /dev/ttyS0 (uart0).
 */
static int console_init(const char *device_name)
{
    int ret = 0;
    struct file *filep = NULL;
    char *fullpath = NULL;
    FAR struct inode *inode = NULL;/*lint !e578*/

    /* allocate memory for filep,in order to unchange the value of filep */
    filep = (struct file *)malloc(sizeof(struct file));
    if (!filep)
    {
        set_errno(ENOMEM);
        return VFS_ERROR;
    }

    /* Adopt procedure of open function to allocate 'filep' to /dev/console */
    ret = vfs_normalize_path(NULL, device_name, &fullpath);
    if (ret < 0)
    {
        free(filep);/*lint !e424*/
        ret = -ret;
        set_errno(ret);
        return VFS_ERROR;
    }

    inode = inode_find(fullpath, NULL);
    if (!inode)
    {
        ret = ENOENT;
        goto errout;
    }


    /* initialize the console filep which is associated with /dev/console,
     * assign the uart0 inode of /dev/ttyS0 to console inod of /dev/console,
     * then we can operate console's filep as if we operate uart0 filep of
     * /dev/ttyS0.
     */
    filep->f_oflags = O_RDWR;
    filep->f_pos    = 0;
    filep->f_inode  = inode;
    filep->f_path   = fullpath;
    filep->f_priv   = NULL;

    if (inode->u.i_ops->open)
    {
        inode->u.i_ops->open(filep);
    }

    /* Use filep to connect console and uart, we can find uart driver function throught filep.
     * now we can operate /dev/console to operate /dev/ttyS0 through filep.
     */
    register_driver(CONSOLE, &console_dev_ops, 0666, (void *)filep);
    inode_release(inode);
    return ENOERR;
errout:
    free(fullpath);
    free(filep);/*lint !e424*/
    set_errno(ret);
    return VFS_ERROR;
}

/* Initialize stdin stdout stderr and return stdin fd,stdout fd,stderr fd */
int stdio_fd_init(int fd, char * fullpath)
{
    struct file *filep = NULL;
    FAR struct inode *inode = NULL;/*lint !e578*/

    inode = inode_find(fullpath, NULL);
    if (!inode)
    {
        goto errout_with_inode;
    }

    /*assignment values to tg_filelist so as to initialize stdin stdout stderr fd */
    tg_filelist.fl_files[fd].f_oflags = O_RDWR;
    tg_filelist.fl_files[fd].f_pos    = 0;
    tg_filelist.fl_files[fd].f_inode  = inode;
    tg_filelist.fl_files[fd].f_priv   = NULL;
    tg_filelist.fl_files[fd].f_path   = NULL; /* The mem will free in close(fd) */

    filep = &tg_filelist.fl_files[fd];

    inode_release(inode);
    return fd;

errout_with_inode:
    inode_release(inode);
    return VFS_ERROR;
}


/* Initialized system console and return stdinfd stdoutfd stderrfd */
int system_console_init(const char *tty_name)
{
    int ret = 0;
    ret = console_init(tty_name);
    if (0 != ret)
    {
        PRINT_ERR("%s, %d",__func__, __LINE__);/*lint !e40*/
        goto errout;
    }

    /* return three fd, 0 1 2
     * allocate default value to filep for 0,1,2
     * then,stdin,stdout,stderr can be used
     * these operating procedure is corresponding to open
     * /dev/console third times.
     */
    /* stdin fd (0) init, return stdin fd = 0 */
    stdio_fd_init(STDIN, CONSOLE);

    /* stdout fd (1) init, return stdout fd = 1 */
    stdio_fd_init(STDOUT, CONSOLE);

    /* stderr fd (2) init, return stderr fd = 2 */
    stdio_fd_init(STDERR, CONSOLE);

    (void)ioctl(STDIN, UART_CFG_RD_BLOCK, UART_RD_BLOCK);
    (void)ioctl(STDIN, UART_CFG_BAUDRATE, CONSOLE_UART_BAUDRATE);

    return ret;

errout:
    set_errno(ret);
    return VFS_ERROR;
}


#ifdef LOSCFG_NET_TELNET
BOOL set_uartdev_nonblock(void)
{
    int ret = 0;

    ret = ioctl(STDIN, UART_CFG_RD_BLOCK, UART_RD_NONBLOCK);
    if (ret != 0)
    {
        return 0;
    }

    return 1;
}

BOOL set_uartdev_block(void)
{
    int ret = 0;

    ret = ioctl(STDIN, UART_CFG_RD_BLOCK, UART_RD_BLOCK);
    if (ret != 0)
    {
        return 1;
    }

    return 0;
}
#endif //LOSCFG_NET_TELNET

/*restore*/
