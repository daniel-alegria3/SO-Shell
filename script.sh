#!/usr/bin/bash

DISK_FILE="./c.img"
MOUNT_DIR="/mnt/virtual"

# Eliminar imagen de disco previa y limpiar dispositivos de bucle
rm -rf $DISK_FILE
if losetup -a | grep -q "/dev/loop0"; then sudo losetup -d /dev/loop0; fi
if losetup -a | grep -q "/dev/loop1"; then sudo losetup -d /dev/loop1; fi
if losetup -a | grep -q "/dev/loop2"; then sudo losetup -d /dev/loop2; fi

sudo umount $MOUNT_DIR 2>/dev/null
sudo rm -rf $MOUNT_DIR

# Crear una imagen de disco más grande, por ejemplo 100 MB
qemu-img create -f raw $DISK_FILE 100M

# Particionar la imagen de disco
(
  echo x;
    echo c;
    echo 4;
    echo h;
    echo 16;
    echo s;
    echo 63;
  echo r;

  echo n;
  echo p;
  echo 1;
  echo ;
  echo ;

  echo w;
) | fdisk "$DISK_FILE"

# Usar el comando `partx` para verificar el offset y tamaño exactos
sudo partx --show $DISK_FILE

# Configurar dispositivos de bucle
OFFSET=$(sudo fdisk -l $DISK_FILE | grep "^$DISK_FILE" | awk '{print $2*512}')
SIZE=$(sudo fdisk -l $DISK_FILE | grep "^$DISK_FILE" | awk '{print $4*512}')
sudo losetup /dev/loop0 $DISK_FILE
sudo losetup -o $OFFSET --sizelimit $SIZE /dev/loop1 $DISK_FILE

# Crear el sistema de archivos y montar la partición
sudo mkdir $MOUNT_DIR
sudo mke2fs -O ^resize_inode /dev/loop1
sudo mount -t ext2 /dev/loop1 $MOUNT_DIR

# Configurar GRUB
sudo mkdir -p $MOUNT_DIR/boot/grub
echo "
(hd0) /dev/loop0
(hd0,1) /dev/loop1
" | sudo tee $MOUNT_DIR/boot/grub/device.map

echo '
set default=0
set timeout=5
set root=(hd0,1)
menuentry "AdaKernel" {
  multiboot /boot/kernel
  boot
}
' | sudo tee $MOUNT_DIR/boot/grub/grub.cfg

# Copiar kernel a la partición
sudo cp kern/kernel $MOUNT_DIR/boot

# Instalar GRUB
sudo grub-install --target=i386-pc --no-floppy --root-directory=$MOUNT_DIR \
--modules="part_msdos ext2 biosdisk configfile normal multiboot multiboot2" \
/dev/loop0

