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

#include "ioslave.h"

#include <QDebug>
#include <QMimeType>
#include <QMimeDatabase>
#include <QDateTime>
#include <QDir>
#include <QMetaType>
#include <QDBusMetaType>
#include <QUrl>
#include <QFile>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusReply>
#include <QCoreApplication>

#include <KLocalizedString>
#include <KConfigGroup>

class KIOPluginForMetaData : public QObject
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.kio.slave.filestash" FILE "ioslave.json")
};

extern "C" {
    int Q_DECL_EXPORT kdemain(int argc, char **argv)
    {
        QCoreApplication app(argc, argv);
        FileStash slave(argv[2], argv[3]);
        slave.dispatchLoop();
        return 0;
    }
}

FileStash::FileStash(const QByteArray &pool, const QByteArray &app) :
    KIO::SlaveBase("stash", pool, app)
{}

FileStash::~FileStash()
{}

void FileStash::createTopLevelDirEntry(KIO::UDSEntry &entry)
{
    entry.clear();
    entry.insert(KIO::UDSEntry::UDS_NAME, QStringLiteral("."));
    entry.insert(KIO::UDSEntry::UDS_FILE_TYPE, 0040000);
    entry.insert(KIO::UDSEntry::UDS_ACCESS, 0700);
    entry.insert(KIO::UDSEntry::UDS_MIME_TYPE, QStringLiteral("inode/directory"));
}

QStringList FileStash::setFileList(const QUrl &url)
{
    QDBusMessage msg = QDBusMessage::createMethodCall(
        "org.kde.kio.StashNotifier", "/StashNotifier", "", "fileList");
    msg << url.path();
    QDBusReply<QStringList> received = QDBusConnection::sessionBus().call(msg);
    return received.value();
}

QString FileStash::setFileInfo(const QUrl &url)
{
    QDBusMessage msg = QDBusMessage::createMethodCall(
        "org.kde.kio.StashNotifier", "/StashNotifier", "", "fileInfo");
    msg << url.path();
    QDBusReply<QString> received = QDBusConnection::sessionBus().call(msg);
    return received.value();
}

void FileStash::stat(const QUrl &url)
{
    qDebug() << "statcalled" << url;
    KIO::UDSEntry entry;
    if (isRoot(url.path())) {
        createTopLevelDirEntry(entry);
    } else {
        QString fileInfo = setFileInfo(url.path());
        FileStash::dirList item = createDirListItem(fileInfo);
        createUDSEntry(entry, item);
    }
    statEntry(entry);
    finished();
}

bool FileStash::createUDSEntry(KIO::UDSEntry &entry, const FileStash::dirList &fileItem)
{
    QDateTime epoch;
    epoch.setMSecsSinceEpoch(0);
    QMimeType fileMimetype;
    QMimeDatabase mimeDatabase;
    QString stringFilePath = fileItem.filePath;

    switch (fileItem.type) {
        case NodeType::DirectoryNode: { // TODO: add logic for number of children by modifying SFS
            entry.insert(KIO::UDSEntry::UDS_FILE_TYPE, 0040000);
            entry.insert(KIO::UDSEntry::UDS_MIME_TYPE, QString("inode/directory"));
            entry.insert(KIO::UDSEntry::UDS_NAME, QUrl(stringFilePath).fileName());
            entry.insert(KIO::UDSEntry::UDS_DISPLAY_NAME, QUrl(stringFilePath).fileName());
            break;
        }
        case NodeType::SymlinkNode: {// TODO: make more elegant
            QByteArray physicalPath_c = QFile::encodeName(fileItem.source);
            QT_STATBUF buff;

            if (QT_LSTAT(physicalPath_c, &buff) == -1) {
                return false;
            }

            QFileInfo entryInfo;
            entryInfo = QFileInfo(fileItem.source);
            fileMimetype = mimeDatabase.mimeTypeForFile(fileItem.source);
            entry.insert(KIO::UDSEntry::UDS_FILE_TYPE, 0120000);
            entry.insert(KIO::UDSEntry::UDS_MIME_TYPE, fileMimetype.name());
            entry.insert(KIO::UDSEntry::UDS_DISPLAY_NAME, QUrl(stringFilePath).fileName());
            entry.insert(KIO::UDSEntry::UDS_NAME, QUrl(stringFilePath).fileName());
            entry.insert(KIO::UDSEntry::UDS_TARGET_URL, QUrl::fromLocalFile(fileItem.source).toString());
            entry.insert(KIO::UDSEntry::UDS_ACCESS, entryInfo.permissions());
            entry.insert(KIO::UDSEntry::UDS_SIZE, entryInfo.size());
            entry.insert(KIO::UDSEntry::UDS_MODIFICATION_TIME, buff.st_mtime);
            entry.insert(KIO::UDSEntry::UDS_ACCESS_TIME, buff.st_atime);
            break;
        }
        case NodeType::FileNode: {
            QByteArray physicalPath_c = QFile::encodeName(fileItem.source);
            QT_STATBUF buff;

            if (QT_LSTAT(physicalPath_c, &buff) == -1) {
                return false;
            }

            QFileInfo entryInfo;
            entryInfo = QFileInfo(fileItem.source);
            fileMimetype = mimeDatabase.mimeTypeForFile(fileItem.source);
            entry.insert(KIO::UDSEntry::UDS_FILE_TYPE, 0100000);
            entry.insert(KIO::UDSEntry::UDS_MIME_TYPE, fileMimetype.name());
            entry.insert(KIO::UDSEntry::UDS_DISPLAY_NAME, QUrl(stringFilePath).fileName());
            entry.insert(KIO::UDSEntry::UDS_NAME, QUrl(stringFilePath).fileName());
            entry.insert(KIO::UDSEntry::UDS_TARGET_URL, QUrl::fromLocalFile(fileItem.source).toString());
            entry.insert(KIO::UDSEntry::UDS_ACCESS, entryInfo.permissions());
            entry.insert(KIO::UDSEntry::UDS_SIZE, entryInfo.size());
            entry.insert(KIO::UDSEntry::UDS_MODIFICATION_TIME, buff.st_mtime);
            entry.insert(KIO::UDSEntry::UDS_ACCESS_TIME, buff.st_atime);
            break;
        }
        case NodeType::InvalidNode: {
            entry.insert(KIO::UDSEntry::UDS_NAME, fileItem.filePath); // TODO: find a generic mimetype for broken files
        }
    }
    return true;
}

