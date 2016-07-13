/**************************************************************************
*   Copyright (C) 2016 by Boudhayan Gupta <bgupta@kde.org>                *
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

    bool delEntry(QString path);
    bool addFile(QString src, QString dest);
    bool addFolder(QString dest);
    bool addSymlink(QString src, QString dest);

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
    bool addNode(QString location, StashNodeData* data);
    void deleteChildren(StashNodeData nodeData);
    QStringList splitPath(QString path);
};

#endif
