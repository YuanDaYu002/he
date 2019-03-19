#include "linux/kernel.h"
#include "linux/i2c.h"
#include "linux/delay.h"
#include "linux/errno.h"
#include "linux/io.h"
#include "i2c-hibvt.h"
#include "i2c.h"
#include "hisoc/i2c.h"

static inline void hibvt_i2c_disable(struct i2c_driver_data *i2c)
{
    unsigned int val;

    val = readl(i2c->regbase + HIBVT_I2C_GLB);
    val &= ~GLB_EN_MASK;
    writel(val, i2c->regbase + HIBVT_I2C_GLB);
}

static inline void hibvt_i2c_disable_irq(struct i2c_driver_data *i2c,
		unsigned int flag)
{
	unsigned int val;

	val = readl(i2c->regbase + HIBVT_I2C_INTR_EN);
	val &= ~flag;
	writel(val, i2c->regbase + HIBVT_I2C_INTR_EN);
}

static inline unsigned int hibvt_i2c_clr_irq(struct i2c_driver_data *i2c)
{
	unsigned int val;

	val = readl(i2c->regbase + HIBVT_I2C_INTR_STAT);
	writel(INTR_ALL_MASK, i2c->regbase + HIBVT_I2C_INTR_RAW);

	return val;
}

static void hibvt_i2c_set_freq(struct i2c_driver_data *i2c)
{
	unsigned int max_freq = 0, freq = 0;
	unsigned int clk_rate;
	unsigned int val;

	freq = i2c->freq;
	clk_rate = i2c->clk;
	max_freq = clk_rate >> 1;

	if (freq > max_freq) {
		i2c->freq = max_freq;
		freq = i2c->freq;
	}

	if (freq <= 100000) {
		val = clk_rate / (freq * 2);
		writel(val, i2c->regbase + HIBVT_I2C_SCL_H);
		writel(val, i2c->regbase + HIBVT_I2C_SCL_L);
	} else {
		val = (clk_rate * 36) / (freq * 100);
		writel(val, i2c->regbase + HIBVT_I2C_SCL_H);
		val = (clk_rate * 64) / (freq * 100);
		writel(val, i2c->regbase + HIBVT_I2C_SCL_L);
	}
	val = readl(i2c->regbase + HIBVT_I2C_GLB);
	val &= ~GLB_SDA_HOLD_MASK;
	val |= ((0xa << GLB_SDA_HOLD_SHIFT) & GLB_SDA_HOLD_MASK);
	writel(val, i2c->regbase + HIBVT_I2C_GLB);
}

/*
 * set i2c controller TX and RX FIFO water
 */
static inline void hibvt_i2c_set_water(struct i2c_driver_data *i2c)
{
	writel(I2C_TXF_WATER, i2c->regbase + HIBVT_I2C_TX_WATER);
	writel(I2C_RXF_WATER, i2c->regbase + HIBVT_I2C_RX_WATER);
}

/*
 *  initialise the controller, set i2c bus interface freq
 */
static void hibvt_i2c_hw_init(struct i2c_driver_data *i2c)
{
    hibvt_i2c_disable(i2c);
    hibvt_i2c_disable_irq(i2c, INTR_ALL_MASK);
    hibvt_i2c_set_freq(i2c);
    hibvt_i2c_set_water(i2c);
}

static inline void hibvt_i2c_cmdreg_set(struct i2c_driver_data *i2c,
		unsigned int cmd, unsigned int *offset)
{
	i2c_msg("hii2c reg: offset=0x%x, cmd=0x%x...\n",
			*offset * 4, cmd);
	writel(cmd, i2c->regbase + HIBVT_I2C_CMD_BASE + *offset * 4);
	(*offset)++;
}

