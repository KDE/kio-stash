#GSoC 2016 File Stash project for KDE

##Introduction

Selecting multiple files in any file manager for copying and pasting has never been a pleasant experience, especially if the files are in a non-continuous order. Often, when selecting files using Ctrl+A or the selection tool, we find that we need to select only a subset of the required files we have selected. This leads to the unwieldy operation of removing files from our selection. Of course, the common workaround is to create a new folder and to put all the items in this folder prior to copying, but this is a very inefficient and very slow process if large files need to be copied. Moreover Ctrl+Click requires fine motor skills to not lose the entire selection of files.

This is an original project with a novel solution to this problem. My solution is to add a virtual folder in Dolphin where the links to files and folders can be temporarily saved for a session. The files and folders are "staged" on this virtual folder. Files can be added to this by using a right-click context menu option, using the mouse scroll click, or by drag and drop. Hence, complex file operations such as moving files across many devices can be made easy by staging the operation before performing it.

##Installation

Make sure you have KF5 backports installed with all the [KDE dependencies](https://community.kde.org/Guidelines_and_HOWTOs/Build_from_source/Install_the_dependencies) installed before you build!

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
