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
    qDebug() << "Launching STAGING NOTIFIER DAEMON";
    new StagingNotifierAdaptor(this);
    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.registerObject("/StagingNotifier", this);
    dbus.registerService("org.kde.StagingNotifier");
    connect(dirWatch, &KDirWatch::dirty, this, &StagingNotifier::dirty);
    connect(dirWatch, &KDirWatch::created, this, &StagingNotifier::created);
    connect(dirWatch, &KDirWatch::deleted, this, &StagingNotifier::deleted);
    connect(this, &StagingNotifier::listChanged, this, &StagingNotifier::displayList);
}

void StagingNotifier::displayList()
{
    for (auto it = m_List.begin(); it != m_List.end(); it++) {
        qDebug() << it->path();
    }
}

void StagingNotifier::sendList()
{
//needs custom types iirc; to be coded later
}

void StagingNotifier::watchDir(const QString &path)
{
    if (QDir(path).exists()) {
        dirWatch->addDir(path);
    } else if (QFile(path).exists()) {
        dirWatch->addFile(path);
    }
    m_List.append(path);
    emit listChanged();
}

void StagingNotifier::removeDir(const QString &path)
{
    if (QDir(path).exists()) {
        dirWatch->removeDir(path);
    } else if (QFile(path).exists()) {
        dirWatch->removeFile(path);
    }
    m_List.removeAll(QUrl(path));
    emit listChanged();
}

void StagingNotifier::dirty(const QString &path)
{
    //what is supposed to happen here?
    //send a d-bus signal?
    qDebug() << "SOMETHING HAS CHANGED:" << path;
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
