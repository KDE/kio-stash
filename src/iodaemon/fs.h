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

#ifndef STASHFS_H
#define STASHFS_H

#include <QObject>
#include <QPointer>
#include <QHash>
#include <QString>
#include <QDebug>

class StashFileSystem : public QObject
{
    Q_OBJECT

public:

    enum NodeType {
        DirectoryNode,
        SymlinkNode,
        FileNode,
        InvalidNode
    };

    struct StashNodeData;

    typedef QHash<QString, StashNodeData> StashNode;

    struct StashNodeData
    {
        StashNodeData(NodeType ntype) :
            type(ntype),
            children(nullptr)
        {}
        ~StashNodeData()
        {}

        NodeType type;
        QString source;
        StashFileSystem::StashNode *children;
    };

    explicit StashFileSystem(QObject *parent = 0);
    virtual ~StashFileSystem();

    bool delEntry(const QString &path);
    bool addFile(const QString &src, const QString &dest);
    bool addFolder(const QString &dest);
    bool addSymlink(const QString &src, const QString &dest);

    StashNodeData findNode(QString path);
    StashNodeData findNode(QStringList path);

    void displayNode(StashNode *node)
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

    void displayRoot()
    {
        displayNode(root->children);
    }

private:
    StashNodeData *root;
    bool addNode(const QString &location, StashNodeData* data);
    void deleteChildren(StashNodeData nodeData);
    QStringList splitPath(const QString &path);
};

#endif
