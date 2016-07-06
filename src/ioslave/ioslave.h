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

#ifndef KIO_FILESTASH_H
#define KIO_FILESTASH_H

#include "../../dirlist.h"

#include <KIO/ForwardingSlaveBase>

class FileStash : public KIO::ForwardingSlaveBase
{
    Q_OBJECT

public:
    FileStash(const QByteArray &pool, const QByteArray &app);
    ~FileStash();

private:
    void registerMetaType();
    void listRoot();
    void displayList();
    bool createRootUDSEntry(
        KIO::UDSEntry &entry, const QString &physicalPath,
        const QString &displayFileName, const QString &internalFileName);
    QList<dirListDBus::dirList> setFileList();

protected:
    void listDir(const QUrl &url) Q_DECL_OVERRIDE;
    void mkdir(const QUrl &url, int permissions) Q_DECL_OVERRIDE;
    void del(const QUrl &url, bool isFile) Q_DECL_OVERRIDE;
    bool rewriteUrl(const QUrl &url, QUrl &newUrl) Q_DECL_OVERRIDE;
};

#endif
