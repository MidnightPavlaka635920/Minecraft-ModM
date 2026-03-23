# Minecraft-ModM
A CLI tool for managing Minecraft mods/plugins for servers.
## About
This project targets server owners, because it might be hard to install mods/plugins in a CLI server interface. I had that problem, and I hated when I had to update my server. So, this is the solution.
## Dependencies
- [libcurl](https://curl.se/libcurl/) (for downloading mods/plugins)  
- A C++ compiler with C++20 support (e.g., `g++`)  
- `make` utility
- nlohmann/json library
## Binary name
This project's binary is called 
```bash
mcmodm
```
## Installation
In order to install this program, you should download a file from `releases` section.
Then, you have to install the program. Installation is different for both Windows and Linux users.
### Windows
#### Step 1
Install MSYS2, then use the MINGW64 shell **THAT IS WHAT YOU WILL BE RUNNING YOUR PROGRAM FROM!!!**
#### Step 2
Open the blue icon of MinGW (I believe it is MSYS2). Youw will be prompted by a shell. **DON'T WORRY ITS NOT SCARY!!!**
#### Step 3
run
```bash
 git clone https://github.com/MidnightPavlaka635920/Minecraft-ModM.git
```
#### Step 4
Run `./installwin.sh`
### Linux (Debian)
1. Download `.deb` file
2. Run
  ```bash
sudo dpkg-deb -i mcmodm.deb
```
3. Simply run `mcmodem <option>`
### Arch Linux
I've made AUR package called `mcmodm`. Use your favorite AUR helper to install it. Example:
```bash
yay -S mcmodm
```
## Setting up
1. `cd` into the minecraft directory. *For Windows users in MinGW64 shell, usr home directory is `/C/Users/Username/`*
2. To create a version file, run
```bash
mcmodm setup <path> <version> <loader> (loader)
```
Multiple loaders can be set
2. You are ready
## Usage
Commands:
- Search for mod/plugins: `mcmodm <mod/plugin name>`
- Then, use project ID and run `mcmodm install <Project ID> (Project ID ) <Install Path>`. Dependencies will be installed. Multiple packages could be installed.
- List installed plugins: `mcmodm list <Install path>`
- Remove mod/plugin: `mcmodm remove <Project ID> (Project ID ) <Install Path>`. Just specified package will be removed
- Update all packages: `mcmodm updateall <new Game version> <Install path>`
- Install packages easily (follow instructions that appear): `mcmodm easy_install <path>`
- Remove packages easily (follow instructiosn that appear): `mcmodm easy_remove <path>`
- Install all packages from a file: `mcmodm iff <path_to/packages.json> <install-path>` There, `path_to/packages.json` should point ot a file.
- Check if all of installed packages are available for updating to a specific version and loader: `mcmodm ck_upd <update_version> <loader> <installation_folder>`
- Install a local .jar file: `mcmodm il <file_to_install> <path_to_install - destination> <name>`
## Compiling yourself
There is a Makefile. Use `make` to compile on Linux/macOS, `sudo make install` to install system-wide. Source files are in src/ and header files are in include/
On Windows, steps are simmilar after installing dependencies, using a MINGW64 shell.
## Notes
In command above, `Install path` or `Path to req.json` is a directory and thus shouldn't contain `req.json`.
## How to use this program?
- Open the shell (**NOT** CMD for Windows, use MinGW as I said)
- If you wish, cd into the directory you will be using.
- Run desired command.
