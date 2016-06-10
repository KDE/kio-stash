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

#include <KDirWatch>
#include <KPluginFactory>
#include <KPluginLoader>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <kdirnotify.h>

K_PLUGIN_FACTORY_WITH_JSON(StagingNotifierFactory,
        "stagingnotifier.json",
        registerPlugin<StagingNotifier>();)

StagingNotifier::StagingNotifier(QObject *parent, const QList<QVariant> &) : KDEDModule(parent)
{
    m_List = new QList<QUrl>(this);
    dirWatch = new KDirWatch(this);
    updateList();
    connect(dirWatch, &KDirWatch::dirty, this, &StagingNotifier::dirty);
}

void Staging::updateList() //convert to lambda fxn for C++0x swag and maintenance :P
{
    QString processedUrl;
    for (auto it = list->begin(); it != list->end(); it++) {
        processedUrl = it->path;
        processedUrl = processedUrl.simplified();
        if (QDir(processedUrl) {
            dirWatch->addDir(processedUrl);
            } else if (QFileInfo(QFile(processedUrl)).exists()) {
            dirWatch->addFile(processedUrl);
            } else {
            qDebug() << "File does not exist " << url.path();
            }
        }
}

void StagingNotifier::loadUrlList()
{
    QFile file("/tmp/staging-files");
    QString url;
    if (file.open(QIODevice::ReadOnly)) {
        while (!file.atEnd()) {
            url = file.readLine();
            // url = url.simplified();
            qDebug() << url;
            m_List->append(QUrl(url));
        }
    } else {
        qDebug() << "I/O ERROR";
    }
    file.close();
}

void StagingNotifier::watchDir(const QString &path)
{
    dirWatch->addDir(path);
}

void StagingNotifier::dirty(const QString &path)
{
}

#include "stagingnotifier.moc"