static void hibvt_i2c_cfg_cmd(struct i2c_driver_data *i2c)
{
	struct i2c_msg *msg = i2c->msgs;
	int offset = 0;

	if (i2c->msg_idx == 0)
		hibvt_i2c_cmdreg_set(i2c, CMD_TX_S, &offset);
	else
		hibvt_i2c_cmdreg_set(i2c, CMD_TX_RS, &offset);

	if (msg->flags & I2C_M_TEN) {
		if (i2c->msg_idx == 0) {
			hibvt_i2c_cmdreg_set(i2c, CMD_TX_D1_2, &offset);
			hibvt_i2c_cmdreg_set(i2c, CMD_TX_D1_1, &offset);
		} else {
			hibvt_i2c_cmdreg_set(i2c, CMD_TX_D1_2, &offset);
		}
	} else {
		hibvt_i2c_cmdreg_set(i2c, CMD_TX_D1_1, &offset);
	}

	if (msg->flags & I2C_M_IGNORE_NAK)
		hibvt_i2c_cmdreg_set(i2c, CMD_IGN_ACK, &offset);
    else
		hibvt_i2c_cmdreg_set(i2c, CMD_RX_ACK, &offset);

	if (msg->flags & I2C_M_RD) {
		if (msg->len >= 2) {
			writel(offset, i2c->regbase + HIBVT_I2C_DST1);
			writel(msg->len - 2, i2c->regbase + HIBVT_I2C_LOOP1);
			hibvt_i2c_cmdreg_set(i2c, CMD_RX_FIFO, &offset);
			hibvt_i2c_cmdreg_set(i2c, CMD_TX_ACK, &offset);
			hibvt_i2c_cmdreg_set(i2c, CMD_JMP1, &offset);
		}
		hibvt_i2c_cmdreg_set(i2c, CMD_RX_FIFO, &offset);
		hibvt_i2c_cmdreg_set(i2c, CMD_TX_NACK, &offset);
	} else {
		writel(offset, i2c->regbase + HIBVT_I2C_DST1);
		writel(msg->len - 1, i2c->regbase + HIBVT_I2C_LOOP1);
		hibvt_i2c_cmdreg_set(i2c, CMD_UP_TXF, &offset);
		hibvt_i2c_cmdreg_set(i2c, CMD_TX_FIFO, &offset);

		if (msg->flags & I2C_M_IGNORE_NAK)
			hibvt_i2c_cmdreg_set(i2c, CMD_IGN_ACK, &offset);
		else
			hibvt_i2c_cmdreg_set(i2c, CMD_RX_ACK, &offset);

		hibvt_i2c_cmdreg_set(i2c, CMD_JMP1, &offset);
	}

	if ((i2c->msg_idx == (i2c->msg_num - 1))
            || (msg->flags & I2C_M_STOP)) {
		i2c_msg("run to %s %d...TX STOP\n",
				__func__, __LINE__);
		hibvt_i2c_cmdreg_set(i2c, CMD_TX_P, &offset);
	}

	hibvt_i2c_cmdreg_set(i2c, CMD_EXIT, &offset);

}

static inline void hibvt_i2c_enable(struct i2c_driver_data *i2c)
{
	unsigned int val;

	val = readl(i2c->regbase + HIBVT_I2C_GLB);
	val |= GLB_EN_MASK;
	writel(val, i2c->regbase + HIBVT_I2C_GLB);
}

/*
 * config i2c slave addr
 */
static inline void hibvt_i2c_set_addr(struct i2c_driver_data *i2c)
{
	struct i2c_msg *msg = i2c->msgs;
	unsigned int addr;

	if (msg->flags & I2C_M_TEN) {
		/* First byte is 11110XX0 where XX is upper 2 bits */
		addr = ((msg->addr & 0x300) << 1) | 0xf000;
		if (msg->flags & I2C_M_RD)
			addr |= 1 << 8;

		/* Second byte is the remaining 8 bits */
		addr |= msg->addr & 0xff;
	} else {
		addr = (msg->addr & 0x7f) << 1;
		if (msg->flags & I2C_M_RD)
			addr |= 1;
	}

	writel(addr, i2c->regbase + HIBVT_I2C_DATA1);
}

/*
 * Start command sequence
 */
static inline void hibvt_i2c_start_cmd(struct i2c_driver_data *i2c)
{
	unsigned int val;

	val = readl(i2c->regbase + HIBVT_I2C_CTRL1);
	val |= CTRL1_CMD_START_MASK;
	writel(val, i2c->regbase + HIBVT_I2C_CTRL1);
}


