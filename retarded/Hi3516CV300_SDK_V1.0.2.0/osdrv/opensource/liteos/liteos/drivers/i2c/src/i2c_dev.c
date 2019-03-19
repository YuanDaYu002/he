#include "linux/kernel.h"
#include "linux/i2c.h"
#include "stdio.h"

#include "i2c.h"
#include "errno.h"
#include "los_mux.h"

#ifdef LOSCFG_FS_VFS
#include "linux/jiffies.h"
#include "linux/fs.h"

#ifndef MAX_JIFFY_OFFSET
#define MAX_JIFFY_OFFSET    ((LONG_MAX >> 1)-1)
#endif
#ifndef MSEC_PER_SEC
#define MSEC_PER_SEC        1000L
#endif
#ifndef HZ
#define HZ          LOSCFG_BASE_CORE_TICK_PER_SECOND
#endif

static unsigned long msecs_to_jiffies(const unsigned int m)
{
    if ((int)m < 0)
        return MAX_JIFFY_OFFSET;

    return (m + (MSEC_PER_SEC / HZ) - 1) / (MSEC_PER_SEC / HZ);
}

ssize_t i2cdev_read(struct file * filep, char __user *buf, size_t count)
{
    int ret;
    struct inode * inode = filep -> f_inode ;
    struct i2c_client * client = inode->i_private;

    if (buf == NULL) {
        PRINT_ERR("buf is null!\n");
        return -EINVAL;
    }
    ret = i2c_master_recv(client, buf, count);

    return ret;
}

ssize_t i2cdev_write(struct file * filep , const char __user *buf,
        size_t count)
{
    int ret;
    struct inode * inode = filep -> f_inode ;
    struct i2c_client * client = inode->i_private;

    if (buf == NULL) {
        PRINT_ERR("buf is null!\n");
        return -EINVAL;
    }
    ret = i2c_master_send(client, buf, count);

    return ret;
}

static int i2cdev_ioctl_rdwr(struct i2c_client *client,
        unsigned int arg)
{
    struct i2c_rdwr_ioctl_data *rdwr_arg = NULL;
    struct i2c_msg *rdwr_pa = NULL;
    int res = 0;
    int i;
    rdwr_arg = (struct i2c_rdwr_ioctl_data *)arg;
    if(!rdwr_arg) {
        PRINT_ERR("Invalid argument!\n");
        return -EINVAL;
    }
    if (rdwr_arg->nmsgs > I2C_RDWR_IOCTL_MAX_MSGS)
        return -EINVAL;
    rdwr_pa = rdwr_arg->msgs;
    if(!rdwr_pa) {
        PRINT_ERR("Invalid argument!\n");
        return -EINVAL;
    }
    for (i = 0; i < rdwr_arg->nmsgs; i++) {
        if (rdwr_pa[i].len > 8192) {
            res = -EINVAL;
            break;
        }
    }
    res = i2c_transfer(client->adapter, rdwr_pa, rdwr_arg->nmsgs);
    return res;
}

static int i2cdev_ioctl(struct file * filep, int cmd, unsigned long arg)
{
    struct inode * inode = filep -> f_inode;
    struct i2c_client * client = inode->i_private;
    unsigned int ret = ENOERR;

    switch (cmd) {
        case I2C_SLAVE:
        case I2C_SLAVE_FORCE:
            if ((arg > 0x3ff) ||
                (((client->flags & I2C_M_TEN) == 0) && arg > 0xfe)) {
                PRINT_ERR("Not support arg(%0d)!!!", arg);
                ret = -EINVAL;
                break;
            }
            client->addr = arg;
            break;
        case I2C_TENBIT:
            if (arg == 1)
                client->flags |= I2C_M_TEN;
            else if (arg == 0)
                client->flags &= ~I2C_M_TEN;
            else {
                PRINT_ERR("error device width! arg = %d\n", arg);
                ret = -EINVAL;
            }
            break;
        case I2C_PEC:
            if (arg)
                client->flags |= I2C_CLIENT_PEC;
            else
                client->flags &= ~I2C_CLIENT_PEC;
            break;
        case I2C_RDWR:
            if (arg == (unsigned long)NULL) {
                PRINT_ERR("arg is null!\n");
                ret = -EINVAL;
                break;
            }
            ret = i2cdev_ioctl_rdwr(client, arg);
            break;
        case I2C_16BIT_REG:
            if (arg == 1)
                client->flags |= I2C_M_16BIT_REG;
            else if (arg == 0)
                client->flags &= ~I2C_M_16BIT_REG;
            else {
                PRINT_ERR("error reg width! arg = %d\n", arg);
                ret = -EINVAL;
            }
            break;
        case I2C_16BIT_DATA:
            if (arg == 1)
                client->flags |= I2C_M_16BIT_DATA;
            else if (arg == 0)
                client->flags &= ~I2C_M_16BIT_DATA;
            else {
                PRINT_ERR("error data width! arg = %d\n", arg);
                ret = -EINVAL;
            }
            break;
        case I2C_RETRIES:
            client->adapter->retries = arg;
            break;
        case I2C_TIMEOUT:
            client->adapter->timeout = msecs_to_jiffies(arg * 10);
            break;
        default:
            PRINT_ERR("Not support cmd(%0d)!!!", cmd);
            ret = -EINVAL;
    }
    return ret;
}

static int i2cdev_open(struct file *filep)
{
    return 0;
}

static int i2cdev_release(struct file *filep)
{
    return 0;
}

static const struct file_operations_vfs i2c_dev_fops =
{
    i2cdev_open,
    i2cdev_release,
    i2cdev_read,
    i2cdev_write,
    NULL,
    i2cdev_ioctl,
#ifndef CONFIG_DISABLE_POLL
    NULL,
#endif
    NULL
};
#endif /* LOSCFG_FS_VFS */

int i2c_dev_init(void)
{
    int ret = 0, res = 0;
    struct i2c_client * client = NULL;
    int adapter_index = 0;
    int index = 0;

    /* init adapter */
    ret = i2c_adapter_init();
    if (ret < 0) {
        PRINT_ERR("Fail to init i2c_adapter!\n");
        goto out;
    }

#ifdef LOSCFG_FS_VFS
    /* create i2c client for adatper */
    for (; index < LOSCFG_I2C_ADAPTER_COUNT; index++) {
        adapter_index = index;
        client = malloc(sizeof(struct i2c_client));
        if (!client) {
            PRINT_ERR("Fail to malloc i2c_client-%d!\n",adapter_index);
            ret = -1;
            goto out;
        }
        memset(client,0,sizeof(struct i2c_client));
        snprintf(client->name, 12, "/dev/i2c-%d", adapter_index);

        /* attach client to adapter */
        res = client_attach(client, adapter_index);
        if (res) {
            PRINT_WARN("Fail to attach client-%d!\n",adapter_index);
            free(client);
            continue;
        }

        res = register_driver(client->name, &i2c_dev_fops,
                0666, client);
        if (res) {
            PRINT_ERR("Fail to register %s driver!!!\n",
                    client->name);
            ret = -1;
            goto free_client;
        }
    }
#endif /* LOSCFG_FS_VFS */
    return 0;
free_client:
    free(client);
out:
    return ret;
}
