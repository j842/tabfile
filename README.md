# tabfile

```
            __ __|       |      ____| _)  |
               |   _` |  __ \   |      |  |   _ \
               |  (   |  |   |  __|    |  |   __/
              _| \__,_| _.__/  _|     _| _| \___|




    tabfile DIRECTORY BASE_URL [OPTIONS]

    Options:
        -d
            run as daemon
```

tabfile is a small custom tool used to create unchanging symlinks that always take you to the latest version of a file resource. It's intended to be used behind a web server (e.g. nginx), to host the files via those symlinks.

Given the directory ```dir```, tabfile watches the tree ```dir/source``` for changes, and creates symlinks, qr codes and a directory index in ```dir/output```.

- The symlink name is the name of the parent folder of a file, together with the file's extension.
- Only the latest file in that folder is symlinked (so each folder reperesents one file, with multiple versions in the folder).
- Files must be named starting with an 8 digit date format: ```YYYMMDD```, otherwise will be ignored.
- The BASE_URL should be in the form ```https://HOSTNAME```, and is used in the index.
- The index is currently an Excel file.

Logs are written to ```/tmp/tabfile.log```. The ```-d``` option causes tabfile to run as a daemon.



## Production Setup
```
sudo apt install libasio-dev libspdlog1.20 libxlsxwriter5
```


## Development Setup

If using VS Code, set the cppStandard to c++17.

### General

```
sudo apt install zlib1g-dev libdeflate-dev cmake libssl-dev libasio-dev clang build-essential libfmt-dev libxlsxwriter-dev
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

