
#ifndef _LINUX_I2C_H
#define _LINUX_I2C_H

#include <linux/types.h>
#include <linux/device.h>
#include <linux/sched.h>
#include <linux/mutex.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#define I2C_SMBUS_BLOCK_MAX    32    /* As specified in SMBus standard */
#define I2C_NAME_SIZE    20

struct module;
struct i2c_adapter;
union i2c_smbus_data;

struct i2c_device_id {
};

struct i2c_msg {
    u_short addr;    /* slave address */
    u_short flags;
#define I2C_M_REV_DIR_ADDR  0x2000
#define I2C_M_RECV_LEN      0x0400
#define I2C_M_IGNORE_NAK    0x1000
#define I2C_M_NO_RD_ACK     0x0800
#define I2C_M_NOSTART       0x4000
#define I2C_M_16BIT_DATA    0x0008
#define I2C_M_16BIT_REG     0x0002
#define I2C_M_TEN           0x0010
#define I2C_M_RD            0x0001
#define I2C_M_STOP          0x8000
    u_short len;      /* msg length */
    u_char *buf;      /* pointer to msg data */
};

/*
 * i2c_algorithm is the interface to hardware solutions using the same bus algorithms
 */
struct i2c_algorithm {
    /* determine the adapter */
    u_long (*functionality) (struct i2c_adapter *);
    int (*smbus_xfer) (struct i2c_adapter *adap, u_short addr,
               unsigned short flags, char read_write,
               u_char command, int size, union i2c_smbus_data *data);

    int (*master_xfer)(struct i2c_adapter *adap, struct i2c_msg *msgs,
               int num);
};

/*
 * i2c_adapter is the structure used to identify a physical i2c bus along
 * with the access algorithms necessary to access it.
 */
struct i2c_adapter {
    int nr;
    char name[48];
    const struct i2c_algorithm *algo; /* the algorithm to access the bus */
    void *algo_data;
    struct module *owner;
    unsigned int adapter_lock;

    struct list_head userspace_clients;

    int retries;
    int timeout;            /* in jiffies */
    struct device dev;        /* the adapter device */
};

/* struct i2c_client - represent an I2C slave device */
struct i2c_client {
    unsigned short flags;               /* div., see below */
    unsigned short addr;               /* chip address - NOTE: 7bit */
    char name[I2C_NAME_SIZE];
    struct i2c_adapter *adapter;    /* the adapter we sit on */
    struct i2c_driver *driver;          /* and our access routines */
    struct device dev;                   /* the device structure */
    int irq;                                   /* irq issued by device */
    struct list_head detected;
};

union i2c_smbus_data {
    u_char byte;
    u_short word;
    u_char block[I2C_SMBUS_BLOCK_MAX + 2];    /* block[0] is used for length */
};

/* struct i2c_board_info - template for device creation */
struct i2c_board_info {
    char        type[I2C_NAME_SIZE];
    unsigned short    flags;
    unsigned short    addr;
    void        *platform_data;
    struct dev_archdata    *archdata;
    struct device_node *of_node;
    int        irq;
};

struct i2c_driver {
    const unsigned short *address_list;
    struct list_head clients;

    struct device_driver driver;
    const struct i2c_device_id *id_table;

    /* a ioctl like command that can be used to perform specific functionswith the device */
    int (*command)(struct i2c_client *client, unsigned int cmd, void *arg);

    /* Alert callback, for example for the SMBus alert protocol */
    void (*alert)(struct i2c_client *, unsigned int data);

    /* Device detection callback for automatic device creation */
    int (*detect)(struct i2c_client *, struct i2c_board_info *);

    /* Standard driver model interfaces */
    int (*remove)(struct i2c_client *);
    int (*probe)(struct i2c_client *, const struct i2c_device_id *);
};

/* I2C_BOARD_INFO - macro used to list an i2c device and its address */
#define I2C_BOARD_INFO(dev_type, dev_addr)    .type = dev_type, .addr = (dev_addr)

