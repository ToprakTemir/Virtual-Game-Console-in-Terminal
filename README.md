# Virtual Game Console in Terminal  

**A virtual game console designed to run directly within the Linux terminal.**  

This project simulates a game console experience, complete with commands to create, load, mount, and delete virtual disk images that house your game files.  

---

## Features  

- **Disk Management**: Simulate game disk creation and removal. You can directly plug in disk images created by other people and play their game.
- **Game Loading**: Automatically compile and load games from the `src` directory into the disk. New source codes for more games can be added.
- **Simple Mounting**: Plug and play functionality for game disks.  
- **Terminal Gameplay**: Run games directly from the terminal.  

---

## Quick Start  

Follow these steps to get started:  

```bash
# Clone the repository
git clone https://github.com/ToprakTemir/Virtual-Game-Console-in-Terminal.git

# Navigate into the project directory
cd Virtual-Game-Console-in-Terminal.git

# Create a virtual disk image
./initialize.sh

# Plug in the virtual disk and mount it
./startup.sh

# Navigate to the mounted disk
cd mount

# Run a game
./game_<name>
