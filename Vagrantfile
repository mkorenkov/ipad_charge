# -*- mode: ruby -*-
# vi: set ft=ruby :

Vagrant::Config.run do |config|
    config.vm.define :icharge do |icharge|
        icharge.vm.network :hostonly, "10.100.0.101", :netmask => "255.255.0.0"
        icharge.vm.host_name = "tow-all"
        icharge.vm.box = "precise64"
        icharge.vm.box_url = "http://files.vagrantup.com/precise64.box"
	icharge.vm.forward_port 22, 2290
        icharge.vm.provision :shell, :path => "provision.sh"
        icharge.vm.customize [
            "modifyvm", :id,
            "--name", "iCharge VM",
            "--memory", "768",
            "--natdnshostresolver1", "on",
            "--usb", "on",
            "--usbehci", "on"
        ]
    end
end

