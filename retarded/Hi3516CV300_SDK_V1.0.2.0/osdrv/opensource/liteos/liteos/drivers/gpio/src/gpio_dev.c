#include "fcntl.h"
#include "linux/kernel.h"
#include "fs/fs.h"

#include "gpio.h"
#include "gpio_dev.h"

struct gpio_descriptor *gpio = NULL;

static int gpio_open(struct file *filep)
{
    return 0;
}

static int gpio_close(struct file *filep)
{
    return 0;
}

static int gpio_ioctl(struct file *filep, int cmd, unsigned long arg)
{
    int ret = 0;
    struct inode * inode = filep ->f_inode;
    struct gpio_descriptor *gd = (struct gpio_descriptor *)(inode->i_private);

	if(!gd) {
		gpio_err("gpio_descriptor is null!\n");
		return -1;
	}
    if (arg == (unsigned long)NULL) {
		gpio_err("arg is null!\n");
		return -1;
    }

    switch(cmd)
    {
        case GPIO_SET_DIR:
			if(gd->ops->setdir)
				ret = gd->ops->setdir(gd,(gpio_groupbit_info*)arg);
			else
				ret = -1;

            break;
        case GPIO_GET_DIR:
			if(gd->ops->getdir)
				ret = gd->ops->getdir(gd,(gpio_groupbit_info*)arg);
			else
				ret = -1;

            break;
        case GPIO_READ_BIT:
			if(gd->ops->readbit)
				ret = gd->ops->readbit(gd,(gpio_groupbit_info*)arg);
			else
				ret = -1;
            break;
        case GPIO_WRITE_BIT:
			if(gd->ops->writebit)
				ret = gd->ops->writebit(gd,(gpio_groupbit_info*)arg);
			else
				ret = -1;
            break;

        default:
            ret = -1;
    }

    return ret;

}

static const struct file_operations_vfs gpio_dev_ops =
{
    gpio_open,  /* open */
    gpio_close, /* close */
    0,  /* read */
    0, /* write */
    0,          /* seek */
    gpio_ioctl  /* ioctl */
#ifndef CONFIG_DISABLE_POLL
        , 0 /* poll */
#endif
};

int gpio_dev_init(void)
{
    int ret;
    gpio_groupbit_info* gpio_info = NULL;

	if (gpio) {
		gpio_err("gpio already init!\n");
		return -1;
	}

    gpio = (struct gpio_descriptor *)malloc(sizeof(struct gpio_descriptor));

    ret = gpio_chip_init(gpio);
    if (ret) {
        gpio_err("gpio_init fail!\n");
        goto free_descptor;
    }
#ifdef LOSCFG_FS_VFS
    gpio_info = (gpio_groupbit_info*)malloc(sizeof(gpio_groupbit_info));
	if (!gpio_info) {
		gpio_err("get gpio descptor fail!\n");
        goto free_descptor;
	}

    gpio->private = gpio_info;
    ret = register_driver("/dev/gpio", &gpio_dev_ops, 0666, gpio);
    if (ret) {
        gpio_chip_deinit(gpio);
        gpio_err("register_driver /dev/gpio failed!\n");
        goto free_gpio_info;
    }
#endif

    return 0;
free_gpio_info:
    free(gpio_info);
free_descptor:
    free(gpio);
    gpio = NULL;
out:
    return ret;
}

int gpio_deinit(void)
{
	int ret=0;

	if(gpio) {
#ifdef LOSCFG_FS_VFS
		ret = unregister_driver("/dev/gpio");
		if(!ret) {
            gpio_err("unregister_driver fail, ret= %d\n", ret);
		}
#endif
		gpio_chip_deinit(gpio);
        free(gpio);
        gpio = NULL;
	} else {
		ret = -1;
		gpio_err("gpio already deinit!\n");
	}

	return ret;
}
