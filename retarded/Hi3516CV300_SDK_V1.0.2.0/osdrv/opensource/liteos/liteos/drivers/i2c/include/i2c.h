#ifndef I2C_DRIVER_H
#define I2C_DRIVER_H

#include "linux/rwsem.h"
#include "linux/i2c.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#define LOSCFG_DEBUG_VERSION
#ifdef LOSCFG_DEBUG_VERSION
    #define i2c_err(x...) \
        do { \
                    dprintf("%s->%d: ", __func__, __LINE__); \
                    dprintf(x); \
                    dprintf("\n"); \
            } while (0)
    #undef I2C_DEBUG
#else
    #define i2c_err(x...) do { } while (0)
    #undef I2C_DEBUG
#endif

#ifdef I2C_DEBUG
    #define i2c_msg(x...) \
        do { \
                    dprintf("%s (line:%d) ", __func__, __LINE__); \
                    dprintf(x); \
            } while (0)
#else
    #define i2c_msg(x...) do { } while (0)
#endif

#ifdef LOSCFG_FS_VFS
/* /dev/i2c-x ioctl commands */
#define I2C_RETRIES     0x0701
#define I2C_TIMEOUT     0x0702
#define I2C_SLAVE       0x0703
#define I2C_SLAVE_FORCE 0x0706
#define I2C_TENBIT      0x0704
#define I2C_FUNCS       0x0705
#define I2C_RDWR        0x0707
#define I2C_PEC         0x0708
#define I2C_SMBUS       0x0720
#define I2C_16BIT_REG   0x0709  /* 16BIT REG WIDTH */
#define I2C_16BIT_DATA  0x070a  /* 16BIT DATA WIDTH */
#endif /* LOSCFG_FS_VFS */

struct i2c_rdwr_ioctl_data {
    struct i2c_msg *msgs;
    unsigned int nmsgs;
};

#define  I2C_RDWR_IOCTL_MAX_MSGS    42

struct i2c_driver_data {
    struct i2c_adapter* adapter;
    volatile unsigned char  *regbase;
    unsigned int      clk;
    unsigned int        freq;
    unsigned int irq;
    struct i2c_msg *msgs;
    unsigned int        msg_num;
    unsigned int        msg_idx;
    UINT32 lock;
    void    *private;    
};

/* open for User */
extern int i2c_dev_init(void);
extern int i2c_adapter_init(void);
extern int client_attach(struct i2c_client * client, int adapter_index);
extern int client_deinit(struct i2c_client * client);

extern int i2c_transfer(struct i2c_adapter *adapter, struct i2c_msg *msgs, int count);
extern int i2c_master_send(struct i2c_client *client, const char *buf, int count);
extern int i2c_master_recv(const struct i2c_client *client, char *buf, int count);

extern int hi_i2c_transfer(struct i2c_adapter *adapter, struct i2c_msg *msgs, int count);
extern int hi_i2c_master_send(struct i2c_client *client, const char *buf, int count);
extern int hi_i2c_master_recv(const struct i2c_client *client, char *buf, int count);
#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif
