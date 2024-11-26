
# along with the removals of terminate.sh, also remove the created img file

sudo umount mount
sudo rm /dev/vgc-disk
rm -r mount
rm vgc-disk
if sudo losetup -a | grep -q "storage_vgc.img"; then
    loop_device=$(sudo losetup -a | grep "storage_vgc.img" | cut -d ':' -f 1)
    sudo losetup -d "$loop_device"
fi

rm storage_vgc.img
