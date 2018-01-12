CFLAGS ?= -O2
ipad_charge: ipad_charge.c
	gcc \
		-fstrict-aliasing \
		-fstack-protector \
		-Werror \
		-Wstack-protector \
		-Wno-multichar \
		-Wimplicit-fallthrough \
		-Wduplicated-branches \
		-Wduplicated-cond \
		-Wfloat-equal \
		-Wunused-macros \
		-Wbad-function-cast \
		-Wcast-qual \
		-Wmisleading-indentation \
		-Wall \
		-Wextra \
		-Wuninitialized \
		-Wconversion \
		-Wlogical-op \
		-Wlogical-not-parentheses \
		-Waggregate-return \
		-Wdangling-else \
		-Wunused-parameter \
		-std=gnu1x \
		-pedantic \
		-pedantic-errors \
		-Wswitch-default \
		-Wswitch-enum \
		-Wswitch-bool \
		$(CFLAGS) \
			ipad_charge.c -lusb-1.0 -o ipad_charge

install: ipad_charge
	install -o root -g root -m 755 -s ipad_charge /usr/bin/
	install -o root -g root -m 644 95-ipad_charge.rules /etc/udev/rules.d/

uninstall:
	rm -rf /usr/bin/ipad_charge
	rm -rf /etc/udev/rules.d/95-ipad_charge.rules

clean:
	rm -f ipad_charge
