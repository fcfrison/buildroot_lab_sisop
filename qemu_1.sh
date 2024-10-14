cd ./custom-scripts/read_from_file/
../../output/host/bin/i686-buildroot-linux-gnu-cc read_f_file.c -O2 -o file_reader
cp file_reader ../../output/target/usr/bin/
cd ../../
rm output/build/linux-custom/.stamp_*
export LINUX_OVERRIDE_SRCDIR=/workspaces/linux-4.13.9/
make
sudo qemu-system-i386 --kernel output/images/bzImage --hda output/images/rootfs.ext2 --hdb sdb.bin --nographic --append "console=ttyS0 root=/dev/sda"