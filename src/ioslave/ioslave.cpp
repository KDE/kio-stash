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

Q_DECLARE_METATYPE(dirListDBus::dirList)

QDBusArgument &operator<<(QDBusArgument &argument, const dirListDBus::dirList &object)
{
    argument.beginStructure();
    argument << object.filePath << object.source << object.type;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, dirListDBus::dirList &object)
{
    argument.beginStructure();
    argument >> object.filePath >> object.source >> object.type;
    argument.endStructure();
    return argument;
}

void FileStash::registerMetaType()
{
    qRegisterMetaType<dirListDBus::dirList>("dirList");
    qDBusRegisterMetaType<dirListDBus::dirList>();
}

FileStash::FileStash(const QByteArray &pool, const QByteArray &app) :
    KIO::ForwardingSlaveBase("stash", pool, app)
{}

FileStash::~FileStash()
{}

QList<dirListDBus::dirList> FileStash::setFileList(const QUrl &url)
{
    QDBusMessage msg = QDBusMessage::createMethodCall(
        "org.kde.kio.StashNotifier", "/StashNotifier", "", "fileList");
    msg << url.path();
    QDBusReply<QList<dirListDBus::dirList>> received = QDBusConnection::sessionBus().call(msg);
    return received.value();
}

bool FileStash::rewriteUrl(const QUrl &url, QUrl &newUrl)
{
    if (url.scheme() != "file") {
        newUrl.setScheme("file");
        newUrl.setPath(url.path());
    } else {
        newUrl = url;
    }
    return true;
}

void FileStash::listRoot()
{

}

bool FileStash::createUDSEntry(
    KIO::UDSEntry &entry, const QString &physicalPath,
    const QString &displayFileName, const QString &internalFileName)
{
    QByteArray physicalPath_c = QFile::encodeName(physicalPath);

    QFileInfo entryInfo = physicalPath;
    QDateTime epoch;

    epoch.setMSecsSinceEpoch(0);

    if (!entryInfo.exists()) {
        error(KIO::ERR_COULD_NOT_READ, physicalPath);
        QDBusMessage msg = QDBusMessage::createMethodCall(
            "org.kde.kio.StashNotifier", "/StashNotifier", "", "removePath");
        msg << physicalPath;
        bool queued = QDBusConnection::sessionBus().send(msg); // remove it from the kded
        return false;
    }

    if (entryInfo.isSymLink()) {
        entry.insert(KIO::UDSEntry::UDS_LINK_DEST, entryInfo.symLinkTarget());
    }

    Q_ASSERT(!internalFileName.isEmpty());
    qDebug() << "physicalPath " << physicalPath;

    QMimeDatabase db; // required for finding the mimeType
    QMimeType mt = db.mimeTypeForFile(physicalPath);
    if (mt.isValid()) {
        entry.insert(KIO::UDSEntry::UDS_MIME_TYPE, mt.name());
    }

    entry.insert(KIO::UDSEntry::UDS_NAME, physicalPath);
    entry.insert(KIO::UDSEntry::UDS_DISPLAY_NAME, displayFileName);

    if (entryInfo.isDir()) { // shittyprogramming, but it works. For now.
        entry.insert(KIO::UDSEntry::UDS_FILE_TYPE, 0040000);
    } else if (entryInfo.isFile()) {
        entry.insert(KIO::UDSEntry::UDS_FILE_TYPE, 0100000);
    } else if (entryInfo.isSymLink()) {
        entry.insert(KIO::UDSEntry::UDS_FILE_TYPE, 0120000);
    }

    entry.insert(KIO::UDSEntry::UDS_ACCESS, entryInfo.permissions());
    entry.insert(KIO::UDSEntry::UDS_SIZE, entryInfo.size());
    entry.insert(KIO::UDSEntry::UDS_MODIFICATION_TIME, QString::number(epoch.secsTo(entryInfo.lastModified()))); // FIXME: Broken af
    entry.insert(KIO::UDSEntry::UDS_ACCESS_TIME, QString::number(epoch.secsTo(entryInfo.lastRead())));

    return true;
}

void FileStash::listDir(const QUrl &url) // FIXME: remove debug statements
{
    auto fileList = setFileList(url);
    KIO::UDSEntry entry;
    for (auto it = fileList.begin(); it != fileList.end(); it++) {
        //createUDSEntry(entry, it->source, it->)
    }
}

void FileStash::displayList(const QUrl &url) // FIXME: remove
{
    auto urlList = setFileList(url);
    for (auto it = urlList.begin(); it != urlList.end(); it++) {
        qDebug() << it->filePath << it->source << it->type;
    }
}

void FileStash::mkdir(const QUrl &url, int permissions)
{
    qDebug() << "mkdirOut" << url;
    finished();
}

void FileStash::del(const QUrl &url, bool isFile)
{
    QDBusMessage msg = QDBusMessage::createMethodCall(
        "org.kde.kio.StashNotifier", "/StashNotifier", "", "removePath");
    msg << url.path();
    bool queued = QDBusConnection::sessionBus().send(msg);
    if (queued) {
        finished();
    } else {
        error(KIO::ERR_CANNOT_DELETE, url.path());
    }
}

#include "ioslave.moc"
