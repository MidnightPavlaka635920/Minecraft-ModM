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
1. Download .zip (not the 'Source code' one) from `releases` section
2. Copy it into your home directory, it should work.
3. Run `cmd.exe` (`Win+R`, type `cmd`, press `Enter`), or easier, open terminal from the folder where `mcmodm.exe` is sotred. If you do so, skip step 4.
4. `cd` into your minecraft folder
5. Run `.\mcmodm.exe <options>`
6. You should downlaod CA certificate from <a href='https://curl.se/docs/caextract.html'>here</a> and copy them into the same folder as `mcmodm.exe`.
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
1. `cd` into the minecraft directory where you extracted the zip. *For Windows users in MinGW64 shell, user home directory is `/C/Users/Username/`*
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
- (OPTIONAL, BUT USEFUL) Add folder where `mcmodm.exe` is located to the PATH, there are online tutorials for that.
## How to use this program?
- Open the shell
- On windows, cd into the directory where the `mcmodm.exe` is. *It is recomended to intsll it into your home folder.*
- Run desired command.
