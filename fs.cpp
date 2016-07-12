#include "fs.h"

StashFileSystem::StashFileSystem(QObject *parent) :
    QObject(parent),
    root(new StashNodeData(DirectoryNode))
{
    root->children = new StashNode();
    addFolder("/kqe");
    //findNode("/kqe").children->insert("jskf", StashNodeData(DirectoryNode));
    addFolder("/kqe/ljkas");
    addFolder("/kqe/ljkas/ipqw");
    addFolder("/kqe/ljkas/ipqw/sfdsf");
    addFolder("/kqe/ljkas/ipqw/sfa1");
    addFolder("/kqe/ljkas/ipqw/sfa2");
    addFolder("/kqe/ljkas/ipqw/sfa3");
    displayRoot();
    //auto it = root->children->begin();
    //qDebug() << it.value();
    //qDebug() << findNode("/kqe/ljkas").children->begin().key();
    //findNode("/kqe/jskf").children->insert("asffd", StashNodeData(DirectoryNode));
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

QStringList StashFileSystem::splitPath(QString path)
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

bool StashFileSystem::addNode(QString location, StashNodeData* data)
{
    qDebug() <<"addnode" << location;
    QStringList path = splitPath(location);
    QString name = path.takeLast();
     qDebug() << "nameoaddnode"<< name;
     for (auto it = path.begin(); it != path.end(); it++)
         qDebug() << *it;
    StashNodeData baseData = findNode(path);

    if (!(baseData.type == DirectoryNode)) {
        delete data->children;
        return false;
    }
    //qDebug() << "parent" << path.last() << "name" << name;

    baseData.children->insert(name, *data);
    return true;
}

bool StashFileSystem::addFile(QString src, QString dest)
{
    StashNodeData *fileData = new StashNodeData(FileNode);
    fileData->source = src;
    return addNode(dest, fileData);
}

bool StashFileSystem::addSymlink(QString src, QString dest)
{
    StashNodeData *fileData = new StashNodeData(SymlinkNode);
    fileData->source = src;
    return addNode(dest, fileData);
}

bool StashFileSystem::addFolder(QString dest)
{
    StashNodeData *fileData = new StashNodeData(DirectoryNode);
    fileData->source = QStringLiteral("");
    fileData->children = new StashNode();
    qDebug() << "New sn ctred for " << dest;

    return addNode(dest, fileData);
}

StashFileSystem::StashNodeData StashFileSystem::findNode(QString path)
{
    qDebug() << "FINDNING NODE FOR" << path;
    return findNode(splitPath(path));
}

StashFileSystem::StashNodeData StashFileSystem::findNode(QStringList path)
{
    StashNode *node = root->children;
    StashNodeData data = StashNodeData(InvalidNode);
    if (!path.size()) {
        qDebug() << "RETURNING ROOT";
        return *root;
    } else {
        for (int i = 0; i < path.size(); ++i) {
            qDebug() << "LOOKING FOR" << path[i];
            if (node->contains(path[i])) {
                qDebug() << "Found" << path[i];
                data = node->value(path[i], StashNodeData(InvalidNode));
                if (data.type == DirectoryNode) {
                    //data.type = DirectoryNode;
                    //qDebug() << "REACHED HERE"<<path[i];
                    node = data.children;
                }
                if (i == path.size() - 1) {
                    qDebug() << "foudnode";
                    return data;
                } /*else {
                    qDebug() << "innode";
                    return StashNodeData(InvalidNode);
                }*/
            } else {
                qDebug() << "invanode";
                return StashNodeData(InvalidNode);
            }
        }
        qDebug() << "invalidnode";
        return StashNodeData(InvalidNode);
    }
}
