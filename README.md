# FavList
<img width="780" height="496" alt="FavList_2" src="https://github.com/user-attachments/assets/36cdb6b8-b188-47b0-b673-e00dc27c0803" />



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
