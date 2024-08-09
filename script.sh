#!/usr/bin/bash

DISK_FILE="./c.img"
MOUNT_DIR="/mnt/virtual"

sudo losetup -d /dev/loop0
sudo losetup -d /dev/loop1
sudo losetup -d /dev/loop2

sudo umount $MOUNT_DIR

rm -rf $DISK_FILE
qemu-img create -f raw $DISK_FILE 32M

(
  echo x;
    echo ;
    echo ;
    echo ;
    echo ;
    echo ;
    echo ;
  echo r;

  echo n;
  echo p;
  echo 1;
  echo ;
  echo ;

  echo a;

  echo w;
) | fdisk "$DISK_FILE"

OFFSET=$(sudo fdisk -l $DISK_FILE | grep "^$DISK_FILE" | awk '{print $3*512}')
# SIZE=$(sudo fdisk -l $DISK_FILE | grep "^$DISK_FILE" | awk '{print $4*512}')
SIZE=$(echo $OFFSET 15728640 | awk '{print $1+$2}')
OFF2=$(echo $OFFSET $SIZE | awk '{print $1+$2}')
# OFF2=32256
echo $OFFSET $SIZE $OFF2

sudo losetup /dev/loop0 $DISK_FILE
sudo losetup -o $OFFSET --sizelimit $SIZE /dev/loop1 $DISK_FILE
# sudo losetup -o $OFF2 /dev/loop2 $DISK_FILE

sudo mkdir -p $MOUNT_DIR
# sudo mke2fs -O ^resize_inode /dev/loop1
sudo mke2fs /dev/loop1
sudo mount -t ext2 /dev/loop1 $MOUNT_DIR

sudo mkdir -p $MOUNT_DIR/boot/grub

echo "(hd0) /dev/loop0
(hd0,1) /dev/loop1
" | sudo tee $MOUNT_DIR/boot/grub/device.map

echo 'set default=0
set timeout=0
set root=(hd0,1)
menuentry "AdaKernel" {
  multiboot /boot/kernel
  boot
}' | sudo tee $MOUNT_DIR/boot/grub/grub.cfg

sudo cp kern/kernel $MOUNT_DIR/boot

sudo grub-install --target=i386-pc --no-floppy --root-directory=$MOUNT_DIR \
--modules="part_msdos ext2 biosdisk configfile normal multiboot multiboot2" \
/dev/loop0

# sudo losetup -d /dev/loop0
# sudo losetup -d /dev/loop1
# sudo losetup -d /dev/loop2
#
# sudo umount $MOUNT_DIR
