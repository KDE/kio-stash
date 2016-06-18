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
#include <QDir>
#include <QUrl>
#include <QFile>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusReply>

#include <KLocalizedString>

class KIOPluginForMetaData : public QObject
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.kio.slave.staging" FILE "ioslave.json")
};

FileStash::FileStash(const QByteArray &pool, const QByteArray &app) :
    KIO::ForwardingSlaveBase("staging", pool, app)
{
    updateList();
}

FileStash::~FileStash()
{}

void FileStash::updateList()
{
    QDBusMessage msg = QDBusMessage::createMethodCall("org.kde.StagingNotifier", "/StagingNotifier", "", "sendList");
    QDBusReply<QStringList> received = QDBusConnection::sessionBus().call(msg);
    if (received.isValid()) {
        m_List = received.value();
    }
}

bool FileStash::rewriteUrl(const QUrl &url, QUrl &newUrl) //don't fuck around with this
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
    displayList();
    KIO::UDSEntry entry;
    QString fileName;
    QString filePath;
    for (auto listIterator = m_List.begin(); listIterator != m_List.end(); ++listIterator) {
        filePath = *listIterator;
        fileName = QFileInfo(filePath).fileName();
        qDebug() << fileName;
        entry.clear();
        if (createRootUDSEntry(entry, filePath, fileName, fileName)) {
            listEntry(entry);
        } else {
            return;
        }
    }
    entry.clear();
    finished();
}

bool FileStash::createRootUDSEntry(KIO::UDSEntry &entry, const QString &physicalPath, const QString &displayFileName, const QString &internalFileName)
{
    QByteArray physicalPath_c = QFile::encodeName(physicalPath);
    QT_STATBUF buff;

    if (QT_LSTAT(physicalPath_c, &buff) == -1) { //if dir doesn't exist;
        error(KIO::ERR_COULD_NOT_READ, physicalPath);
        QDBusMessage msg = QDBusMessage::createMethodCall("org.kde.StagingNotifier", "/StagingNotifier", "", "removeDir");
        msg << physicalPath;
        QDBusConnection::sessionBus().send(msg); //tells the KDED that this path doesn't exist
        return false;
    }

    if (S_ISLNK(buff.st_mode)) {
        char buffer2[1000];
        int n = readlink(physicalPath_c, buffer2, 999);
        if (n != -1) {
            buffer2[n] = 0;
        }
        entry.insert(KIO::UDSEntry::UDS_LINK_DEST, QFile::decodeName(buffer2));
    }

    mode_t type = buff.st_mode & S_IFMT; // extract file type
    mode_t access = buff.st_mode & 07777; // extract permissions
    //access &= 07555; // make it readonly?

    Q_ASSERT(!internalFileName.isEmpty());
    qDebug() << "physicalPath " << physicalPath;

    QMimeDatabase db; //required for finding the mimeType
    QMimeType mt = db.mimeTypeForFile(physicalPath);
    if (mt.isValid()) {
        entry.insert(KIO::UDSEntry::UDS_MIME_TYPE, mt.name());
    }

    entry.insert(KIO::UDSEntry::UDS_NAME, physicalPath);   //internal filename, like ~/foo used for path.
    entry.insert(KIO::UDSEntry::UDS_DISPLAY_NAME, displayFileName);   //user-visible filename, like "foo"
    entry.insert(KIO::UDSEntry::UDS_FILE_TYPE, type);
    entry.insert(KIO::UDSEntry::UDS_ACCESS, access);
    entry.insert(KIO::UDSEntry::UDS_SIZE, buff.st_size);
    entry.insert(KIO::UDSEntry::UDS_MODIFICATION_TIME, buff.st_mtime);
    entry.insert(KIO::UDSEntry::UDS_ACCESS_TIME, buff.st_atime);

    return true;
}

void FileStash::listDir(const QUrl &url)
{
    updateList();
    if (url.path() == QString("") || url.path() == QString("/")) {
        listRoot();
        qDebug() << "Rootlist";
        return;
    } else if (checkUrl(url)) {
        QUrl newUrl;
        rewriteUrl(url, newUrl);
        qDebug() << "Good url; FSB called" << newUrl.path();
        KIO::ForwardingSlaveBase::listDir(newUrl);
        return;
    } else {
        error(KIO::ERR_SLAVE_DEFINED, i18n("The URL %1 either does not exist or has not been staged yet", url.path()));
    }
}

void FileStash::displayList() //actually print list :P
{
    for (auto it = m_List.begin(); it != m_List.end(); it++) {
        qDebug() << *it;
    }
}

bool FileStash::checkUrl(const QUrl &url) //replace with a more efficient algo later
{
    for (auto listIterator = m_List.begin(); listIterator != m_List.end(); listIterator++) {
        if (*listIterator == url.path() || url.path().startsWith(*listIterator)) { //prevents dirs which are not children from being accessed
            return true;
        }
    }
    qDebug() << "Bad Url" << url.path();
    return false;
}

#include "ioslave.moc"
