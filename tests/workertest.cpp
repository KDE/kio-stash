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

#include "workertest.h"

#include <QTest>
#include <QDir>
#include <QFile>
#include <QTemporaryFile>
#include <QFileInfo>
#include <QDBusMessage>
#include <QDBusConnection>
#include <QStandardPaths>
#include <QDebug>

#include <KIO/Job>
#include <KIO/CopyJob>
#include <KIO/DeleteJob>
#include <KFileItem>

WorkerTest::WorkerTest() : tmpFolder("WorkerTest"),
    m_fileTestFile("TestFile"),
    m_fileTestFolder("TestSubFolder"),
    m_stashTestFolder("StashTestFolder"),
    m_stashTestSymlink("StashTestSymlink"),
    m_stashTestFile("StashFile"),
    m_stashTestFileInSubDirectory("SubTestFile"),
    m_newStashFileName("NewStashFile"),
    m_stashFileForRename("StashRenameFile"),
    m_absolutePath(QDir::currentPath())
{}

void WorkerTest::initTestCase()
{
    //enclose a check statement around this block to see if kded5 is not already there
    QDBusMessage msg;
    QDBusMessage replyMessage;

    stashDaemonProcess = new QProcess();

    msg = QDBusMessage::createMethodCall(
              "org.kde.kio.StashNotifier", "/StashNotifier", "", "pingDaemon");
    replyMessage = QDBusConnection::sessionBus().call(msg);
    if (replyMessage.type() == QDBusMessage::ErrorMessage) {
        qDebug() << "Launching fallback daemon";
        const QString program = "./testdaemon";
        stashDaemonProcess->start(program, QStringList{});
    }

    replyMessage = QDBusConnection::sessionBus().call(msg);

    if (replyMessage.type() != QDBusMessage::ErrorMessage) {
        qDebug() << "Test case initialised";
    } else {
        qDebug() << "Something is wrong!";
    }
    createTestFiles();
}

void WorkerTest::createTestFiles() //also find a way to reset the directory prior to use
{
    QDir tmpDir;
    tmpDir.mkdir(tmpDirPath()); //creates test dir
    tmpDir.mkdir(tmpDirPath() + m_fileTestFolder);

    QFile tmpFile;
    stashDirectory('/' + m_stashTestFolder);

    QUrl src = QUrl::fromLocalFile(tmpDirPath() + m_fileTestFile); //creates a file to be tested
    tmpFile.setFileName(src.path());
    QVERIFY(tmpFile.open(QIODevice::ReadWrite));
    tmpFile.close();

    src = QUrl::fromLocalFile(tmpDirPath() + m_stashTestFile); //creates a file to be stashed
    tmpFile.setFileName(src.path());
    QVERIFY(tmpFile.open(QIODevice::ReadWrite));
    tmpFile.close();
    stashFile(src.path(), '/' + m_stashTestFile);

    src = QUrl::fromLocalFile(tmpDirPath() + m_stashFileForRename);
    tmpFile.setFileName(src.path());
    QVERIFY(tmpFile.open(QIODevice::ReadWrite));
    tmpFile.close();
    stashFile(src.path(), '/' + m_stashFileForRename);

    src = QUrl::fromLocalFile(tmpDirPath() + m_stashTestFileInSubDirectory);
    tmpFile.setFileName(src.path());
    QVERIFY(tmpFile.open(QIODevice::ReadWrite));
    tmpFile.close();

    stashFile(src.path(), '/' + m_stashTestFolder + '/' + m_stashTestFileInSubDirectory);
}

void WorkerTest::cleanupTestCase()
{
    QDir dir(tmpDirPath());
    dir.removeRecursively();
    stashDaemonProcess->terminate();
}

QString WorkerTest::tmpDirPath()
{
    return m_absolutePath + '/' + tmpFolder + '/';
}

void WorkerTest::statItem(const QUrl &url, const int &type)
{
    KIO::UDSEntry entry;
    QVERIFY(statUrl(url, entry));
    KFileItem item(entry, url);
    switch (type) {
    case NodeType::DirectoryNode:
        QVERIFY(item.isDir());
        break;
    case NodeType::SymlinkNode:
        QVERIFY(item.isLink()); //don't worry this is intentional :)
    case NodeType::FileNode:
        QVERIFY(item.isFile());
    }
    QVERIFY(item.isReadable());
    QVERIFY(!item.isHidden());
    QCOMPARE(item.text(), url.fileName());
}

void WorkerTest::stashFile(const QString &realPath, const QString &stashPath)
{
    QDBusMessage msg = QDBusMessage::createMethodCall(
                           "org.kde.kio.StashNotifier", "/StashNotifier", "", "addPath");
    msg << realPath << stashPath << NodeType::FileNode;
    bool queued = QDBusConnection::sessionBus().send(msg);
    QVERIFY(queued);
}