static int hibvt_i2c_wait_rx_noempty(struct i2c_driver_data *i2c)
{
	unsigned int time_cnt = 0;
	unsigned int val;

	do {
		val = readl(i2c->regbase + HIBVT_I2C_STAT);
		if (val & STAT_RXF_NOE_MASK)
			return 0;

		udelay(50);
	} while (time_cnt++ < I2C_TIMEOUT_COUNT);

	i2c_msg("wait rx no empty timeout, RIS: 0x%x, SR: 0x%x\n",
			readl(i2c->regbase + HIBVT_I2C_INTR_RAW), val);
	return -EIO;
}

static int hibvt_i2c_wait_tx_nofull(struct i2c_driver_data *i2c)
{
	unsigned int time_cnt = 0;
	unsigned int val;

	do {
		val = readl(i2c->regbase + HIBVT_I2C_STAT);
		if (val & STAT_TXF_NOF_MASK)
			return 0;

		udelay(50);
	} while (time_cnt++ < I2C_TIMEOUT_COUNT);

	i2c_msg("wait rx no empty timeout, RIS: 0x%x, SR: 0x%x\n",
			readl(i2c->regbase + HIBVT_I2C_INTR_RAW), val);
	return -EIO;
}

static int hibvt_i2c_wait_idle(struct i2c_driver_data *i2c)
{
	unsigned int time_cnt = 0;
	unsigned int val;

	do {
		val = readl(i2c->regbase + HIBVT_I2C_INTR_RAW);
		if (val & (INTR_ABORT_MASK)) {
			i2c_err("wait idle abort!, RIS: 0x%x\n",val);
			return -EIO;
		}

		if (val & INTR_CMD_DONE_MASK)
			return 0;

		udelay(50);
	} while (time_cnt++ < I2C_WAIT_TIMEOUT);

	i2c_msg("wait idle timeout, RIS: 0x%x, SR: 0x%x\n",
			val, readl(i2c->regbase + HIBVT_I2C_STAT));

	return -EIO;
}

static int hibvt_i2c_polling_xfer_one_msg(struct i2c_driver_data *i2c)
{
    int status;
    unsigned int val;
    struct i2c_msg *msg = i2c->msgs;
    unsigned int msg_buf_ptr = 0;

    i2c_msg("[%s,%d]msg->flags=0x%x, len=0x%x\n",
            __func__, __LINE__, msg->flags, msg->len);

    hibvt_i2c_enable(i2c);
    hibvt_i2c_clr_irq(i2c);
    hibvt_i2c_set_addr(i2c);
    hibvt_i2c_cfg_cmd(i2c);
    hibvt_i2c_start_cmd(i2c);

	if (msg->flags & I2C_M_RD) {
		while (msg_buf_ptr < msg->len) {
			status = hibvt_i2c_wait_rx_noempty(i2c);
			if (status)
				goto end;

			val = readl(i2c->regbase + HIBVT_I2C_RXF);
			msg->buf[msg_buf_ptr] = val;
			msg_buf_ptr++;

		}
	} else {
		while (msg_buf_ptr < msg->len) {
            
			status = hibvt_i2c_wait_tx_nofull(i2c);
			if (status)
				goto end;

			val = msg->buf[msg_buf_ptr];
			writel(val, i2c->regbase + HIBVT_I2C_TXF);
			msg_buf_ptr++;
		}
	}

	status = hibvt_i2c_wait_idle(i2c);
end:
	hibvt_i2c_disable(i2c);

	return status;
}

static inline void hibvt_i2c_cfg_irq(struct i2c_driver_data *i2c,
		unsigned int flag)
{
	writel(flag, i2c->regbase + HIBVT_I2C_INTR_EN);
}

