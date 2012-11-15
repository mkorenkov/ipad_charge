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
#define PRODUCT_IPAD1		0x129a
#define PRODUCT_IPAD2		0x129f
#define PRODUCT_IPAD2_3G	0x12a2
#define PRODUCT_IPAD2_4		0x12a9
#define PRODUCT_IPAD2_3GV	0x12a3
#define PRODUCT_IPAD3	    0x12a4
#define PRODUCT_IPAD3_4G    0x12a6
#define PRODUCT_IPAD4       0x12ab

#define PRODUCT_IPOD_TOUCH_2G 0x1293
#define PRODUCT_IPHONE_3GS 0x1294
#define PRODUCT_IPHONE_4_GSM 0x1297
#define PRODUCT_IPOD_TOUCH_3G 0x1299
#define PRODUCT_IPHONE_4_CDMA 0x129c
#define PRODUCT_IPOD_TOUCH_4G 0x129e
#define PRODUCT_IPHONE_4S 0x12a0
#define PRODUCT_IPHONE_5 0x12a8

int set_charging_mode(libusb_device *dev, bool enable) {
	int ret;
	struct libusb_device_handle *dev_handle;

	if ((ret = libusb_open(dev, &dev_handle)) < 0) {
		fprintf(stderr, "ipad_charge: unable to open device: error %d\n", ret);
		return ret;
	}

	if ((ret = libusb_claim_interface(dev_handle, 0)) < 0) {
		fprintf(stderr, "ipad_charge: unable to claim interface: error %d\n", ret);
		goto out_close;
	}

	if ((ret = libusb_control_transfer(dev_handle, CTRL_OUT, 0x40, 0x6400, enable ? 0x6400 : 0, NULL, 0, 2000)) < 0) {
		fprintf(stderr, "ipad_charge: unable to send command: error %d\n", ret);
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
                        fprintf(stderr, "Try '%s --help' for more information.\n", argv[0]);
                        exit(100);
                }
        }

	if (getenv("BUSNUM") && getenv("DEVNUM")) {
		busnum = atoi(getenv("BUSNUM"));
		devnum = atoi(getenv("DEVNUM"));
	}

	if (libusb_init(NULL) < 0) {
		fprintf(stderr, "ipad_charge: failed to initialise libusb\n");
		exit(1);
	}

	libusb_device **devs;
	if (libusb_get_device_list(NULL, &devs) < 0) {
		fprintf(stderr, "ipad_charge: unable to enumerate USB devices\n");
		ret = 2;
		goto out_exit;
	}

	libusb_device *dev;
	int i = 0, count = 0;
	/* if BUSNUM and DEVNUM were specified (by udev), find device by address */
	if (busnum && devnum) {
		while ((dev = devs[i++]) != NULL) {
			if (libusb_get_bus_number(dev) == busnum &&
			    libusb_get_device_address(dev) == devnum) {
			    	if (set_charging_mode(dev, enable) < 0)
			    		fprintf(stderr, "ipad_charge: error setting charge mode\n");
				else
					count++;
				break;
			}
		}
	/* otherwise apply to all devices */
	} else {
		while ((dev = devs[i++]) != NULL) {
			struct libusb_device_descriptor desc;
			if ((ret = libusb_get_device_descriptor(dev, &desc)) < 0) {
				fprintf(stderr, "ipad_charge: failed to get device descriptor: error %d\n", ret);
				continue;
			}
			if (desc.idVendor == VENDOR_APPLE && (desc.idProduct == PRODUCT_IPAD1
					|| desc.idProduct == PRODUCT_IPAD2
					|| desc.idProduct == PRODUCT_IPAD2_3G
					|| desc.idProduct == PRODUCT_IPAD2_4
					|| desc.idProduct == PRODUCT_IPAD2_3GV
					|| desc.idProduct == PRODUCT_IPAD3
					|| desc.idProduct == PRODUCT_IPAD3_4G
					|| desc.idProduct == PRODUCT_IPOD_TOUCH_2G
					|| desc.idProduct == PRODUCT_IPHONE_3GS
					|| desc.idProduct == PRODUCT_IPHONE_4_GSM
					|| desc.idProduct == PRODUCT_IPOD_TOUCH_3G
					|| desc.idProduct == PRODUCT_IPHONE_4_CDMA
					|| desc.idProduct == PRODUCT_IPOD_TOUCH_4G
					|| desc.idProduct == PRODUCT_IPHONE_4S
					|| desc.idProduct == PRODUCT_IPHONE_5
					|| desc.idProduct == PRODUCT_IPAD4)) {

				if (set_charging_mode(dev, enable) < 0)
					fprintf(stderr, "ipad_charge: error setting charge mode\n");
				else
					count++;
			}
		}
	}

	if (count < 1) {
		fprintf(stderr, "ipad_charge: no such device or an error occured\n");
		ret = 3;
	} else
		ret = 0;

	libusb_free_device_list(devs, 1);
out_exit:
	libusb_exit(NULL);

	return ret;
}
