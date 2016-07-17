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

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMetaType>
#include <QDBusMetaType>

#include <KDirWatch>
#include <KPluginFactory>
#include <KPluginLoader>
#include <kdirnotify.h>

K_PLUGIN_FACTORY_WITH_JSON(StashNotifierFactory, "stashnotifier.json", registerPlugin<StashNotifier>();)

StashNotifier::StashNotifier(QObject *parent, const QList<QVariant> &var) : KDEDModule(parent)
{
    dirWatch = new KDirWatch(this);
    qDebug() << "Launching STASH daemon";

    new StashNotifierAdaptor(this);
    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.registerObject("/StashNotifier", this);
    dbus.registerService("org.kde.kio.StashNotifier");

    fileSystem = new StashFileSystem(parent);

    connect(dirWatch, &KDirWatch::dirty, this, &StashNotifier::dirty);
    connect(dirWatch, &KDirWatch::created, this, &StashNotifier::created);
    //connect(dirWatch, &KDirWatch::deleted, this, &StashNotifier::removePath);
    connect(this, &StashNotifier::listChanged, this, &StashNotifier::displayRoot);
    addPath("/home/nic/ioslave.cpp", "/f", 2);
    addPath("", "/fas", 0);
    for (int i = 0; i < 3; i++) {
        qDebug() << i;
        addPath("/home/nic/" + QString::number(i), "/fas/" + QString::number(i), 2);
    }
    displayRoot();
    qDebug() << "init finished";
}

StashNotifier::~StashNotifier()
{
}

QString StashNotifier::encodeString(StashFileSystem::StashNode::iterator node, const QString &path) //format type::stashpath::source
{
    QString encodedString;

    switch (node.value().type) {
        case StashFileSystem::NodeType::DirectoryNode:
            encodedString = "dir";
            break;
        case StashFileSystem::NodeType::FileNode:
            encodedString = "file";
            break;
        case StashFileSystem::NodeType::SymlinkNode:
            encodedString = "symlink";
            break;
        case StashFileSystem::NodeType::InvalidNode:
            encodedString = "invalid";
            break;
    }

    if (path == "/") {
        encodedString += "::" + QStringLiteral("/") + node.key();
    } else {
        encodedString += "::" + path + QStringLiteral("/") + node.key();
    }


    if (node.value().type == StashFileSystem::NodeType::FileNode ||
        node.value().type == StashFileSystem::NodeType::SymlinkNode) {
        encodedString += "::" + node.value().source;
    } else {
        encodedString += "::";
    }
    //qDebug() << "ENCODED STRING" << encodedString;
    return encodedString;
}

QString StashNotifier::encodeString(StashFileSystem::StashNodeData nodeData, const QString &path)
{
    QString encodedString;

    switch (nodeData.type) {
        case StashFileSystem::NodeType::DirectoryNode:
            encodedString = "dir";
            break;
        case StashFileSystem::NodeType::FileNode:
            encodedString = "file";
            break;
        case StashFileSystem::NodeType::SymlinkNode:
            encodedString = "symlink";
            break;
        case StashFileSystem::NodeType::InvalidNode:
            encodedString = "invalid";
            break;
    }

    encodedString += "::" + path;

    if (nodeData.type == StashFileSystem::NodeType::FileNode ||
        nodeData.type == StashFileSystem::NodeType::SymlinkNode) {
        encodedString += "::" + nodeData.source;
    } else {
        encodedString += "::";
    }

    qDebug() << "ENCODED STRING4stats" << encodedString;
    return encodedString;
}

QStringList StashNotifier::fileList(const QString &path) //forwards list over QDBus to the KIO slave
{
    QStringList contents;
    StashFileSystem::StashNodeData node = fileSystem->findNode(path);
    if (node.type != StashFileSystem::NodeType::DirectoryNode) {
        contents.append("error::error::InvalidNode");
    } else {
        for (auto it = node.children->begin(); it != node.children->end(); it++) {
            contents.append(encodeString(it, path));
        }
    }
    return contents;
}

QString StashNotifier::fileInfo(const QString &path) //forwards data of a single file to the KIO slave
{
    QString fileData;
    StashFileSystem::StashNodeData node = fileSystem->findNode(path);
    fileData = encodeString(node, path);
    return fileData;
}

void StashNotifier::addPath(const QString &source, const QString &stashPath, const int &fileType)
{
    QString processedPath = processString(stashPath);
    if (fileSystem->findNode(stashPath).type == StashFileSystem::NodeType::InvalidNode) { //only folders not already exisiting are added
        if (fileType == StashFileSystem::NodeType::DirectoryNode) {
            dirWatch->addDir(processedPath);
            fileSystem->addFolder(processedPath);
        } else if (fileType == StashFileSystem::NodeType::FileNode) {
            dirWatch->addFile(source);
            fileSystem->addFile(processString(source), stashPath);
        } else if (fileType == StashFileSystem::NodeType::SymlinkNode) {
            dirWatch->addFile(source);
            fileSystem->addSymlink(processString(source), stashPath);
        }
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

void StashNotifier::removePath(const QString &path)
{
    qDebug() << "delete request called for " << path;
    StashFileSystem::NodeType fileType;
    QString processedPath = processString(path);
/*    if (fileType == StashFileSystem::NodeType::DirectoryNode) {
        dirWatch->removeDir(processedPath);
    } else {
        dirWatch->removeFile(processedPath);
    }*/
    fileSystem->delEntry(path);
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
