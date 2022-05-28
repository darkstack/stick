#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
//#include "libopencm3/stm32/syscfg.h"
#include "libopencm3/stm32/f4/rcc.h"
#include "libopencm3/stm32/rcc.h"
#include "libopencm3/usb/usbd.h"
#include "libopencm3/usb/hid.h"
#include "libopencm3/usb/usbstd.h"
#include "libopencm3/stm32/gpio.h"
#include "libopencm3/usb/dwc/otg_fs.h"
static const char *usb_string[] = {
	"Darkstack",
	"Arcade Stick",
	"LaDOSE"
};

const struct usb_device_descriptor dev_descr = {
	.bLength = USB_DT_DEVICE_SIZE,
	.bDescriptorType = USB_DT_DEVICE,
	.bcdUSB = 0x0200,
	.bDeviceClass = USB_HID_SUBCLASS_NO,
	.bDeviceSubClass = 0,
	.bDeviceProtocol = 0,
	.bMaxPacketSize0 = 64,
	.idVendor = 0x6666,
	.idProduct = 0x5710,
	.bcdDevice = 0x0100,
	.iManufacturer = 1,
	.iProduct = 2,
	.iSerialNumber = 3,
	.bNumConfigurations = 1,
};
//Dumped from Qanba PS3 stick 
static const uint8_t hid_report_descriptor[] = {
0x05,0x01,0x09,0x05,0xA1,0x01,0x15,0x00,0x26,0xFF,0x00,0x35,0x00,0x46,0xFF,0x00, 
0x09,0x30,0x09,0x31,0x75,0x08,0x95,0x02,0x81,0x02,0x95,0x03,0x81,0x03,0x15,0x00,
0x25,0x01,0x35,0x00,0x45,0x01,0x75,0x01,0x95,0x0B,0x05,0x09,0x19,0x01,0x29,0x0B,
0x81,0x02,0x95,0x01,0x81,0x03,0x95,0x04,0x81,0x03,0x05,0x01,0x25,0x07,0x46,0x3B,
0x01,0x75,0x04,0x95,0x01,0x65,0x14,0x09,0x39,0x81,0x42,0x65,0x00,0x95,0x01,0x81,
0x03,0x05,0x08,0x09,0x43,0x15,0x00,0x26,0xFF,0x00,0x35,0x00,0x46,0xFF,0x00,0x75,
0x08,0x95,0x01,0x91,0x82,0x09,0x44,0x91,0x82,0x09,0x45,0x91,0x82,0x09,0x46,0x91,
0x82,0xC0
};

static const uint8_t hid_report_descriptor2[] = {
	0x05, 0x01, /* USAGE_PAGE (Generic Desktop)         */
	0x09, 0x05, /* USAGE (Joystick)                        */
	0xa1, 0x01, /* COLLECTION (Application)             */
	0x09, 0x01, /*   USAGE (Pointer)                    */
	0xa1, 0x00, /*   COLLECTION (Physical)              */
	0x05, 0x09, /*     USAGE_PAGE (Button)              */
	0x19, 0x01, /*     USAGE_MINIMUM (Button 1)         */
	0x29, 0x03, /*     USAGE_MAXIMUM (Button 3)         */
	0x15, 0x00, /*     LOGICAL_MINIMUM (0)              */
	0x25, 0x01, /*     LOGICAL_MAXIMUM (1)              */
	0x95, 0x03, /*     REPORT_COUNT (3)                 */
	0x75, 0x01, /*     REPORT_SIZE (1)                  */
	0x81, 0x02, /*     INPUT (Data,Var,Abs)             */
	0x95, 0x01, /*     REPORT_COUNT (1)                 */
	0x75, 0x05, /*     REPORT_SIZE (5)                  */
	0x81, 0x01, /*     INPUT (Cnst,Ary,Abs)             */
	0x05, 0x01, /*     USAGE_PAGE (Generic Desktop)     */
	0x09, 0x30, /*     USAGE (X)                        */
	0x09, 0x31, /*     USAGE (Y)                        */
	0x09, 0x38, /*     USAGE (Wheel)                    */
	0x15, 0x81, /*     LOGICAL_MINIMUM (-127)           */
	0x25, 0x7f, /*     LOGICAL_MAXIMUM (127)            */
	0x75, 0x08, /*     REPORT_SIZE (8)                  */
	0x95, 0x03, /*     REPORT_COUNT (3)                 */
	0x81, 0x06, /*     INPUT (Data,Var,Rel)             */
	0xc0,       /*   END_COLLECTION                     */
	0x09, 0x3c, /*   USAGE (Motion Wakeup)              */
	0x05, 0xff, /*   USAGE_PAGE (Vendor Defined Page 1) */
	0x09, 0x01, /*   USAGE (Vendor Usage 1)             */
	0x15, 0x00, /*   LOGICAL_MINIMUM (0)                */
	0x25, 0x01, /*   LOGICAL_MAXIMUM (1)                */
	0x75, 0x01, /*   REPORT_SIZE (1)                    */
	0x95, 0x02, /*   REPORT_COUNT (2)                   */
	0xb1, 0x22, /*   FEATURE (Data,Var,Abs,NPrf)        */
	0x75, 0x06, /*   REPORT_SIZE (6)                    */
	0x95, 0x01, /*   REPORT_COUNT (1)                   */
	0xb1, 0x01, /*   FEATURE (Cnst,Ary,Abs)             */
	0xc0        /* END_COLLECTION                       */
};


