#!/bin/bash
cd ./custom-scripts/webserver/
../../output/host/bin/i686-buildroot-linux-gnu-cc server.c -O2 -o server
../../output/host/bin/i686-buildroot-linux-gnu-cc app.c -O2 -o app
cp server app ../../output/target/usr/bin/
cd ../../
make
sudo qemu-system-i386 \
    --device e1000,netdev=eth0,mac=aa:bb:cc:dd:ee:ff \
	--netdev tap,id=eth0,script=custom-scripts/qemu-ifup \
	--kernel output/images/bzImage \
	--hda output/images/rootfs.ext2 \
	--nographic \
	--append "console=ttyS0 root=/dev/sda"