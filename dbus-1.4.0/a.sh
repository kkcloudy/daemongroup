#/bin/bash
cd ../.. ; make dbus ; cd -
cp ./dbus/.libs/libdbus-1.a .
mips-linux-gnu-gcc test_dbus.c libdbus-1.a -I.
