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

#include "stashnotifier.h"
#include "stash_adaptor.h"
#include "fs.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>

#include <KDirWatch>
#include <KPluginFactory>
#include <KPluginLoader>
#include <kdirnotify.h>

K_PLUGIN_FACTORY_WITH_JSON(StashNotifierFactory, "stashnotifier.json", registerPlugin<StashNotifier>();)

StashNotifier::StashNotifier(QObject *parent, const QList<QVariant> &var) : KDEDModule(parent)
{
    dirWatch = new KDirWatch(this);
    qDebug() << "Launching STASH NOTIFIER DAEMON";

    new StashNotifierAdaptor(this);
    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.registerObject("/StashNotifier", this);
    dbus.registerService("org.kde.kio.StashNotifier");

    connect(dirWatch, &KDirWatch::dirty, this, &StashNotifier::dirty);
    connect(dirWatch, &KDirWatch::created, this, &StashNotifier::created);
    connect(dirWatch, &KDirWatch::deleted, this, &StashNotifier::removePath);
    connect(this, &StashNotifier::listChanged, this, &StashNotifier::displayList);
}

StashNotifier::~StashNotifier()
{
}

void StashNotifier::displayList()
{
    for (auto it = m_List.begin(); it != m_List.end(); it++) {
        qDebug() << *it;
    }
}

QStringList StashNotifier::fileList() //forwards list over QDBus to the KIO slave
{
    return m_List;
}

void StashNotifier::addPath(const QString &path)
{
    QString processedPath = processString(path);
    if (!m_List.contains(processedPath)) {
        if (QFileInfo(processedPath).isDir()) {
            dirWatch->addDir(processedPath);
        } else if (QFileInfo(processedPath).isFile()) {
            dirWatch->addFile(processedPath);
        }
        m_List.append(processedPath);
        emit listChanged();
    }
}

QString StashNotifier::processString(const QString &path) //removes trailing slash and strips newline character
{
    QString processedPath = path.simplified();
    if (processedPath.at(processedPath.size() - 1) == '/') {
        processedPath.chop(1);
    }
    return processedPath;
}

void StashNotifier::removePath(const QString &path) //handles KDirWatch and QDBus signals
{
    QString processedPath = processString(path);
    if (QFileInfo(processedPath).isDir()) {
        dirWatch->removeDir(processedPath);
    } else if (QFileInfo(processedPath).isFile()) {
        dirWatch->removeFile(processedPath);
    }
    m_List.removeAll(processedPath);
    emit listChanged();
}

void StashNotifier::dirty(const QString &path)
{
    //what is supposed to happen here?
    qDebug() << "SOMETHING HAS CHANGED:" << path;
}

void StashNotifier::created(const QString &path)
{
    qDebug() << "CREATED:" << path;
}

#include "stashnotifier.moc"