void WorkerTest::stashDirectory(const QString &path)
{
    QDBusMessage msg = QDBusMessage::createMethodCall(
                           "org.kde.kio.StashNotifier", "/StashNotifier", "", "addPath");
    QString destinationPath = path;
    msg << "" << destinationPath << NodeType::DirectoryNode;
    bool queued = QDBusConnection::sessionBus().send(msg);
    QVERIFY(queued);
}

void WorkerTest::stashSymlink(const QString &realPath, const QString &stashPath)
{
    QDBusMessage msg = QDBusMessage::createMethodCall(
                           "org.kde.kio.StashNotifier", "/StashNotifier", "", "addPath");
    msg << realPath << stashPath << NodeType::SymlinkNode;
    bool queued = QDBusConnection::sessionBus().send(msg);
    QVERIFY(queued);
}

bool WorkerTest::statUrl(const QUrl &url, KIO::UDSEntry &entry)
{
    KIO::StatJob *statJob = KIO::stat(url, KIO::HideProgressInfo);
    bool ok = statJob->exec();
    if (ok) {
        entry = statJob->statResult();
    }
    return ok;
}

void WorkerTest::nukeStash()
{
    QDBusMessage msg = QDBusMessage::createMethodCall(
                           "org.kde.kio.StashNotifier", "/StashNotifier", "", "nukeStash");
    bool queued = QDBusConnection::sessionBus().send(msg);
    QVERIFY(queued);
}

void WorkerTest::stashCopy(const QUrl &src, const QUrl &dest)
{
    KIO::CopyJob *job = KIO::copy(src, dest, KIO::HideProgressInfo);
    bool ok = job->exec();
    QVERIFY(ok);
}

void WorkerTest::moveFromStash(const QUrl &src, const QUrl &dest) //make this work
{
    KIO::Job *job = KIO::move(src, dest, KIO::HideProgressInfo);
    bool ok = job->exec();
    QVERIFY(ok);
}

void WorkerTest::deleteFromStash(const QUrl &url)
{
    KIO::Job *delJob = KIO::del(url, KIO::HideProgressInfo);
    bool ok = delJob->exec();
    QVERIFY(ok);
}

void WorkerTest::listRootDir()
{
    KIO::ListJob *job = KIO::listDir(QUrl(QStringLiteral("stash:/")), KIO::HideProgressInfo);
    connect(job, SIGNAL(entries(KIO::Job*, KIO::UDSEntryList)),
            SLOT(slotEntries(KIO::Job*, KIO::UDSEntryList)));
    bool ok = job->exec();
    QVERIFY(ok);
}

void WorkerTest::listSubDir()
{
    KIO::ListJob *job = KIO::listDir(QUrl("stash:/" + m_stashTestFolder), KIO::HideProgressInfo);
    connect(job, SIGNAL(entries(KIO::Job*, KIO::UDSEntryList)),
            SLOT(slotEntries(KIO::Job*, KIO::UDSEntryList)));
    bool ok = job->exec();
    QVERIFY(ok);
}

void WorkerTest::createDirectory()
{
    QUrl directoryPath = QUrl("");
    KIO::SimpleJob *job = KIO::mkdir(directoryPath);
    bool ok = job->exec();
    QVERIFY(QFile::exists(directoryPath.path()));
    QCOMPARE(ok, true);
}

void WorkerTest::statRoot()
{
    QUrl url("stash:/");
    KIO::UDSEntry entry;
    QVERIFY(statUrl(url, entry));
    KFileItem item(entry, url);
    QVERIFY(item.isDir());
    QVERIFY(!item.isLink());
    QVERIFY(item.isReadable());
    QVERIFY(item.isWritable());
    QVERIFY(!item.isHidden());
    QCOMPARE(item.name(), QStringLiteral("."));
}

void WorkerTest::statFileInRoot()
{
    QFile file;
    QUrl url("stash:/" + m_stashTestFile);
    stashFile(url.path(), url.path());
    KIO::UDSEntry entry;
    QVERIFY(statUrl(url, entry));
    KFileItem item(entry, url);
    QVERIFY(item.isFile());
    QVERIFY(!item.isDir());
    QVERIFY(!item.isLink());
    QVERIFY(item.isReadable());
    QVERIFY(!item.isHidden());
}

void WorkerTest::statDirectoryInRoot()
{
    QUrl url("stash:/" + m_stashTestFolder);
    stashDirectory(url.path());
    KIO::UDSEntry entry;
    QVERIFY(statUrl(url, entry));
    KFileItem item(entry, url);
    QVERIFY(!item.isFile());
    QVERIFY(item.isDir());
    QVERIFY(!item.isLink());
    QVERIFY(item.isReadable());
    QVERIFY(!item.isHidden());
}

