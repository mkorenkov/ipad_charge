Apple charger utility 
======================
USB charging control utility used to enable/disable charging of an Apple device connected to USB port.

Supported devices:
------------------
* iPad
* iPad2
* iPad3
* iPad4
* iPad mini
* iPad Air
* iPad mini 2
* iPad Air 2
* iPad mini 3
* iPad mini 4
* iPad 5
* iPad 6
* iPad Air 3
* iPad mini 5
* iPad 7
* iPod Touch 2G
* iPod Touch 3G
* iPod Touch 4G
* iPhone 3GS
* iPhone 4
* iPhone 4S
* iPhone 5
* iPhone 5C
* iPhone 5S
* iPhone 6
* iPhone SE

Requirements:
-------------
Linux, GCC, Make, libusb-1.0

Wiki:
-----
[https://github.com/mkorenkov/ipad_charge/wiki](https://github.com/mkorenkov/ipad_charge/wiki)

libusb-1.0
----------
[git://git.libusb.org/libusb.git](Official Repository)

If you use Fedora 17 x86_64, just compile libusb-1.0 from official repository then you will be able to `make` ipad_charge.

For Fedora 18 i686 or x86_64, you will need libusbx devel installed:
```
sudo yum install libusbx-devel libusbx-devel.i686
```

Authors & Maintainers:
-------
* Ondrej Zary
* Max Korenkov
* Pranava Swaroop

