/***************************************************************************
 *   Copyright (C) 2016 by Boudhayan GUp <bgupta@kde.org>                  *
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

#include "fs.h"

struct StashFileSystem::StashNodeData
{
    StashNodeData(NodeType ntype) : //ctr for SND
        type(ntype),                //can be dir/file/symlink
        children(nullptr)
    {}

    NodeType type;
    QString source;
    StashFileSystem::StashNode *children;
};

StashFileSystem::StashFileSystem(QObject *parent) :
    QObject(parent),
    root(new StashNodeData(DirectoryNode))
{
    root->children = new StashNode(); //hashmap w/path and type
}

StashFileSystem::~StashFileSystem()
{
    deleteChildren(*root);
    delete root;
}

void StashFileSystem::deleteChildren(StashNodeData nodeData)
{
    if (nodeData.children != nullptr) {
        Q_FOREACH(auto data, nodeData.children->values()) {
            deleteChildren(data);
        }
        delete nodeData.children;
        nodeData.children = nullptr;
    }
}

QStringList StashFileSystem::splitPath(QString path) //split into names
{
    if (path.startsWith('/')) {
        path = path.right(path.size() - 1);
    }

    if (path.endsWith('/')) {
        path = path.left(path.size() - 1);
    }

    return path.split('/');
}

bool StashFileSystem::delEntry(QString location)
{
    QStringList path = splitPath(location);
    QString name = path.takeLast();
    StashNodeData baseData = findNode(path);

    if (!(baseData.type == DirectoryNode)) {
        return false;
    }

    if (!(baseData.children->contains(name))) {
        return false;
    }

    deleteChildren(baseData.children->value(name, StashNodeData(InvalidNode)));
    return (baseData.children->remove(name) > 0);
}

bool StashFileSystem::addNode(QString location, StashNodeData data)
{
    QStringList path = splitPath(location);
    QString name = path.takeLast();
    StashNodeData baseData = findNode(path);

    if (!(baseData.type == DirectoryNode)) {
        delete data.children;
        return false;
    }

    baseData.children->insert(name, data);
    return true;
}

bool StashFileSystem::addFile(QString src, QString dest)
{
    StashNodeData fileData = StashNodeData(FileNode);
    fileData.source = src;
    return addNode(dest, fileData);
}

bool StashFileSystem::addSymlink(QString src, QString dest)
{
    StashNodeData fileData = StashNodeData(SymlinkNode);
    fileData.source = src;
    return addNode(dest, fileData);
}

bool StashFileSystem::addFolder(QString dest)
{
    StashNodeData fileData = StashNodeData(DirectoryNode);
    fileData.children = new StashNode();
    return addNode(dest, fileData);
}

StashFileSystem::StashNodeData StashFileSystem::findNode(QString path)
{
    return findNode(splitPath(path));
}

StashFileSystem::StashNodeData StashFileSystem::findNode(QStringList path)
{
    StashNode *node = root->children;
    StashNodeData data = StashNodeData(InvalidNode);

    for (int i = 0; i < path.size(); ++i) {
        if (node->contains(path[i])) {
            data = node->value(path[i], StashNodeData(InvalidNode));
            if (data.type == DirectoryNode) {
                node = data.children;
            }

            if (i == path.size() - 1) {
                return data;
            } else {
                return StashNodeData(InvalidNode);
            }
        } else {
            return StashNodeData(InvalidNode);
        }
    }
    return StashNodeData(InvalidNode);
}
