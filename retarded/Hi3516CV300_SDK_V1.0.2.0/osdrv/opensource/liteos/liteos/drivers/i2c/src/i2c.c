#include "linux/kernel.h"
#include "linux/errno.h"
#include "linux/i2c.h"
#include "i2c.h"
#include "los_mux.h"

extern int check_host_enable(unsigned int host_num);
extern int i2c_host_init(unsigned int max_adap_num);

struct i2c_adapter adapter_array[LOSCFG_I2C_ADAPTER_COUNT];

int i2c_transfer(struct i2c_adapter *adapter, struct i2c_msg *msgs, int count)
{
    int ret;

    if (LOS_MuxPend(adapter->adapter_lock, LOSCFG_BASE_CORE_TICK_PER_SECOND * 10)  != LOS_OK) {
        PRINT_ERR("Lock adapter fail!\n");
        ret = -ETIMEDOUT;
        return ret;
    }

    ret = adapter->algo->master_xfer(adapter, msgs, count);
    (void)LOS_MuxPost(adapter->adapter_lock);

    return ret;
}

int i2c_master_send(struct i2c_client *client, const char *buf, int count)
{
    int ret;
    struct i2c_msg msg;
    struct i2c_adapter *adapter = client->adapter;

    msg.addr = client->addr;
    msg.flags = client->flags & (~I2C_M_RD);
    msg.len = count;
    msg.buf = (char *)buf;

    ret = i2c_transfer(adapter, &msg, 1);

    return (ret == 1) ? count : ret;
}

int i2c_master_recv(const struct i2c_client *client, char *buf, int count)
{
    int ret;
    struct i2c_msg msg;
    struct i2c_adapter *adapter = client->adapter;

    msg.addr = client->addr;
    msg.flags = client->flags;

    msg.flags |= I2C_M_RD;
    msg.len = count;
    msg.buf = buf;

    ret = i2c_transfer(adapter, &msg, 1);

    /*
     * If everything went ok (i.e. 1 msg received), return #bytes received,
     * else error code.
     */
    return (ret == 1) ? count : ret;
}

struct i2c_adapter * get_adapter_by_index(unsigned int index)
{
    int ret = -1;
    struct i2c_adapter *adapter = NULL;

    if (index >= LOSCFG_I2C_ADAPTER_COUNT) {
        PRINT_ERR("argument out of range!\n");
        return NULL;
    }
    ret = check_host_enable(index);
    if (ret != index) {
        PRINT_WARN("driver:host %d is disable!\n",index);
        return NULL;
    }
    adapter = &adapter_array[index];
    
    return adapter;
}

int i2c_adapter_init(void)
{
    int ret = -1, i = 0;
    static int reinit = 0;

    if (reinit) {
        PRINT_ERR("adapter initialized!\n");
        return -1;
    }
    else
        reinit++;

    memset(adapter_array, 0 , (LOSCFG_I2C_ADAPTER_COUNT * sizeof(struct i2c_adapter)));
    for(; i < LOSCFG_I2C_ADAPTER_COUNT; i++)
        (void)LOS_MuxCreate(&(adapter_array[i].adapter_lock));

    ret = i2c_host_init(LOSCFG_I2C_ADAPTER_COUNT);
    if(ret < 0) {
        PRINT_ERR("adapter init fail!\n");
        return -1;
    }
    return ret;
}

/* client_attach: attach cliet to adapter
 * @client: i2c client
 * @adapter_index: i2c host number
 * return: 
 * 0 : success
 * -1: fail
 */
int client_attach(struct i2c_client * client, int adapter_index)
{
    struct i2c_adapter *adapter = NULL;
    int ret = -1;
    adapter = get_adapter_by_index(adapter_index);
    if (adapter == NULL) {
        return -1;
    }
    client->adapter = adapter;
    return 0;
}

int client_deinit(struct i2c_client * client)
{
    client->adapter = NULL;
    client->driver = NULL;
    return 0;
}
