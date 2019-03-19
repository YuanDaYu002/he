deps_config := \
	../../drivers/pm/Kconfig \
	../../drivers/video/Kconfig \
	../../drivers/usb/Kconfig \
	../../drivers/spi/Kconfig \
	../../drivers/rtc/Kconfig \
	../../drivers/random/Kconfig \
	../../drivers/mtd/spi_nor/Kconfig \
	../../drivers/mtd/nand/Kconfig \
	../../drivers/mmc/Kconfig \
	../../drivers/mem/Kconfig \
	../../drivers/i2c/Kconfig \
	../../drivers/higmac/Kconfig \
	../../drivers/hieth-sf/Kconfig \
	../../drivers/hidmac/Kconfig \
	../../drivers/gpio/Kconfig \
	../../drivers/uart/Kconfig \
	../../tools/pcap/Kconfig \
	../../tools/iperf-2.0.5/Kconfig \
	../../net/telnet/Kconfig \
	../../shell/Kconfig \
	../../net/Kconfig \
	../../fs/jffs2/Kconfig \
	../../fs/proc/Kconfig \
	../../fs/nfs/Kconfig \
	../../fs/yaffs2/Kconfig \
	../../fs/ramfs/Kconfig \
	../../fs/fat/Kconfig \
	../../fs/vfs/Kconfig \
	../../compat/linux/Kconfig \
	../../compat/posix/Kconfig \
	../../lib/Kconfig \
	../../kernel/Kconfig \
	extra/Configs/Config.in

.config include/bits/uClibc_config.h: $(deps_config)

$(deps_config):