void WorkerTest::statSymlinkInRoot()
{
    QUrl url("stash:/" + m_stashTestSymlink);
    stashSymlink(url.path(), url.path());
    KIO::UDSEntry entry;
    QVERIFY(statUrl(url, entry));
    KFileItem item(entry, url);
    QVERIFY(item.isFile());
    QVERIFY(!item.isDir());
    QVERIFY(item.isReadable());
    QVERIFY(!item.isHidden());
}

void WorkerTest::statFileInDirectory()
{
    QUrl url("stash:/" + m_stashTestFolder + '/' + m_stashTestFileInSubDirectory);
    KIO::UDSEntry entry;
    QVERIFY(statUrl(url, entry));
    KFileItem item(entry, url);
    QVERIFY(item.isFile());
    QVERIFY(!item.isDir());
    QVERIFY(!item.isLink());
    QVERIFY(item.isReadable());
    QVERIFY(!item.isHidden());
}

void WorkerTest::copyFileToStash()
{
    QUrl src = QUrl::fromLocalFile(tmpDirPath() + m_fileTestFile);
    QFile testFile(src.path());
    QVERIFY(testFile.open(QIODevice::WriteOnly));
    QUrl dest("stash:/" + m_fileTestFile);

    stashCopy(src, dest);
    QVERIFY(testFile.exists());
    statItem(dest, NodeType::FileNode);
}

void WorkerTest::copySymlinkFromStashToFile() //create test case
{
    stashSymlink(tmpDirPath() + m_fileTestFile, '/' + m_stashTestSymlink);
    QUrl src("stash:/" + m_stashTestSymlink);
    QUrl dest = QUrl::fromLocalFile(tmpDirPath() + m_fileTestFolder + '/' + m_stashTestSymlink);

    stashCopy(src, dest);
    QVERIFY(QFile::exists(dest.path()));
}

void WorkerTest::copyStashToFile()
{
    QUrl src("stash:/" + m_stashTestFile);
    QUrl dest = QUrl::fromLocalFile(tmpDirPath() + m_fileTestFolder + '/' + m_stashTestFile);
    KIO::UDSEntry entry;
    stashCopy(src, dest);
    QVERIFY(QFile::exists(dest.toLocalFile()));
    QFile(dest.path()).remove();
}

void WorkerTest::moveToFileFromStash()
{
    QUrl src("stash:/" + m_stashTestFile);
    QUrl dest = QUrl::fromLocalFile(tmpDirPath() + m_fileTestFolder + '/' + m_stashTestFile);

    moveFromStash(src, dest);
    KIO::UDSEntry entry;
    statUrl(src, entry);
    KFileItem item(entry, src);
    QVERIFY(item.name() != m_stashTestFile);
    QVERIFY(QFile::exists(dest.toLocalFile()));
}

void WorkerTest::copyStashToStash()
{
    QUrl src("stash:/" + m_stashTestFile);
    QUrl dest("stash:/" + m_stashTestFolder + '/' + m_stashTestFile);
    stashCopy(src, dest);
    KIO::UDSEntry entry;
    statUrl(src, entry);
    KFileItem item(entry, src);
    QVERIFY(item.name() == m_stashTestFile);
}

void WorkerTest::renameFileInStash()
{
    QUrl src("stash:/" + m_stashFileForRename);
    QUrl dest("stash:/" + m_newStashFileName);

    KIO::UDSEntry entry;

    moveFromStash(src, dest);

    statUrl(src, entry);
    KFileItem item(entry, src);
    QVERIFY(item.name() != m_stashFileForRename);

    statUrl(dest, entry);
    item = KFileItem(entry, src);
    QVERIFY(item.name() == m_newStashFileName);
}


void WorkerTest::delRootFile()
{
    QUrl url("stash:/" + m_stashTestFile);
    deleteFromStash(url);
    KIO::UDSEntry entry;
    QVERIFY(statUrl(url, entry));
    KFileItem item(entry, url);
    QVERIFY(item.name() != m_stashTestFile);
}

void WorkerTest::delFileInDirectory()
{
    QUrl url("stash:/" + m_stashTestFolder + '/' + m_stashTestFileInSubDirectory);
    deleteFromStash(url);
    KIO::UDSEntry entry;
    QVERIFY(statUrl(url, entry));
    KFileItem item(entry, url);
    QVERIFY(item.name() != m_stashTestFileInSubDirectory);
}

void WorkerTest::delDirectory()
{
    QUrl url("stash:/" + m_stashTestFolder);
    deleteFromStash(url);
    KIO::UDSEntry entry;
    QVERIFY(statUrl(url, entry));
    KFileItem item(entry, url);
    QVERIFY(item.name() != m_stashTestFolder);
}

void WorkerTest::init()
{
    createTestFiles();
}

void WorkerTest::cleanup()
{
    QDir dir(tmpDirPath());
    dir.removeRecursively();
}

QTEST_MAIN(WorkerTest)