static int hibvt_i2c_interrupt_xfer_one_msg(struct i2c_driver_data *i2c)
{
	int status;
	struct i2c_msg *msg = i2c->msgs;
	unsigned int event_status;
	unsigned long flags;
    unsigned int msg_buf_ptr = 0;
    struct hibvt_platform_i2c *hpi = (struct hibvt_platform_i2c *) i2c->private;

	i2c_msg("[%s,%d]msg->flags=0x%x, len=0x%x\n",
			__func__, __LINE__, msg->flags, msg->len);

	msg_buf_ptr = 0;
	hpi->status = -EIO;

    /* lock irq here */
    flags = LOS_IntLock();
	hibvt_i2c_enable(i2c);
	hibvt_i2c_clr_irq(i2c);
	if (msg->flags & I2C_M_RD)
		hibvt_i2c_cfg_irq(i2c, INTR_USE_MASK & ~INTR_TX_MASK);
	else
		hibvt_i2c_cfg_irq(i2c, INTR_USE_MASK & ~INTR_RX_MASK);

	hibvt_i2c_set_addr(i2c);
	hibvt_i2c_cfg_cmd(i2c);
	hibvt_i2c_start_cmd(i2c);
    LOS_IntRestore(flags);

    event_status = LOS_EventRead(&(hpi->msg_event), I2C_WAIT_RESPOND,
            LOS_WAITMODE_OR+LOS_WAITMODE_CLR, I2C_WAIT_TIMEOUT);

	if (event_status == LOS_ERRNO_EVENT_READ_TIMEOUT) {
		hibvt_i2c_disable_irq(i2c, INTR_ALL_MASK);
		status = -EIO;
		i2c_err("%s timeout\n",
			 msg->flags & I2C_M_RD ? "rx" : "tx");
	} else {
		status = hpi->status;
	}

	hibvt_i2c_disable(i2c);

	return status;
}

int hibvt_i2c_xfer(struct i2c_adapter *adap, struct i2c_msg *msgs, int num)
{
    struct i2c_driver_data *i2c = (struct i2c_driver_data *)adap->dev.driver_data;
    int status;
    unsigned long irq_save;

    if(!msgs) {
        i2c_err("invalid argument\n");
        return -EIO;
    }

    irq_save = LOS_IntLock();

    i2c->msgs = msgs;
    i2c->msg_num = num;
    i2c->msg_idx = 0;

#ifdef HIBVT_I2C_INTERRUPT_MODE
    while (i2c->msg_idx < i2c->msg_num) {
        status = hibvt_i2c_interrupt_xfer_one_msg(i2c);
        if (status)
            break;

        i2c->msgs++;
        i2c->msg_idx++;
    }
#else
    while (i2c->msg_idx < i2c->msg_num) {
        status = hibvt_i2c_polling_xfer_one_msg(i2c);
        if (status)
            break;

        i2c->msgs++;
        i2c->msg_idx++;
    }
#endif
	if (!status || i2c->msg_idx > 0)
		status = i2c->msg_idx;

    LOS_IntRestore(irq_save);

	return status;
}


int hi_i2c_transfer(struct i2c_adapter *adapter, struct i2c_msg *msgs, int count)
{
    int ret;

    ret = hibvt_i2c_xfer(adapter, msgs, count);

    return ret;
}

int hi_i2c_master_send(struct i2c_client *client, const char *buf, int count)
{
    int ret;
    struct i2c_msg msg;
    struct i2c_adapter *adapter = client->adapter;

    msg.addr = client->addr;
    msg.flags = client->flags & (~I2C_M_RD);
    msg.len = count;
    msg.buf = (char *)buf;

    ret = hi_i2c_transfer(adapter, &msg, 1);

    return (ret == 1) ? count : ret;
}

int hi_i2c_master_recv(const struct i2c_client *client, char *buf, int count)
{
    int ret;
    struct i2c_msg msg;
    struct i2c_adapter *adapter = client->adapter;

    msg.addr = client->addr;
    msg.flags = client->flags;

    msg.flags |= I2C_M_RD;
    msg.len = count;
    msg.buf = buf;

    ret = hi_i2c_transfer(adapter, &msg, 1);

    /*
     * If everything went ok (i.e. 1 msg received), return #bytes received,
     * else error code.
     */
    return (ret == 1) ? count : ret;
}

