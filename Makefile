ipad_charge: ipad_charge.c
	gcc -Wall -Wextra ipad_charge.c -lusb-1.0 -o ipad_charge

install: ipad_charge
	install -o root -g root -m 755 -s ipad_charge /usr/bin/
	install -o root -g root -m 644 95-ipad_charge.rules /etc/udev/rules.d/

uninstall:
	rm -rf /usr/bin/ipad_charge
	rm -rf /etc/udev/rules.d/95-ipad_charge.rules

clean:
	rm -f ipad_charge