static const struct {
	struct usb_hid_descriptor hid_descriptor;
	struct {
		uint8_t bReportDescriptorType;
		uint16_t wDescriptorLength;
	} __attribute__((packed)) hid_report;
} __attribute__((packed)) hid_function = {
	.hid_descriptor = {
		.bLength = sizeof(hid_function),
		.bDescriptorType = USB_DT_HID,
		.bcdHID = 0x0100,
		.bCountryCode = 0,
		.bNumDescriptors = 1,
	},
	.hid_report = {
		.bReportDescriptorType = USB_DT_REPORT,
		.wDescriptorLength = sizeof(hid_report_descriptor),
	}
};


const struct usb_endpoint_descriptor hid_endpoint[] = {{
	.bLength = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,
	.bEndpointAddress = 0x01,
	.bmAttributes = USB_ENDPOINT_ATTR_BULK,
	.wMaxPacketSize = 64,
	.bInterval = 0x1,
},
{
	.bLength = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,
	.bEndpointAddress = 0x81,
	.bmAttributes = USB_ENDPOINT_ATTR_INTERRUPT,
	.wMaxPacketSize = 64,
	.bInterval = 0x1,
}
}
;


static usbd_device * _dev;
/* Fucking USB Descriptor shit 
 * @_@
 */
uint8_t usbd_control_buffer[128];

static const struct usb_interface_descriptor stick_control_iface[] ={{
	.bLength = USB_DT_INTERFACE_SIZE,
		.bDescriptorType = USB_DT_INTERFACE,
		.bInterfaceNumber = 0,
		.bAlternateSetting = 0,
		.bNumEndpoints = 2,
		.bInterfaceClass = USB_CLASS_HID,
		.bInterfaceSubClass = USB_HID_SUBCLASS_NO,
		.bInterfaceProtocol = 0, // ? 
		.endpoint = hid_endpoint,
        .extra = &hid_function,
        .extralen = sizeof(hid_function)
}};
static const struct usb_interface ifaces[]={
	{
		.num_altsetting = 1,
		.altsetting = stick_control_iface,
	}
};
static const struct usb_config_descriptor config = {
	.bLength = USB_DT_CONFIGURATION_SIZE,
	.bDescriptorType = USB_DT_CONFIGURATION,
	.wTotalLength = 0,
	.bNumInterfaces = 1,
	.bConfigurationValue = 1,
	.iConfiguration = 0,
	.bmAttributes = 0x80,
	.bMaxPower = 0x32,
	.interface = ifaces,

};

static void toggle_led(void)
{

	gpio_toggle(GPIOC, GPIO13); /* led on/off */
	gpio_toggle(GPIOB, GPIO9);  /* led on/off */
	for (int i = 0; i < 8400000; i++) { /* wait a bit. */
		__asm__("nop");
	}

	gpio_toggle(GPIOC, GPIO13); /* Led on/off */
	gpio_toggle(GPIOB, GPIO9);  /* Led on/off */
}

static void stick_rx(usbd_device *usbd_dev,uint8_t ep)
{
	(void)ep;
	char buf[64];
	int len = usbd_ep_read_packet(usbd_dev, 0x01, buf, 64);
	toggle_led();
	if (len) {
		while (usbd_ep_write_packet(usbd_dev, 0x82, buf, len) == 0);
	}

}

static void stick_tx(usbd_device * usbd_dev, uint8_t ep)
{
    char buf[64];
    (void)ep;
    /* Keep sending packets */
    usbd_ep_write_packet(usbd_dev, 0x82, buf, 64);
    toggle_led();

}

static enum usbd_request_return_codes hid_control_request(usbd_device *dev, struct usb_setup_data *req, uint8_t **buf, uint16_t *len,
			void (**complete)(usbd_device *, struct usb_setup_data *))
{
	(void)complete;
	(void)dev;

	if((req->bmRequestType != 0x81) ||
	   (req->bRequest != USB_REQ_GET_DESCRIPTOR) ||
	   (req->wValue != 0x2200))
		return USBD_REQ_NOTSUPP;

	/* Handle the HID report descriptor. */
	*buf = (uint8_t *)hid_report_descriptor;
	*len = sizeof(hid_report_descriptor);

	return USBD_REQ_HANDLED;
}

static void stick_config(usbd_device *usbd_dev, uint16_t value)
{
	(void) value;
	usbd_ep_setup(usbd_dev, 0x01,USB_ENDPOINT_ATTR_BULK,64,stick_rx);
	usbd_ep_setup(usbd_dev, 0x81,USB_ENDPOINT_ATTR_INTERRUPT ,4,NULL);
	usbd_register_control_callback(
				usbd_dev ,
				USB_REQ_TYPE_STANDARD | USB_REQ_TYPE_INTERFACE,
				USB_REQ_TYPE_TYPE | USB_REQ_TYPE_RECIPIENT,
				hid_control_request);
}

static void gpio_setup(void)
{
	rcc_clock_setup_pll(&rcc_hse_25mhz_3v3[RCC_CLOCK_3V3_84MHZ]);
	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_OTGFS);
	rcc_periph_clock_enable(RCC_GPIOB);
	rcc_periph_clock_enable(RCC_GPIOC);
	gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO11 | GPIO12);
	gpio_set_af(GPIOA, GPIO_AF10, GPIO11 | GPIO12);

	gpio_mode_setup(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO9);
	gpio_mode_setup(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO13);
	
}
int main(void) {
	gpio_setup();
	gpio_toggle(GPIOC, GPIO13);

	_dev = usbd_init(&otgfs_usb_driver, &dev_descr, &config,
                     usb_string, 3,usbd_control_buffer,sizeof(usbd_control_buffer));
	//Without VBUS sense 
	OTG_FS_GCCFG |= OTG_GCCFG_NOVBUSSENS;
	usbd_register_set_config_callback(_dev, stick_config);
	toggle_led();
	printf("Debug Test");
	while (1) {
		usbd_poll(_dev);
	}
	return 1;
}