static irqreturn_t hibvt_i2c_isr(int irq, void *dev_i2c)
{
    struct i2c_driver_data *i2c = (struct i2c_driver_data *)dev_i2c;
    struct hibvt_platform_i2c *hpi = (struct hibvt_platform_i2c *) i2c->private;
    unsigned int irq_status;
    struct i2c_msg *msg = i2c->msgs;
    unsigned int msg_buf_ptr = 0;

    /* FIXME: lock i2c here */

    irq_status = hibvt_i2c_clr_irq(i2c);
    i2c_msg("%s RIS:  0x%x\n", __func__, irq_status);

    if (!irq_status) {
        i2c_msg("no irq\n");
        goto end;
    }

    if (irq_status & INTR_ABORT_MASK) {
       i2c_err("irq abort,RIS: 0x%x\n", irq_status);
       hpi->status = -EIO;
       hibvt_i2c_disable_irq(i2c, INTR_ALL_MASK);

       LOS_EventWrite(&(hpi->msg_event), I2C_WAIT_RESPOND);
       goto end;
    }

    if (msg->flags & I2C_M_RD) {
        while((readl(i2c->regbase + HIBVT_I2C_STAT) & STAT_RXF_NOE_MASK)
                && (msg_buf_ptr < msg->len)) {
            msg->buf[msg_buf_ptr] =
                readl(i2c->regbase + HIBVT_I2C_RXF);
            msg_buf_ptr++;
        }
    } else {
        while ((readl(i2c->regbase + HIBVT_I2C_STAT) & STAT_TXF_NOF_MASK)
                && (msg_buf_ptr < msg->len)) {
            writel(msg->buf[msg_buf_ptr],
                    i2c->regbase + HIBVT_I2C_TXF);
            msg_buf_ptr++;
        }
    }

	if (msg_buf_ptr >= msg->len)
		hibvt_i2c_disable_irq(i2c, INTR_TX_MASK | INTR_RX_MASK);

	if (irq_status & INTR_CMD_DONE_MASK) {
		i2c_msg("cmd done\n");
		hpi->status =  0;
		hibvt_i2c_disable_irq(i2c, INTR_ALL_MASK);

        LOS_EventWrite(&(hpi->msg_event), I2C_WAIT_RESPOND);
	}
end:
    /* unlock here*/
    return IRQ_HANDLED;
}

extern struct i2c_adapter adapter_array[];

static const struct i2c_algorithm hibvt_i2c_algo = {
    .master_xfer        = &hibvt_i2c_xfer,
};

int check_host_enable(unsigned int host_num)
{
    if (host_num >= I2C_NUM)
        return -1;
    return i2c_host_cfg[host_num];
}

int i2c_host_init(unsigned int max_adap_num)
{
    int index = 0;
    unsigned int count = 0;
    int ret = 0;
    struct i2c_adapter *adapter = NULL;
    struct i2c_driver_data* i2c = NULL;
    struct hibvt_platform_i2c * hpi = NULL;

    count = (I2C_NUM < max_adap_num)?(I2C_NUM):max_adap_num;

    for (; index < count; index++) {
        ret = check_host_enable(index);
        if(ret < 0)
            continue;
        i2c = &hibvt_i2c_data[index];
        hpi = &hibvt_i2c_platform_data[index];
        i2c->clk = get_host_clock(index);
        i2c->private = (void *)(hpi);
        adapter = &adapter_array[index];
        i2c->adapter = adapter;
        hibvt_i2c_hw_init(i2c);
#ifdef HIBVT_I2C_INTERRUPT_MODE
        LOS_EventInit(&(hpi->msg_event));
        ret = request_irq(hpi->irq, (irq_handler_t)hibvt_i2c_isr, 0, "HIBVT_I2C", i2c);
        if(ret) {
            i2c_err("regist irq fail!\n");
            return -1;
        }
#endif
        adapter->nr = index;
        adapter->algo = &hibvt_i2c_algo;
        adapter->dev.driver_data = (void*)i2c;
    }
    return index;
}
