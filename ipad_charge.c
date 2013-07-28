#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libusb-1.0/libusb.h>

#define VERSION "1.1"

#define CTRL_OUT	(LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_OUT)
#define VENDOR_APPLE		0x05ac
const uint16_t PRODUCT_APPLE[] = {
	0x129a,		/* PRODUCT_IPAD1 */
	0x129f,		/* PRODUCT_IPAD2 */
	0x12a2,		/* PRODUCT_IPAD2_3G */
	0x12a9,		/* PRODUCT_IPAD2_4 */
	0x12a3,		/* PRODUCT_IPAD2_3GV */
	0x12a4,		/* PRODUCT_IPAD3 */
	0x12a6,		/* PRODUCT_IPAD3_4G */
	0x12ab,		/* PRODUCT_IPAD4 */

	0x1293,		/* PRODUCT_IPOD_TOUCH_2G */
	0x1294,		/* PRODUCT_IPHONE_3GS */
	0x1297,		/* PRODUCT_IPHONE_4_GSM */
	0x1299,		/* PRODUCT_IPOD_TOUCH_3G */
	0x129c,		/* PRODUCT_IPHONE_4_CDMA */
	0x129e,		/* PRODUCT_IPOD_TOUCH_4G */
	0x12a0,		/* PRODUCT_IPHONE_4S */
	0x12a8,		/* PRODUCT_IPHONE_5 */
};

#define ERROR(fmt, ...)	do {	\
	fprintf(stderr, "ipad_charge: %s#%d: " fmt, __func__, __LINE__, ## __VA_ARGS__); \
} while (0)

int is_apple_product(uint16_t productId) {
	unsigned int i;

	for (i = 0; i < sizeof(PRODUCT_APPLE)/sizeof(PRODUCT_APPLE[0]); i++) {
		if (productId == PRODUCT_APPLE[i]) {
			return 1;
		}
	}

	return 0;
}

int set_charging_mode(libusb_device *dev, bool enable) {
	int ret;
	struct libusb_device_handle *dev_handle;

	if ((ret = libusb_open(dev, &dev_handle)) < 0) {
		ERROR("unable to open device: error %d\n", ret);
		return ret;
	}

	if ((ret = libusb_claim_interface(dev_handle, 0)) < 0) {
		ERROR("unable to claim interface: error %d\n", ret);
		goto out_close;
	}

	if ((ret = libusb_control_transfer(dev_handle, CTRL_OUT, 0x40, 0x6400, enable ? 0x6400 : 0, NULL, 0, 2000)) < 0) {
		ERROR("unable to send command: error %d\n", ret);
		goto out_release;
	}
	
	ret = 0;

out_release:
	libusb_release_interface(dev_handle, 0);
out_close:
	libusb_close(dev_handle);

	return ret;
}

void help(char *progname) {
	printf("Usage: %s [OPTION]\n", progname);
	printf("iPad USB charging control utility\n\n");
	printf("Available OPTIONs:\n");
	printf("  -0, --off\t\t\tdisable charging instead of enabling it\n");
	printf("  -h, --help\t\t\tdisplay this help and exit\n");
	printf("  -V, --version\t\t\tdisplay version information and exit\n");
	printf("\nExamples:\n");
	printf("  ipad_charge\t\t\t\t\tenable charging on all connected iPads\n");
	printf("  BUSNUM=004 DEVNUM=014 ipad_charge -off\tdisable charging on iPad connected on bus 4, device 14\n");
}

void version() {
	printf("ipad_charge v%s - iPad USB charging control utility\n", VERSION);
	printf("Copyright (c) 2010 Ondrej Zary - http://www.rainbow-software.org\n");
	printf("License: GLPv2\n");
}

int main(int argc, char *argv[]) {
	int ret, devnum = 0, busnum = 0;
	bool enable = 1;
	char *subsys;
	libusb_device **devs;
	libusb_device *dev;
	int i = 0, count = 0;

	while (1) {
                struct option long_options[] = {
                        { .name = "off",	.has_arg = 0, .val = '0' },
                        { .name = "help",	.has_arg = 0, .val = 'h' },
                        { .name = "version",	.has_arg = 0, .val = 'V' },
                        { .name = NULL },
                };
                int opt = getopt_long(argc, argv, "0hV", long_options, NULL);
                if (opt < 0)
                        break;
                switch (opt) {
                case '0':
                        enable = 0;
                        break;
                case 'h':
                        help(argv[0]);
                        exit(0);
                case 'V':
                        version();
                        exit(0);
                default:
                        ERROR("Try '%s --help' for more information.\n", argv[0]);
                        exit(100);
                }
        }

	if (libusb_init(NULL) < 0) {
		ERROR("failed to initialise libusb\n");
		exit(1);
	}

	if (libusb_get_device_list(NULL, &devs) < 0) {
		ERROR("unable to enumerate USB devices\n");
		ret = 2;
		goto out_exit;
	}

	subsys = getenv("SUBSYSTEM");
	/* if called by hotplug system */
	if (subsys && !strcmp(subsys, "usb")) {
		if (getenv("BUSNUM") && getenv("DEVNUM")) {
			busnum = atoi(getenv("BUSNUM"));
			devnum = atoi(getenv("DEVNUM"));
		}

		while ((dev = devs[i++]) != NULL) {
			if (libusb_get_bus_number(dev) == busnum &&
					libusb_get_device_address(dev) == devnum) {
				if (set_charging_mode(dev, enable) < 0)
					ERROR("error setting charge mode\n");
				else
					count++;
				break;
			}
		}
	}
	/* otherwise apply to all devices */
	else {
		while ((dev = devs[i++]) != NULL) {
			struct libusb_device_descriptor desc;
			if ((ret = libusb_get_device_descriptor(dev, &desc)) < 0) {
				ERROR("failed to get device descriptor: error %d\n", ret);
				continue;
			}
			if (desc.idVendor == VENDOR_APPLE && is_apple_product(desc.idProduct)) {
				if (set_charging_mode(dev, enable) < 0)
					ERROR("error setting charge mode\n");
				else
					count++;
			}
		}
	}

	if (count < 1) {
		ERROR("no such device or an error occured\n");
		ret = 3;
	} else
		ret = 0;

	libusb_free_device_list(devs, 1);
out_exit:
	libusb_exit(NULL);

	return ret;
}
