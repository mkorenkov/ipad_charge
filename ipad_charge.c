#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifndef _WIN32
#include <syslog.h>
#include <sys/param.h>
#else
#include <windows.h>
#define min MIN
#define syslog(...)
#endif

#include <libusb-1.0/libusb.h>

#define VERSION "1.1"

#define CTRL_OUT	(LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_OUT)
#define VENDOR_APPLE		0x05ac

#define PRODUCT_IPAD1		0x129a
#define PRODUCT_IPAD2		0x129f
#define PRODUCT_IPAD2_3G	0x12a2
#define PRODUCT_IPAD2_4		0x12a9
#define PRODUCT_IPAD2_3GV	0x12a3
#define PRODUCT_IPAD3		0x12a4
#define PRODUCT_IPAD3_4G	0x12a6
#define PRODUCT_IPAD4		0x12ab
#define PRODUCT_IPOD_TOUCH_2G	0x1293
#define PRODUCT_IPHONE_3GS		0x1294
#define PRODUCT_IPHONE_4_GSM	0x1297
#define PRODUCT_IPOD_TOUCH_3G	0x1299
#define PRODUCT_IPHONE_4_CDMA	0x129c
#define PRODUCT_IPOD_TOUCH_4G	0x129e
#define PRODUCT_IPHONE_4S		0x12a0
#define PRODUCT_IPHONE_5		0x12a8

int set_charging_mode(libusb_device *dev, bool enable) {
	int ret;
	struct libusb_device_handle *dev_handle;

	if ((ret = libusb_open(dev, &dev_handle)) < 0) {
		syslog(LOG_ERR, "ipad_charge: unable to open device: error %d\n", ret);
		syslog(LOG_ERR, "ipad_charge: %s\n", libusb_strerror(ret));
		return ret;
	}

	if ((ret = libusb_claim_interface(dev_handle, 0)) < 0) {
		fprintf(stderr, "ipad_charge: unable to claim interface: error %d\n", ret);
		fprintf(stderr, "ipad_charge: %s\n", libusb_strerror(ret));
		goto out_close;
	}

	// the 4th and 5th numbers are the extra current in mA that the Apple device may draw in suspend state.
	// Originally, the 5th was 0x6400, or 25600mA. I believe this was a bug and they meant 0x640, or 1600 mA which would be the max
	// for the MFi spec. Also the typical values for the 4th listed in the MFi spec are 0, 100, 500 so I chose 500 for that.
	// And changed it to decimal to be clearer.
	uint16_t index = enable ? 1600 : 0; // 2100 mA total
	if (getenv("AMPS") && enable) {
		int input = atoi(getenv("AMPS"));
		input = input < 0 ? 0 : input;
		index = (uint16_t)MIN(1900, input); // maximum 2400 mA
	}
	const uint16_t wvalue_unknown = 500;
	const unsigned int timeout = 2000; // ms
	if ((ret = libusb_control_transfer(dev_handle, CTRL_OUT, LIBUSB_REQUEST_TYPE_VENDOR, wvalue_unknown, index, NULL, 0, timeout)) < 0) {
		syslog(LOG_ERR, "ipad_charge: %s\n", libusb_strerror(ret));
		goto out_release;
	}
	else {
		syslog(LOG_INFO, "Sucessfully set charging mode for device (+%d mA)", index);
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

	setlogmask(LOG_UPTO(LOG_INFO));
	openlog("ipad_charge", LOG_CONS | LOG_NDELAY | LOG_PID, LOG_LOCAL0);

	if (libusb_init(NULL) < 0) {
		fprintf(stderr, "ipad_charge: failed to initialise libusb\n");
		syslog(LOG_ERR, "failed to initialise libusb");
		exit(1);
	}

	libusb_device **devs;
	if (libusb_get_device_list(NULL, &devs) < 0) {
		fprintf(stderr, "ipad_charge: unable to enumerate USB devices\n");
		syslog(LOG_ERR, "unable to enumerate USB devices");
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
			    	if (set_charging_mode(dev, enable) < 0) {
						fprintf(stderr, "ipad_charge: error setting charge mode\n");
						syslog(LOG_ERR, "error setting charge mode");
					}
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
				fprintf(stderr, "ipad_charge: failed to get device descriptor: %s\n", libusb_strerror(ret));
				syslog(LOG_ERR, "failed to get device descriptor: %s", libusb_strerror(ret));
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
				if (set_charging_mode(dev, enable) < 0) {
					fprintf(stderr, "ipad_charge: error setting charge mode\n");
					syslog(LOG_ERR, "error setting charge mode");
                }
				else
					count++;
			}
		}
	}

	if (count < 1) {
		fprintf(stderr, "ipad_charge: no such device or an error occured");
		syslog(LOG_ERR, "no such device or an error occured");
		ret = 3;
	} else
		ret = 0;

	libusb_free_device_list(devs, 1);
out_exit:
	libusb_exit(NULL);

	return ret;
}
