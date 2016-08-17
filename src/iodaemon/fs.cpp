/*
 * Copyright 2016 Boudhayan Gupta <bgupta@kde.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of KDE e.V. (or its successor approved by the
 *    membership of KDE e.V.) nor the names of its contributors may be used
 *    to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "fs.h"

StashFileSystem::StashFileSystem(QObject *parent) :
    QObject(parent),
    root(DirectoryNode)
{
    root.children = new StashNode();
    displayRoot();
}

StashFileSystem::~StashFileSystem()
{
    deleteChildren(root);
}

void StashFileSystem::deleteChildren(StashNodeData nodeData)
{
    if (nodeData.children != nullptr) {
        Q_FOREACH (auto data, nodeData.children->values()) {
            deleteChildren(data);
        }
        delete nodeData.children;
        nodeData.children = nullptr;
    }
}

QStringList StashFileSystem::splitPath(const QString &path)
{
    QString filePath = path;
    if (filePath.startsWith('/')) {
        filePath = filePath.right(filePath.size() - 1);
    }

    if (filePath.endsWith('/')) {
        filePath = filePath.left(filePath.size() - 1);
    }
    return filePath.split('/');
}

bool StashFileSystem::delEntry(const QString &location)
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

bool StashFileSystem::addNode(const QString &location, const StashNodeData &data)
{
    QStringList path = splitPath(location);
    QString name = path.takeLast();
    StashNodeData baseData = findNode(path);

    if (!(baseData.type == DirectoryNode)) {
        deleteChildren(data);
        return false;
    }

    baseData.children->insert(name, data);
    return true;
}

bool StashFileSystem::addFile(const QString &src, const QString &dest)
{
    StashNodeData fileData(FileNode);
    fileData.source = src;
    return addNode(dest, fileData);
}

bool StashFileSystem::addSymlink(const QString &src, const QString &dest)
{
    StashNodeData fileData(SymlinkNode);
    fileData.source = src;
    return addNode(dest, fileData);
}

bool StashFileSystem::addFolder(const QString &dest)
{
    StashNodeData fileData(DirectoryNode);
    fileData.source = QStringLiteral("");
    fileData.children = new StashNode();

    return addNode(dest, fileData);
}

bool StashFileSystem::copyFile(const QString &src, const QString &dest)
{
    StashNodeData fileToCopy = findNode(src);
    return addNode(dest, fileToCopy);
}

StashFileSystem::StashNodeData StashFileSystem::findNode(QString path)
{
    return findNode(splitPath(path));
}

StashFileSystem::StashNodeData StashFileSystem::findNode(QStringList path)
{
    StashNode *node = root.children;
    StashNodeData data = StashNodeData(InvalidNode);
    if (!path.size() || path.at(0) == "") {
        return root;
    } else {
        for (int i = 0; i < path.size(); ++i) {
            if (node->contains(path[i])) {
                data = node->value(path[i], StashNodeData(InvalidNode));
                if (data.type == DirectoryNode) {
                    node = data.children;
                }
                if (i == path.size() - 1) {
                    return data;
                }
            } else {
                return StashNodeData(InvalidNode);
            }
        }
        return StashNodeData(InvalidNode);
    }
}

void StashFileSystem::deleteAllItems()
{
    deleteChildren(root);
}

void StashFileSystem::displayNode(StashNode *node)
{
    for (auto it = node->begin(); it != node->end(); it++)
    {
        qDebug() << "stashpath" << it.key();
        qDebug() << "filepath" << it.value().source;
        qDebug() << "filetype" << it.value().type;
        if (it.value().type == DirectoryNode) {
            qDebug() << "parent" << it.key();
            displayNode(it.value().children);
        }
    }
    return;
}

void StashFileSystem::displayRoot()
{
    displayNode(root.children);
}
