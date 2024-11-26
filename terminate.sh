
# unmount the vgc-disk image and remove all files created by startup.sh
# (detach the device file, remove it and its symbolic link)

sudo umount mount
sudo rm /dev/vgc-disk
rm -r mount
rm vgc-disk

if sudo losetup -a | grep -q "storage_vgc.img"; then
    loop_device=$(sudo losetup -a | grep "storage_vgc.img" | cut -d ':' -f 1)
    sudo losetup -d "$loop_device"
fi

