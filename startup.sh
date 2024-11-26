
# given an image, mount the image and make the virtual filesystem ready to run

# create a device file and link it
#sudo rm -r /dev/vgc-disk
sudo mknod /dev/vgc-disk b 7 0   # vgc -> virtual game console
ln -s /dev/vgc-disk vgc-disk

# attach the image to the device file
sudo losetup /dev/vgc-disk storage_vgc.img

# mount the image
mkdir -p mount
sudo mount /dev/vgc-disk mount/

sudo chown -R "$(whoami):$(whoami)" mount   # change the owner from root to current user
sudo chmod -R 777 mount