FileStash::dirList FileStash::createDirListItem(QString fileInfo)
{
    QStringList strings = fileInfo.split("::", QString::KeepEmptyParts);
    FileStash::dirList item;
    if (strings.at(0) == "dir") {
        item.type = FileStash::NodeType::DirectoryNode;
    } else if (strings.at(0) == "file") {
        item.type = FileStash::NodeType::FileNode;
    } else if (strings.at(0) == "symlink") {
        item.type = FileStash::NodeType::SymlinkNode;
    } else if (strings.at(0) == "invalid") {
        item.type = FileStash::NodeType::InvalidNode;
    }
    item.filePath = strings.at(1);
    item.source = strings.at(2);
    return item;
}

void FileStash::listDir(const QUrl &url) // FIXME: remove debug statements
{
    currentDir = url.path();
    QStringList fileList = setFileList(url);

    if (!fileList.size()) {
        qDebug() << "empty dir";
        finished();
        return;
    }

    FileStash::dirList item;
    KIO::UDSEntry entry;

    if (isRoot(url.path())) {
        createTopLevelDirEntry(entry);
        listEntry(entry);
    }

    if (fileList.at(0) == "error::error::InvalidNode") {
        error(KIO::ERR_SLAVE_DEFINED, QString("The file either does not exist or has not been stashed yet."));
    } else {
        for (auto it = fileList.begin(); it != fileList.end(); it++) {
            entry.clear();
            item = createDirListItem(*it);
            createUDSEntry(entry, item);
            listEntry(entry);
        }
        finished();
    }
}

void FileStash::mkdir(const QUrl &url, int permissions)
{
    qDebug() << "mkdirOut" << url;
    QDBusMessage msg = QDBusMessage::createMethodCall(
        "org.kde.kio.StashNotifier", "/StashNotifier", "", "addPath");
    QString destinationPath = url.path();
    qDebug() << "" << destinationPath << NodeType::DirectoryNode;
    msg << "" << destinationPath << NodeType::DirectoryNode;
    bool queued = QDBusConnection::sessionBus().send(msg);
    finished();
}

void FileStash::copy(const QUrl &src, const QUrl &dest, int permissions, KIO::JobFlags flags)
{
    qDebug() << "COPY CALLED";
    NodeType fileType;
    QFileInfo fileInfo = QFileInfo(src.path());
    if (fileInfo.isFile()) {
        fileType = NodeType::FileNode;
    } else if (fileInfo.isSymLink()) {
        fileType = NodeType::SymlinkNode;
    } else if (fileInfo.isDir()) { // if I'm not wrong, this can never happen, but we should handle it anyway
        fileType = NodeType::DirectoryNode;
        qDebug() << "DirectoryNode...created?";
    } else {
        error(KIO::ERR_SLAVE_DEFINED, QString("Could not determine file type."));
    }
    QDBusMessage msg = QDBusMessage::createMethodCall(
        "org.kde.kio.StashNotifier", "/StashNotifier", "", "addPath");
    QString destinationPath = dest.path();
    qDebug() << src.path() << destinationPath << (int) fileType;
    msg << src.path() << destinationPath << (int) fileType;
    bool queued = QDBusConnection::sessionBus().send(msg);
    if (queued) {
        finished();
    } else {
        error(KIO::ERR_SLAVE_DEFINED, QString("Cannot reach the stash daemon."));
    }
}

void FileStash::del(const QUrl &url, bool isFile)
{
    qDebug() << "Del request CALLLED" << url.path() << "for dir" << currentDir;
    QDBusMessage msg = QDBusMessage::createMethodCall(
        "org.kde.kio.StashNotifier", "/StashNotifier", "", "removePath");
    if (isRoot(currentDir)) {
        msg << url.fileName();
    } else {
        msg << url.path();
    }
    bool queued = QDBusConnection::sessionBus().send(msg);
    if (queued) {
        finished();
    } else {
        error(KIO::ERR_CANNOT_DELETE, url.path());
    }
}

bool FileStash::isRoot(const QString &string)
{
    if (string.isEmpty() || string == "/") {
        return true;
    }
    return false;
}

#include "ioslave.moc"
