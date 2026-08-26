/*
 *   SPDX-FileCopyrightText: 2016 Arnav Dhamija <arnav.dhamija@gmail.com>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "filestash.h"

#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusMetaType>
#include <QDBusReply>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QMetaType>
#include <QMimeDatabase>
#include <QMimeType>
#include <QUrl>
#include <unistd.h>

#include <KConfigGroup>
#include <KFileItem>
#include <KIO/Job>
#include <KIO/StatJob>
#include <KLocalizedString>

class KIOPluginForMetaData : public QObject
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.kio.worker.filestash" FILE "filestash.json")
};

extern "C" {
int Q_DECL_EXPORT kdemain(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    FileStash worker(argv[2], argv[3]);
    worker.dispatchLoop();
    return 0;
}
}

FileStash::FileStash(const QByteArray &pool, const QByteArray &app, const QString &daemonService, const QString &daemonPath)
    : KIO::ForwardingWorkerBase("stash", pool, app)
    , m_daemonService(daemonService)
    , m_daemonPath(daemonPath)
{
}

FileStash::~FileStash()
{
}

bool FileStash::rewriteUrl(const QUrl &url, QUrl &newUrl)
{
    if (url.scheme() != QLatin1String("stash")) {
        return false;
    }

    const QString fileInfo = setFileInfo(url);
    const FileStash::dirList item = createDirListItem(fileInfo);
    if (item.type == NodeType::InvalidNode || item.source.isEmpty()) {
        return false;
    }

    newUrl = QUrl::fromLocalFile(item.source);
    return true;
}

void FileStash::createTopLevelDirEntry(KIO::UDSEntry &entry)
{
    entry.clear();
    entry.fastInsert(KIO::UDSEntry::UDS_NAME, QStringLiteral("."));
    entry.fastInsert(KIO::UDSEntry::UDS_FILE_TYPE, 0040000);
    entry.fastInsert(KIO::UDSEntry::UDS_ACCESS, 0700);
    entry.fastInsert(KIO::UDSEntry::UDS_LOCAL_USER_ID, ::getuid());
    entry.fastInsert(KIO::UDSEntry::UDS_MIME_TYPE, QStringLiteral("inode/directory"));
}

QStringList FileStash::setFileList(const QUrl &url)
{
    QDBusMessage msg = QDBusMessage::createMethodCall(m_daemonService, m_daemonPath, "", "fileList");
    msg << url.path();
    QDBusReply<QStringList> received = QDBusConnection::sessionBus().call(msg);
    return received.value();
}

QString FileStash::setFileInfo(const QUrl &url)
{
    QDBusMessage msg = QDBusMessage::createMethodCall(m_daemonService, m_daemonPath, "", "fileInfo");
    msg << url.path();
    QDBusReply<QString> received = QDBusConnection::sessionBus().call(msg);
    return received.value();
}

KIO::WorkerResult FileStash::stat(const QUrl &url)
{
    KIO::UDSEntry entry;
    if (isRoot(url.path())) {
        createTopLevelDirEntry(entry);
    } else {
        QString fileInfo = setFileInfo(url);
        FileStash::dirList item = createDirListItem(fileInfo);
        if (!createUDSEntry(entry, item)) {
            return KIO::WorkerResult::fail(KIO::ERR_DOES_NOT_EXIST, url.toDisplayString());
        }
    }
    statEntry(entry);
    return KIO::WorkerResult::pass();
}

bool FileStash::statUrl(const QUrl &url, KIO::UDSEntry &entry)
{
    KIO::StatJob *statJob = KIO::stat(url, KIO::HideProgressInfo);
    bool ok = statJob->exec();
    if (ok) {
        entry = statJob->statResult();
    }
    return ok;
}

bool FileStash::createUDSEntry(KIO::UDSEntry &entry, const FileStash::dirList &fileItem)
{
    QMimeType fileMimetype;
    QMimeDatabase mimeDatabase;
    QString stringFilePath = fileItem.filePath;

    switch (fileItem.type) {
    case NodeType::DirectoryNode:
        entry.fastInsert(KIO::UDSEntry::UDS_FILE_TYPE, 0040000);
        entry.fastInsert(KIO::UDSEntry::UDS_ACCESS, 0700);
        entry.fastInsert(KIO::UDSEntry::UDS_LOCAL_USER_ID, ::getuid());
        entry.fastInsert(KIO::UDSEntry::UDS_MIME_TYPE, QString("inode/directory"));
        entry.fastInsert(KIO::UDSEntry::UDS_NAME, QUrl(stringFilePath).fileName());
        entry.fastInsert(KIO::UDSEntry::UDS_DISPLAY_NAME, QUrl(stringFilePath).fileName());
        break;
    case NodeType::InvalidNode:
        return false;
    default:
        QByteArray physicalPath_c = QFile::encodeName(fileItem.source);
        QT_STATBUF buff;
        QT_LSTAT(physicalPath_c, &buff);
        mode_t access = buff.st_mode & 07777;

        QFileInfo entryInfo;
        entryInfo = QFileInfo(fileItem.source);
        fileMimetype = mimeDatabase.mimeTypeForFile(fileItem.source);
        entry.fastInsert(KIO::UDSEntry::UDS_TARGET_URL, QUrl::fromLocalFile(fileItem.source).toString());
        entry.fastInsert(KIO::UDSEntry::UDS_MIME_TYPE, fileMimetype.name());
        entry.fastInsert(KIO::UDSEntry::UDS_LOCAL_USER_ID, ::getuid());
        entry.fastInsert(KIO::UDSEntry::UDS_DISPLAY_NAME, QUrl(stringFilePath).fileName());
        entry.fastInsert(KIO::UDSEntry::UDS_NAME, QUrl(stringFilePath).fileName());
        entry.fastInsert(KIO::UDSEntry::UDS_ACCESS, access);
        entry.fastInsert(KIO::UDSEntry::UDS_SIZE, entryInfo.size());
        entry.fastInsert(KIO::UDSEntry::UDS_MODIFICATION_TIME, buff.st_mtime);
        entry.fastInsert(KIO::UDSEntry::UDS_ACCESS_TIME, buff.st_atime);

        if (fileItem.type == NodeType::FileNode) {
            entry.fastInsert(KIO::UDSEntry::UDS_FILE_TYPE, 0100000);
        } else if (fileItem.type == NodeType::SymlinkNode) {
            entry.fastInsert(KIO::UDSEntry::UDS_FILE_TYPE, 0120000);
            entry.fastInsert(KIO::UDSEntry::UDS_LINK_DEST, entryInfo.symLinkTarget());
        } else {
            return false;
        }
    }
    return true;
}

FileStash::dirList FileStash::createDirListItem(const QString &fileInfo)
{
    FileStash::dirList item;
    QStringList strings = fileInfo.split("::", Qt::KeepEmptyParts);
    if (strings.size() < 3) {
        item.type = FileStash::NodeType::InvalidNode;
        return item;
    }
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

KIO::WorkerResult FileStash::listDir(const QUrl &url)
{
    QStringList fileList = setFileList(url);
    if (!fileList.isEmpty() && fileList.at(0) == "error::error::InvalidNode") {
        return KIO::WorkerResult::fail(KIO::ERR_DOES_NOT_EXIST, url.toDisplayString());
    }
    FileStash::dirList item;
    KIO::UDSEntry entry;
    if (isRoot(url.path())) {
        createTopLevelDirEntry(entry);
        listEntry(entry);
    }
    if (fileList.at(0) == "error::error::InvalidNode") {
        return KIO::WorkerResult::fail(KIO::ERR_DOES_NOT_EXIST, url.toDisplayString());
    }
    for (auto it = fileList.begin(); it != fileList.end(); ++it) {
        entry.clear();
        item = createDirListItem(*it);
        if (createUDSEntry(entry, item)) {
            listEntry(entry);
        } else {
            return KIO::WorkerResult::fail(KIO::ERR_WORKER_DEFINED, i18n("The UDS Entry could not be created."));
        }
    }
    return KIO::WorkerResult::pass();
}

KIO::WorkerResult FileStash::mkdir(const QUrl &url, int permissions)
{
    Q_UNUSED(permissions)

    QDBusMessage replyMessage;
    QDBusMessage msg;
    msg = QDBusMessage::createMethodCall(m_daemonService, m_daemonPath, "", "addPath");
    QString destinationPath = url.path();
    msg << "" << destinationPath << NodeType::DirectoryNode;
    replyMessage = QDBusConnection::sessionBus().call(msg);
    if (replyMessage.type() == QDBusMessage::ErrorMessage) {
        return KIO::WorkerResult::fail(KIO::ERR_WORKER_DEFINED, i18n("Could not create a directory"));
    }
    return KIO::WorkerResult::pass();
}

bool FileStash::copyFileToStash(const QUrl &src, const QUrl &dest, KIO::JobFlags flags)
{
    Q_UNUSED(flags)

    NodeType fileType;
    QFileInfo fileInfo = QFileInfo(src.path());
    if (fileInfo.isFile()) {
        fileType = NodeType::FileNode;
    } else if (fileInfo.isSymLink()) {
        fileType = NodeType::SymlinkNode;
    } else if (fileInfo.isDir()) { // if I'm not wrong, this can never happen, but we should handle it anyway
        fileType = NodeType::DirectoryNode;
    } else {
        return false;
    }

    QDBusMessage replyMessage;
    QDBusMessage msg;
    msg = QDBusMessage::createMethodCall(m_daemonService, m_daemonPath, "", "addPath");
    QString destinationPath = dest.path();

    msg << src.path() << destinationPath << (int)fileType;
    replyMessage = QDBusConnection::sessionBus().call(msg);
    if (replyMessage.type() != QDBusMessage::ErrorMessage) {
        return true;
    } else {
        return false;
    }
}

bool FileStash::copyStashToFile(const QUrl &src, const QUrl &dest, KIO::JobFlags flags)
{
    const QString destInfo = setFileInfo(src);
    const FileStash::dirList fileItem = createDirListItem(destInfo);

    if (fileItem.type != NodeType::DirectoryNode) {
        QByteArray physicalPath_c = QFile::encodeName(fileItem.source);
        QT_STATBUF buff;
        QT_LSTAT(physicalPath_c, &buff);
        mode_t access = buff.st_mode & 07777;
        const auto result = KIO::ForwardingWorkerBase::copy(QUrl::fromLocalFile(fileItem.source), dest, access, flags);
        return result.success();
    }
    return false;
}

bool FileStash::copyStashToStash(const QUrl &src, const QUrl &dest, KIO::JobFlags flags)
{
    Q_UNUSED(flags)

    KIO::UDSEntry entry;

    statUrl(src, entry);
    KFileItem fileItem(entry, src);

    const dirList item = createDirListItem(setFileInfo(src));
    NodeType fileType;
    if (fileItem.isFile()) {
        fileType = NodeType::FileNode;
    } else if (fileItem.isLink()) {
        fileType = NodeType::SymlinkNode;
    } else if (fileItem.isDir()) {
        fileType = NodeType::DirectoryNode;
    } else {
        return false;
    }

    QDBusMessage replyMessage;
    QDBusMessage msg;
    msg = QDBusMessage::createMethodCall(m_daemonService, m_daemonPath, "", "addPath");
    msg << item.source << dest.path() << fileType;
    replyMessage = QDBusConnection::sessionBus().call(msg);
    if (replyMessage.type() != QDBusMessage::ErrorMessage) {
        return true;
    } else {
        return false;
    }
}

KIO::WorkerResult FileStash::copy(const QUrl &src, const QUrl &dest, int permissions, KIO::JobFlags flags)
{
    KIO::UDSEntry entry;
    statUrl(src, entry);
    KFileItem item(entry, src);
    QUrl newDestPath;
    newDestPath = QUrl(dest.adjusted(QUrl::RemoveFilename).toString() + item.name());

    if (src.scheme() != "stash" && dest.scheme() == "stash") {
        if (copyFileToStash(src, newDestPath, flags)) {
            return KIO::WorkerResult::pass();
        }

        return KIO::WorkerResult::fail(KIO::ERR_WORKER_DEFINED, i18n("Could not copy."));
    }
    if (src.scheme() == "stash" && dest.scheme() != "stash") {
        if (copyStashToFile(src, newDestPath, flags)) {
            return KIO::WorkerResult::pass();
        }
        return KIO::WorkerResult::fail(KIO::ERR_WORKER_DEFINED, i18n("Could not copy."));
    }
    if (src.scheme() == "stash" && dest.scheme() == "stash") {
        if (copyStashToStash(src, newDestPath, flags)) {
            return KIO::WorkerResult::pass();
        }
        return KIO::WorkerResult::fail(KIO::ERR_WORKER_DEFINED, i18n("Could not copy."));
    }
    if (dest.scheme() == "mtp") {
        return KIO::WorkerResult::fail(KIO::ERR_WORKER_DEFINED, i18n("Copying to mtp workers is still under development!"));
    }
    return KIO::ForwardingWorkerBase::copy(item.targetUrl(), newDestPath, permissions, flags);
}

KIO::WorkerResult FileStash::del(const QUrl &url, bool isFile)
{
    Q_UNUSED(isFile)

    if (deletePath(url)) {
        return KIO::WorkerResult::pass();
    }
    return KIO::WorkerResult::fail(KIO::ERR_WORKER_DEFINED, QString("Could not reach the stash daemon"));
}

bool FileStash::deletePath(const QUrl &url)
{
    NodeType fileType;

    QDBusMessage replyMessage;
    QDBusMessage msg;
    msg = QDBusMessage::createMethodCall(m_daemonService, m_daemonPath, "", "removePath");

    if (isRoot(url.adjusted(QUrl::RemoveFilename).toString())) {
        msg << url.fileName();
    } else {
        msg << url.path();
    }

    replyMessage = QDBusConnection::sessionBus().call(msg);
    if (replyMessage.type() != QDBusMessage::ErrorMessage) {
        return true;
    } else {
        return false;
    }
}

KIO::WorkerResult FileStash::rename(const QUrl &src, const QUrl &dest, KIO::JobFlags flags)
{
    if (src.scheme() == "stash" && dest.scheme() == "stash") {
        if (copyStashToStash(src, dest, flags)) {
            if (deletePath(src)) {
                return KIO::WorkerResult::pass();
            }
        }
        return KIO::WorkerResult::fail(KIO::ERR_WORKER_DEFINED, i18n("Could not rename."));
    }
    if (src.scheme() == "file" && dest.scheme() == "stash") {
        if (copyFileToStash(src, dest, flags)) {
            return KIO::WorkerResult::pass();
        }
        return KIO::WorkerResult::fail(KIO::ERR_WORKER_DEFINED, i18n("Could not rename."));
    }
    if (src.scheme() == "stash" && dest.scheme() == "file") {
        if (copyStashToFile(src, dest, flags)) {
            if (deletePath(src)) {
                return KIO::WorkerResult::pass();
            }
        }
        return KIO::WorkerResult::fail(KIO::ERR_WORKER_DEFINED, i18n("Could not rename."));
    }
    return KIO::WorkerResult::fail();
}

bool FileStash::isRoot(const QString &string)
{
    if (string.isEmpty() || string == "/") {
        return true;
    }
    return false;
}

#include "filestash.moc"
