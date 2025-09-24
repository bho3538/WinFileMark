# WinFileMark
File mark and Folder custom message notification utility for Windows explorer.\
This program is designed to display critical personal files.\
And efficiently manage files on team shared file servers and to display important announcements in folders.

This project has been discontinued.

https://github.com/user-attachments/assets/5fa86a89-8322-4697-b375-b48de3212187

# How dows it work?
A shell extension (WinFilemarkExt.dll) will load explorer.exe and other process.\
It hooks some internal shell functions to show display icon columns at ListView.

Each shell extensions (WinFIlemarkExt.dll on some processes) communicate with main agent program (WinFilemarkExt.exe)
to receive file's mark status.

# License
MIT License
