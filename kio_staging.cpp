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
    for (auto listIterator = m_List.begin(); listIterator != m_List.end(); ++listIterator) {
        filePath = listIterator->path();
        fileName = QFileInfo(filePath).fileName();
        qDebug() << fileName;
        entry.clear();
        createRootUDSEntry(entry, filePath, fileName, fileName);
        listEntry(entry);
    }
    entry.clear();
    finished();
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
    }

    mode_t type = buff.st_mode & S_IFMT; // extract file type
    mode_t access = buff.st_mode & 07777; // extract permissions
    //access &= 07555; // make it readonly, since it's in the trashcan
    Q_ASSERT(!internalFileName.isEmpty());
    //entry.insert(KIO::UDSEntry::UDS_LOCAL_PATH, "/home/nic/Dropbox"); //this makes it work correctly!
    qDebug() << "physicalPath " << physicalPath;
    entry.insert(KIO::UDSEntry::UDS_NAME, physicalPath);   // internal filename, like "0-foo"
    entry.insert(KIO::UDSEntry::UDS_DISPLAY_NAME, displayFileName);   // user-visible filename, like "foo"
    entry.insert(KIO::UDSEntry::UDS_FILE_TYPE, type);
    QMimeDatabase db;
    QMimeType mt = db.mimeTypeForFile(physicalPath);
    if (mt.isValid()) {
        entry.insert(KIO::UDSEntry::UDS_MIME_TYPE, mt.name());
    }
    entry.insert(KIO::UDSEntry::UDS_ACCESS, access);
    entry.insert(KIO::UDSEntry::UDS_SIZE, buff.st_size);
    entry.insert(KIO::UDSEntry::UDS_MODIFICATION_TIME, buff.st_mtime);
    entry.insert(KIO::UDSEntry::UDS_ACCESS_TIME, buff.st_atime);
}

void Staging::buildList()
{
    m_List.append(QUrl("/home/nic/gsoc-2016"));
    m_List.append(QUrl("/home/nic/Dropbox"));
    m_List.append(QUrl("/home/nic/kdesrc/kde/applications"));
}

void Staging::listDir(const QUrl &url) //think a bit about finding a file under a subdir and not
                                        //allowing folders which are parents of a dir
{
    //KIO::ForwardingSlaveBase::listDir(QUrl("file:///home/nic/gsoc-2016"));
    QString tmp = url.path();
    //dirty parsing hack, will fix later
    //tmp.replace(0, QString("staging:").length(), "");
    qDebug() << "Received url" << tmp;
    if (tmp == QString("")) {
        listRoot();
        qDebug() << "Rootlist";
        return;
    } else if (true/*checkURL(url)*/) {
        QUrl mrl = "file://" + url.path();
        qDebug() << "Good url; FSB called" << mrl;
        KIO::ForwardingSlaveBase::listDir(mrl);
        return;
    }
    finished();
}

void Staging::rename(const QUrl &src, const QUrl &dest, KIO::JobFlags flags)
{
    KIO::ForwardingSlaveBase::rename(src, dest, flags);
}

bool Staging::checkURL(const QUrl &url) //replace with a more efficient algo later
{
    QUrl mrl = QUrl("/home/nic" + url.path());
    for(auto listIterator = m_List.begin(); listIterator != m_List.end(); listIterator++) {
        if(listIterator->url() == mrl.url()) {
            return true;
        }
    }
    qDebug() << "Bad Url" << mrl.path();
    return false;
}

#include "kio_staging.moc"
