#/usr/bin/bash

DISK_FILE="./c.img"

rm -rf $DISK_FILE
sudo losetup -d /dev/loop0
sudo losetup -d /dev/loop1
sudo losetup -d /dev/loop2

sudo umount /mnt/virtual
sudo rm -rf /mnt/virtual

# bximage -hd -flat 10 - c.img
qemu-img create -f raw $DISK_FILE 10M

# fdisk c.img x x x -c 20 -h 16 s 63 - r n, p, 1 a 1 w w w
(
  echo x;
    echo c;
    echo 20;
    echo h;
    echo 16;
    echo s;
    echo 63;
  echo r;

  echo n;
  echo p;
  echo 1
  echo ;
  echo ;

  echo w;
) | fdisk "$DISK_FILE"

sudo losetup /dev/loop0 $DISK_FILE
sudo losetup -o 32256 --sizelimit 5644800 /dev/loop1 $DISK_FILE
sudo losetup -o 5677056 /dev/loop2 $DISK_FILE

sudo mkdir /mnt/virtual
sudo mke2fs -O ^resize_inode /dev/loop1
sudo mount -t ext2 /dev/loop1 /mnt/virtual

sudo mkdir -p /mnt/virtual/boot/grub
echo "
(hd0) /dev/loop0
(hd0,1) /dev/loop1
(hd0,2) /dev/loop2
" | sudo tee /mnt/virtual/boot/grub/device.map

echo '
set default=0
set timeout=5
set root=(hd0,1)
menuentry "AdaKernel" {
  multiboot /boot/kernel
  boot
}
' | sudo tee /mnt/virtual/boot/grub/grub.cfg

sudo cp kern/kernel /mnt/virtual/boot

sudo grub-install --target=i386-pc --no-floppy --root-directory=/mnt/virtual \
--modules="part_msdos ext2 biosdisk configfile normal multiboot multiboot2" \
/dev/loop0

