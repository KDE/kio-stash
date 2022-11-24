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

#ifndef IOSLAVE_H
#define IOSLAVE_H

#include <QObject>
#include <QString>
#include <KIO/ForwardingSlaveBase>

class FileStash : public KIO::ForwardingSlaveBase
{
    Q_OBJECT

public:
    FileStash(const QByteArray &pool, const QByteArray &app,
              const QString &daemonService = "org.kde.kio.StashNotifier",
              const QString &daemonPath = "/StashNotifier");
    ~FileStash();

    enum NodeType {
        DirectoryNode,
        SymlinkNode,
        FileNode,
        InvalidNode
    };

    struct dirList
    {
        QString filePath;
        QString source;
        FileStash::NodeType type;

        dirList()
        {}

        ~dirList()
        {}

        dirList(const dirList &obj)
        {
            filePath = obj.filePath;
            source = obj.source;
            type = obj.type;
        }
    };

private:
    void createTopLevelDirEntry(KIO::UDSEntry &entry);
    bool isRoot(const QString &string);
    bool statUrl(const QUrl &url, KIO::UDSEntry &entry);
    bool createUDSEntry(
        KIO::UDSEntry &entry, const FileStash::dirList &fileItem);
    bool copyFileToStash(const QUrl &src, const QUrl &dest, KIO::JobFlags flags);
    bool copyStashToFile(const QUrl &src, const QUrl &dest, KIO::JobFlags flags);
    bool copyStashToStash(const QUrl &src, const QUrl &dest, KIO::JobFlags flags);
    bool deletePath(const QUrl &src);

    QStringList setFileList(const QUrl &url);
    QString setFileInfo(const QUrl &url);
    FileStash::dirList createDirListItem(const QString &fileInfo);

    const QString m_daemonService;
    const QString m_daemonPath;

protected:
    void listDir(const QUrl &url) override;
    void copy(const QUrl &src, const QUrl &dest, int permissions, KIO::JobFlags flags) override;
    void mkdir(const QUrl &url, int permissions) override;
    bool rewriteUrl(const QUrl &url, QUrl &newUrl) override;
    void del(const QUrl &url, bool isFile) override;
    void stat(const QUrl &url) override;
    void rename(const QUrl &src, const QUrl &dest, KIO::JobFlags flags) override;
};

#endif
