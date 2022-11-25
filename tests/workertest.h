/***************************************************************************
 *   Copyright (C) 2016 by Arnav Dhamija <arnav.dhamija@gmail.com>         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA            *
 ***************************************************************************/

#ifndef WORKERTEST_H
#define WORKERTEST_H

#include <QObject>
#include <KIO/Job>
#include <QProcess>
#include <QString>

class WorkerTest : public QObject
{
    Q_OBJECT

public:
    WorkerTest();
    ~WorkerTest() {}

    enum NodeType {
        DirectoryNode,
        SymlinkNode,
        FileNode,
        InvalidNode
    };

private:
    QString tmpDirPath();
    bool statUrl(const QUrl &url, KIO::UDSEntry &entry);
    void stashFile(const QString &realPath, const QString &stashPath);
    void stashSymlink(const QString &realPath, const QString &stashPath);
    void stashDirectory(const QString &path);
    void stashCopy(const QUrl &src, const QUrl &dest);
    void statItem(const QUrl &url, const int &type);
    void createDirectory();
    void moveFromStash(const QUrl &src, const QUrl &dest);
    void deleteFromStash(const QUrl &url);
    void createTestFiles();
    void nukeStash(); //deletes every folder and file in stash:/

    QProcess *stashDaemonProcess;

    const QString tmpFolder;
    const QString m_fileTestFile;
    const QString m_fileTestFolder;
    const QString m_stashTestFolder;
    const QString m_stashTestSymlink;
    const QString m_stashTestFile;
    const QString m_stashTestFileInSubDirectory;
    const QString m_newStashFileName;
    const QString m_stashFileForRename;
    const QString m_absolutePath;

private slots:
    void initTestCase();
    void init();
    void copyFileToStash();
    void copyStashToFile();
    void copyStashToStash();
    void copySymlinkFromStashToFile();

    void moveToFileFromStash();

    void renameFileInStash();
    void statRoot();
    void statFileInRoot();
    void statDirectoryInRoot();
    void statSymlinkInRoot();
    void statFileInDirectory();

    void listRootDir();
    void listSubDir();

    void delRootFile();
    void delFileInDirectory();
    void delDirectory();

    void cleanup();
    void cleanupTestCase();
};

#endif
