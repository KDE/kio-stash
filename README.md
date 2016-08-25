# GSoC 2016 File Stash project for KDE

## Introduction

Selecting multiple files in any file manager for copying and pasting has never been a pleasant experience, especially if the files are in a non-continuous order. Often, when selecting files using Ctrl+A or the selection tool, we find that we need to select only a subset of the required files we have selected. This leads to the unwieldy operation of removing files from our selection. Of course, the common workaround is to create a new folder and to put all the items in this folder prior to copying, but this is a very inefficient and very slow process if large files need to be copied. Moreover Ctrl+Click requires fine motor skills to not lose the entire selection of files.

This is an original project with a novel solution to this problem. My solution is to add a virtual folder in all KIO applications, where the links to files and folders can be temporarily saved for a session. The files and folders are "staged" on this virtual folder. Files can be added to this by using all the regular file management operations such as Move, Copy and Paste, or by drag and drop. Hence, complex file operations such as moving files across many devices can be made easy by staging the operation before performing it.

## Project Overview

This project consists of the following modules. As there is no existing implementation in KIO for managing virtual directories, all the following modules were written completely from scratch.

### KIO Slave

The KIO slave is the backbone of the project. This KIO slave is responsible for interfacing with the GUI of a KDE application and provides the methods for various operations such as copying, deleting, and renaming files. All operations on the KIO slave are applied on a virtual stash filesystem (explained below). These operations are applied through inter process communication using the Qt's D-Bus API.

The advantage of the KIO slave is that it provides a consistent experience throughout the entire KDE suite of applications. Hence, this feature would work with all KIO compatible applications.

### Stash File System

The Stash File System (SFS) is used for virtually staging all the files and directories added to the ioslave. When a file is copied to the SFS, a new File Node is created to it under the folder to which it is copied. On copying a folder, a new Directory Node is created on the SFS with all the files and directories under it copied recursively as dictated by KIO. The SFS is a very important feature of the project as it allows the user to create folders and move items on the stash ioslave without touching the physical file system at all. Once a selection is curated on the ioslave, it can be seamlessly copied to the physical filesystem.

The SFS is implemented using a QHash pair of the URL as a key, containing the location of the file on the SFS and the value containing a StashNodeData object which contains all the properties (such as file name, source, children files for directories) of a given node in SFS.

Memory use of the SFS is nominal on a per file basis - each file staged on the SFS requires roughly 300 bytes of memory.

### Stash Daemon

The Stash File System runs in the KDE Daemon (kded5) container process. An object of the SFS is created on startup when the daemon is initialized. The daemon responds to calls from the ioslave communicated over the session bus and creates and removes nodes in the SFS.

## Installation

Make sure you have KF5 backports with all the [KDE dependency libraries](https://community.kde.org/Guidelines_and_HOWTOs/Build_from_source/Install_the_dependencies) installed before you build!

Clone master to your favorite folder and run:

```
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=/usr -DKDE_INSTALL_USE_QT_SYS_PATHS=TRUE ..
make
sudo make install
kdeinit5
```

Adaptor classes for D-Bus were generated using:

```
qdbuscpp2xml -a stashnotifier.h -o ../../dbus/org.kde.kio.StashNotifier.xml
qdbusxml2cpp -c StashNotifierAdaptor -a stash_adaptor.h:stash_adaptor.cpp ../../dbus/org.kde.kio.StashNotifier.xml
```
