ipad_charge - iPad USB charging control utility

Used to enable/disable charging of an iPad connected to USB port.

Requirements:
-------------
Linux, GCC, Make, libusb-1.0

Compilation:
------------
$ make

Installation:
-------------
# make install

This installs ipad_charge to /usr/bin and 95-ipad_charge.rules to
/etc/udev/rules.d.
The udev rule automatically enables charging when an iPad is connected.

Simply running ipad_charge with no arguments enables charging on all connected
iPads. Running with argument --off disables charging. If you want to control
only one device, set environment variables BUSNUM and DEVNUM, e.g.:
$ BUSNUM=004 DEVNUM=014 ipad_charge --off
