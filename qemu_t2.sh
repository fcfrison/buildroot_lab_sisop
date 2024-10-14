rm output/build/linux-custom/.stamp_*
export LINUX_OVERRIDE_SRCDIR=/workspaces/linux-4.13.9/
cd modules/sstf-iosched
make
cd ../../
make
sudo qemu-system-i386 --kernel output/images/bzImage --hda output/images/rootfs.ext2 --hdb sdb.bin --nographic --append "console=ttyS0 root=/dev/sda"
