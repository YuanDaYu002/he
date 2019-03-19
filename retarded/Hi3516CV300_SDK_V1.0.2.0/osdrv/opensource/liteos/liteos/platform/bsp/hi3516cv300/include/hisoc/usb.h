#ifndef __HISOC_USB_H__
#define    __HISOC_USB_H__

#include "los_base.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#define CONFIG_HIUSB_EHCI_IOBASE    0x10120000
#define CONFIG_HIUSB_EHCI_IOSIZE    0x00010000

#define CONFIG_HIUSBUDC_REG_BASE_ADDRESS         0x10130000
#define CONFIG_HIUSBUDC_REG_BASE_ADDRESS_LEN    0x40000

#define USB_CACHE_ALIGN_SIZE 32

#define SKB_DATA_ALIGN(X)  ALIGN(X, USB_CACHE_ALIGN_SIZE)

#define UVC_USE_CTRL_EP 1

extern void hiusb_start_hcd(void);
extern void hiusb_stop_hcd(void);
extern void hiusb_reset_hcd(void);

extern void hiusb_host2device(void);
extern void hiusb_device2host(void);

extern int hiusb_is_device_mode(void);

extern void usb_otg_run(void);

extern void usb_otg_sw_set_host_state(void);
extern void usb_otg_sw_set_device_state(void);

extern void usb_otg_sw_clear_host_state(void);
extern void usb_otg_sw_clear_device_state(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif

