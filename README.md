Virtual game console designed to be run on Linux terminal. Initialize.sh creates an image representing a video game disk, and compiles and loads the source files in the src directory into the disk. Startup.sh is effectively plugging the disk in, mounting it to the current directory. Terminate.sh is taking the disk out. Purge.sh is deleting the image and completely. 

Execution instructions:
./initialize.sh
./startup.sh
cd mount
./game_<name>

