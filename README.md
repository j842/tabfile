# tabfile

```
tabfile [-d] DIRECTORY
```

Given the directory dir, tabfile watches the tree ```dir/source``` for changes, and creates symlinks in ```dir/output```.

- The symlink name is the name of the parent folder of a file.
- Only the latest file in that folder is symlinked (so each folder reperesents one file, with multiple versions in the folder).
- Files must be named starting with an 8 digit date format: ```YYYMMDD```, otherwise will be ignored.




## Production Setup
```
sudo apt install libasio-dev libspdlog1.10
```


## Development Setup

If using VS Code, set the cppStandard to c++17.

### General

```
sudo apt install zlib1g-dev libdeflate-dev cmake libssl-dev libasio-dev clang build-essential libfmt-dev
```

### Make

Install version 4.4.1 of Make (build from source) - required for .WAIT in parallel tasks. In a temp folder:
```
wget https://ftp.gnu.org/gnu/make/make-4.4.1.tar.gz
tar zxvf make-4.4.1.tar.gz
cd make-4.4.1/
./configure
make
sudo make install && sudo rm -f /usr/bin/make && sudo ln -s /usr/local/bin/make /usr/bin/make
make --version
```

### Spdlog
```
git clone https://github.com/gabime/spdlog.git
cd spdlog && mkdir build && cd build
cmake .. && make -j
sudo make install
```

