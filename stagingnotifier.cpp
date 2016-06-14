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
#include "stagingnotifier.h"
#include "staging_adaptor.h"
#include <kdirnotify.h>
//D-Bus to be implemented
K_PLUGIN_FACTORY_WITH_JSON(StagingNotifierFactory, "stagingnotifier.json", registerPlugin<StagingNotifier>();)

StagingNotifier::StagingNotifier(QObject *parent, const QList<QVariant> &var) : KDEDModule(parent)
{
    dirWatch = new KDirWatch(this);
    loadUrlList();
    //updateList();
    qDebug() << "Launching STAGING NOTIFIER DAEMON";
    //dirWatch->addFile("/tmp/staging-files");
    // new StagingNotifierAdaptor(this);
    //StagingNotifierAdaptor *adaptor = new StagingNotifierAdaptor(this);
    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.registerObject("/StagingNotifier", this);
    dbus.registerService("org.kde.StagingNotifier");
    connect(dirWatch, &KDirWatch::dirty, this, &StagingNotifier::dirty);
    connect(dirWatch, &KDirWatch::created, this, &StagingNotifier::created);
    connect(dirWatch, &KDirWatch::deleted, this, &StagingNotifier::deleted);
}

void StagingNotifier::updateList() //convert to lambda fxn for C++0x swag and maintenance :P
{
    QString processedUrl;
    for (auto it = m_List.begin(); it != m_List.end(); it++) {
        processedUrl = it->path();
        processedUrl = processedUrl.simplified();
        if (QDir(processedUrl).exists()) {
            dirWatch->addDir(processedUrl);
            } else if (QFileInfo(QFile(processedUrl)).exists()) {
            dirWatch->addFile(processedUrl);
            } else {
            qDebug() << "File does not exist" << processedUrl;
            }
        }
}

void StagingNotifier::sendList()
{
//needs custom types iirc; to be coded later
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
            m_List.append(QUrl(url));
        }
        updateList();
    } else {
        qDebug() << "I/O ERROR";
    }
    file.close();
}

void StagingNotifier::watchDir(const QString &path)
{
    dirWatch->addDir(path);
    emit listChanged();
}

void StagingNotifier::removeDir(const QString &path)
{
    dirWatch->removeDir(path);
    emit listChanged();
}

void StagingNotifier::dirty(const QString &path)
{
    //what is supposed to happen here?
    //send a d-bus signal?
    qDebug() << "SOMETHING HAS CHANGED:" << path;
    if (path == QString("/tmp/staging-files")) {
        loadUrlList();
    }
}

void StagingNotifier::created(const QString &path)
{
    //kded should keep pinging /tmp/staging-files
    qDebug() << "CREATED:" << path;
}

void StagingNotifier::deleted(const QString &path)
{
    //should ideally remove the URL from dirWatch
    qDebug() << "REMOVED:" << path;
}

#include "stagingnotifier.moc"
