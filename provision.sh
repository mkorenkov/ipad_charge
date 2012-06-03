#!/bin/sh

apt-get install -y build-essential libusb-1.0-0 libusb-1.0-0-dev vim git-core git-doc
git clone https://github.com/mkorenkov/ipad_charge.git
cd ipad_charge
make
make install
cd -
