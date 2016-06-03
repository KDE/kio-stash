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

#include "kio_staging.h"
#include <QDebug>
#include <QMimeType>
#include <QMimeDatabase>
#include <QDir>
#include <QUrl>
#include <QFile>
#include <QCoreApplication>

class KIOPluginForMetaData : public QObject
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.kio.slave.staging" FILE "staging.json")
};

extern "C" {
    int Q_DECL_EXPORT kdemain(int argc, char **argv)
    {
        qDebug() << "out";
        QCoreApplication app(argc, argv);
        Staging slave(argv[2], argv[3]);
        slave.dispatchLoop();
        return 0;
    }
}

Staging::Staging(const QByteArray &pool, const QByteArray &app) : KIO::ForwardingSlaveBase("staging", pool, app)
{
    buildList();
}

bool Staging::rewriteUrl(const QUrl &url, QUrl &newUrl) //don't fuck around with this
{
    if (url.scheme() != QLatin1String("file")) {
        return false;
    }
    newUrl = url;
    return true;
}

void Staging::listRoot()
{
    KIO::UDSEntry entry;
    QString fileName;
    QString filePath;
    for (auto listIterator = m_List.begin(); m_List.begin() != m_List.end(); ++listIterator) {
        filePath = listIterator->path();
        fileName = QFileInfo(listIterator->url()).fileName();
        qDebug() << fileName;
        entry.clear();
        createRootUDSEntry(entry, filePath, fileName, fileName);
        listEntry(entry);
    }
}

void Staging::createRootUDSEntry( KIO::UDSEntry &entry, const QString &physicalPath, const QString &displayFileName, const QString &internalFileName) //needs a lot of changes imo
{
    QByteArray physicalPath_c = QFile::encodeName(physicalPath);
    QT_STATBUF buff;
    if (QT_LSTAT(physicalPath_c, &buff) == -1) {
        qWarning() << "couldn't stat " << physicalPath;
        return;
    }
    if (S_ISLNK(buff.st_mode)) {
        char buffer2[ 1000 ];
        int n = readlink(physicalPath_c, buffer2, 999);
        if (n != -1) {
            buffer2[ n ] = 0;
        }

        entry.insert(KIO::UDSEntry::UDS_LINK_DEST, QFile::decodeName(buffer2));
        // Follow symlink
        // That makes sense in kio_file, but not in the trash, especially for the size
        // #136876
#if 0
        if (KDE_stat(physicalPath_c, &buff) == -1) {
            // It is a link pointing to nowhere
            buff.st_mode = S_IFLNK | S_IRWXU | S_IRWXG | S_IRWXO;
            buff.st_mtime = 0;
            buff.st_atime = 0;
            buff.st_size = 0;
        }
#endif
    }

    mode_t type = buff.st_mode & S_IFMT; // extract file type
    mode_t access = buff.st_mode & 07777; // extract permissions
    //access &= 07555; // make it readonly, since it's in the trashcan
    Q_ASSERT(!internalFileName.isEmpty());
    entry.insert(KIO::UDSEntry::UDS_NAME, internalFileName);   // internal filename, like "0-foo"
    entry.insert(KIO::UDSEntry::UDS_DISPLAY_NAME, displayFileName);   // user-visible filename, like "foo"
    entry.insert(KIO::UDSEntry::UDS_FILE_TYPE, type);
    //if ( !url.isEmpty() )
    //    entry.insert( KIO::UDSEntry::UDS_URL, url );

    QMimeDatabase db;
    QMimeType mt = db.mimeTypeForFile(physicalPath);
    if (mt.isValid()) {
        entry.insert(KIO::UDSEntry::UDS_MIME_TYPE, mt.name());
    }
    entry.insert(KIO::UDSEntry::UDS_ACCESS, access);
    entry.insert(KIO::UDSEntry::UDS_SIZE, buff.st_size);
    entry.insert(KIO::UDSEntry::UDS_MODIFICATION_TIME, buff.st_mtime);
    entry.insert(KIO::UDSEntry::UDS_ACCESS_TIME, buff.st_atime);   // ## or use it for deletion time?
    /*QMimeDatabase db;
    QMimeType mt = db.mimeTypeForFile(physicalPath);
    if (mt.isValid()) {
        entry.insert(KIO::UDSEntry::UDS_MIME_TYPE, mt.name());
    }
    entry.insert(KIO::UDSEntry::UDS_LINK_DEST, physicalPath);
    entry.insert(KIO::UDSEntry::UDS_DISPLAY_NAME, displayFileName);
    entry.insert(KIO::UDSEntry::UDS_NAME, internalFileName);
    entry.insert(KIO::UDSEntry::UDS_TARGET_URL, physicalPath);*/
}

void Staging::buildList()
{
    m_List.append(QUrl("file:///home/nic/gsoc-2016"));
    m_List.append(QUrl("file:///home/nic/Dropbox"));
}

void Staging::listDir(const QUrl &url)
{
    //KIO::ForwardingSlaveBase::listDir(QUrl("file:///home/nic/gsoc-2016"));
    QString tmp = url.path();
    //dirty parsing hack, will fix later
    tmp.replace(0, QString("staging:").length(), "");
    if (tmp.isEmpty() || tmp == QString("/")) {
        listRoot();
    } else {
        qDebug() << tmp;
    }
    finished();
}

void Staging::rename(const QUrl &src, const QUrl &dest, KIO::JobFlags flags)
{
    KIO::ForwardingSlaveBase::rename(src, dest, flags);
}

bool Staging::checkURL(const QUrl &url)
{
    for(auto listIterator = m_List.begin(); listIterator != m_List.end(); listIterator++) {
        if(listIterator->url() == url.url()) {
            return true;
        }
    }
    return false;
}

#include "kio_staging.moc"