/* Construct an I2C_CLIENT_END-terminated array of i2c addresses */
#define I2C_ADDRS(addr, addrs...) \
    ((const unsigned short []){ addr, ## addrs, I2C_CLIENT_END })

#define to_i2c_driver(d)       container_of(d, struct i2c_driver, driver)
#define to_i2c_client(d)        container_of(d, struct i2c_client, dev)
#define to_i2c_adapter(d)    container_of(d, struct i2c_adapter, dev)

/* i2c adapter classes (bitmask) */
#define I2C_CLASS_DDC          (1<<3)
#define I2C_CLASS_SPD          (1<<7)
#define I2C_CLASS_HWMON    (1<<0)

/*flags for the client struct: */
#define I2C_CLIENT_TEN       0x10
#define I2C_CLIENT_WAKE    0x80
#define I2C_CLIENT_PEC       0x04
#define I2C_CLIENT_END       0xfffeU



/* SMBus transaction types */
#define I2C_SMBUS_I2C_BLOCK_DATA         8
#define I2C_SMBUS_BLOCK_PROC_CALL      7
#define I2C_SMBUS_I2C_BLOCK_BROKEN    6
#define I2C_SMBUS_BLOCK_DATA                5
#define I2C_SMBUS_PROC_CALL                  4
#define I2C_SMBUS_WORD_DATA                3
#define I2C_SMBUS_BYTE_DATA                  2
#define I2C_SMBUS_BYTE                            1
#define I2C_SMBUS_QUICK                          0

#define I2C_SMBUS_WRITE    0
#define I2C_SMBUS_READ      1

/* To determine what functionality is present */
#define I2C_FUNC_SMBUS_PROC_CALL    0x00800000
#define I2C_FUNC_SMBUS_WRITE_WORD_DATA    0x00400000
#define I2C_FUNC_SMBUS_WORD_DATA    0x00300000
#define I2C_FUNC_SMBUS_READ_WORD_DATA    0x00200000
#define I2C_FUNC_SMBUS_WRITE_BYTE_DATA    0x00100000



#define I2C_FUNC_SMBUS_WRITE_I2C_BLOCK    0x08000000
#define I2C_FUNC_SMBUS_READ_I2C_BLOCK    0x04000000
#define I2C_FUNC_SMBUS_BLOCK_DATA    0x03000000
#define I2C_FUNC_SMBUS_WRITE_BLOCK_DATA 0x02000000
#define I2C_FUNC_SMBUS_READ_BLOCK_DATA    0x01000000


#define I2C_FUNC_SMBUS_I2C_BLOCK    0x0c000000

#define I2C_FUNC_SMBUS_READ_BYTE_DATA    0x00080000
#define I2C_FUNC_SMBUS_BYTE        0x00060000
#define I2C_FUNC_SMBUS_WRITE_BYTE    0x00040000
#define I2C_FUNC_SMBUS_READ_BYTE    0x00020000
#define I2C_FUNC_SMBUS_QUICK        0x00010000
#define I2C_FUNC_SMBUS_BLOCK_PROC_CALL    0x00008000


#define I2C_FUNC_SMBUS_BYTE_DATA    0x00180000

#define I2C_FUNC_SMBUS_PEC        0x00000008
#define I2C_FUNC_PROTOCOL_MANGLING    0x00000004
#define I2C_FUNC_10BIT_ADDR        0x00000002
#define I2C_FUNC_I2C            0x00000001

#define I2C_FUNC_SMBUS_EMUL        (I2C_FUNC_SMBUS_QUICK | \
                     I2C_FUNC_SMBUS_WORD_DATA | \
                     I2C_FUNC_SMBUS_BYTE | \
                     I2C_FUNC_SMBUS_BYTE_DATA | \
                     I2C_FUNC_SMBUS_PROC_CALL | \
                     I2C_FUNC_SMBUS_WRITE_BLOCK_DATA | \
                     I2C_FUNC_SMBUS_I2C_BLOCK | \
                     I2C_FUNC_SMBUS_PEC)

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* _LINUX_I2C_H */
