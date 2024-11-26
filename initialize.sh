
# create an image containing the executable games in the bin

# check for dependencies and install it if it is not installed
if ! dpkg -l | grep -q ncurses; then
  echo "ncurses library is not installed. Installing now..."
  sudo apt update && sudo apt install -y libncurses5-dev libncursesw5-dev
fi

# compile the .c files in src and put the executables in bin
mkdir -p bin
for src_file in src/*.c; do
  file=$(basename "$src_file" .c)
  gcc -o "./bin/${file}" "$src_file" -lncurses  # Compile with ncurses
done

# create (or override) the image file and format it with ext4
dd if=/dev/zero of=storage_vgc.img bs=1M count=50 conv=notrunc
sudo mkfs.ext4 storage_vgc.img

mkdir -p mount
sudo mount -o loop storage_vgc.img mount/

sudo chown -R "$(whoami):$(whoami)" mount
sudo chmod -R 777 mount

# copy the executables to the mounted image
cp bin/* mount/

# unmount the image and clean up
sudo umount mount
sudo rm -rf mount




