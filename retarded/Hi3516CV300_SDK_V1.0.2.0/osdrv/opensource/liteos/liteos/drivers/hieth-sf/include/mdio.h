#ifndef __HIETH_MDIO_H
#define __HIETH_MDIO_H

#include "hisi_mac.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */


struct hieth_mdio_local {
    unsigned long iobase;
    unsigned long iobase_phys;

    union {
        unsigned long w32_bits_flags;
        struct {
            /* mdio_bus freq-div, 1 for 1/100, 0 for 1/50 */
            int mdio_frqdiv;
        };
    };

    spinlock_t lock;
    unsigned long lockflags;
};

#define MDIO_RWCTRL        0x1100
#define MDIO_RO_DATA        0x1104
#define U_MDIO_PHYADDR        0x0108
#define D_MDIO_PHYADDR        0x2108
#define U_MDIO_RO_STAT        0x010C
#define D_MDIO_RO_STAT        0x210C
#define U_MDIO_ANEG_CTRL    0x0110
#define D_MDIO_ANEG_CTRL    0x2110
#define U_MDIO_IRQENA        0x0114
#define D_MDIO_IRQENA        0x2114

#define MDIO_MK_RWCTL(cpu_data_in, finish, rw, phy_exaddr, frq_div, phy_regnum)\
        (((cpu_data_in)<<16) | \
          (((finish)&0x01)<<15) | \
          (((rw)&0x01)<<13) | \
          (((phy_exaddr)&0x1F)<<8) | \
          (((frq_div)&0x7)<<5) | \
          ((phy_regnum)&0x1F))

/* hardware set bit'15 of MDIO_REG(0) if mdio ready */
#define test_mdio_ready(ld) (hieth_readl(ld, MDIO_RWCTRL)&(1<<15))

#define mdio_start_phyread(ld, phy_addr, regnum) \
    hieth_writel(ld,\
        MDIO_MK_RWCTL(0, 0, 0, phy_addr, (ld)->mdio_frqdiv, regnum),\
        MDIO_RWCTRL)

#define mdio_get_phyread_val(ld) (hieth_readl(ld, MDIO_RO_DATA) & 0xFFFF)

#define mdio_phywrite(ld, phy_addr, regnum, val) \
    hieth_writel(ld,\
        MDIO_MK_RWCTL(val, 0, 1, phy_addr, (ld)->mdio_frqdiv, regnum),\
        MDIO_RWCTRL)

/* write mdio registers reset value */
#define mdio_reg_reset(ld) do { \
        hieth_writel(ld, 0x00008000, MDIO_RWCTRL); \
        hieth_writel(ld, 0x00000001, U_MDIO_PHYADDR); \
        hieth_writel(ld, 0x00000001, D_MDIO_PHYADDR); \
        hieth_writel(ld, 0x04631EA9, U_MDIO_ANEG_CTRL); \
        hieth_writel(ld, 0x04631EA9, D_MDIO_ANEG_CTRL); \
        hieth_writel(ld, 0x00000000, U_MDIO_IRQENA); \
        hieth_writel(ld, 0x00000000, D_MDIO_IRQENA); \
    } while (0)

int hieth_mdio_read(struct hieth_netdev_local *ld,
                        int phy_addr, int regnum);
int hieth_mdio_write(
                    struct hieth_netdev_local *ld,
                    int phy_addr, int regnum, int val);
int hieth_mdio_reset(struct hieth_netdev_local *ld);
int hieth_mdio_init(struct hieth_netdev_local *ld);
void hieth_mdio_exit(struct hieth_netdev_local *ld);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif

/* vim: set ts=8 sw=8 tw=78: */
