## Trash
# trash — Simple Linux Trash Utility

A lightweight command-line tool that moves files and directories to the user's Trash folder (`~/.local/share/Trash`) following the Freedesktop.org Trash specification.

Similar to `gio trash`, `trash-put` or `rmtrash`.

### Features
- Moves files and folders to trash
- Creates proper `.trashinfo` metadata (original path + deletion date)
- Handles name conflicts by appending _1, _2, etc.
- Supports both relative and absolute paths

### Installation
```bash
g++ -std=c++17 -Wall trash.cpp -o trash
# For older GCC versions:
g++ -std=c++17 -Wall trash.cpp -lstdc++fs -o trash

sudo cp trash /usr/local/bin/
