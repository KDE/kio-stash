#ifndef STASHFS_H
#define STASHFS_H

#include <QObject>
#include <QPointer>
#include <QHash>
#include <QString>

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

    explicit StashFileSystem(QObject *parent = 0);
    virtual ~StashFileSystem();

    bool delEntry(QString path);
    bool addFile(QString src, QString dest);
    bool addFolder(QString dest);
    bool addSymlink(QString src, QString dest);

    StashNodeData findNode(QString path);
    StashNodeData findNode(QStringList path);

private:

    bool addNode(QString location, StashNodeData data);
    void deleteChildren(StashNodeData nodeData);
    QStringList splitPath(QString path);
    StashNodeData *root;
};

#endif
