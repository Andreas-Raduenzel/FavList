# FavList
<img width="382" height="512" alt="FavList" src="https://github.com/user-attachments/assets/5efaeca6-ce77-41e3-9580-dcf843e616d4" />


**FavList** is a lightweight favorites launcher for **KDE Plasma 6**.  
It provides quick access to files and folders directly from the system tray.

The focus of FavList is clarity, simplicity, and native KDE integration — without external scripts or background services.

---

## Features

- System tray favorites launcher
- Supports files and folders
- Drag & drop reordering
- Native Qt 6 / KDE Plasma 6 application
- Icons included in the application package
- No Python, no Bash scripts, no background daemons

---

## Requirements

- KDE Plasma 6
- Qt 6
- CMake
- Extra CMake Modules (ECM)
- C++17 compatible compiler

---

## Build

```bash
mkdir build
cd build
cmake ..
make
