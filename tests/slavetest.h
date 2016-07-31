#ifndef SLAVETEST_H
#define SLAVETEST_H

#include <QObject>
#include <KIO/Job>

class SlaveTest : public QObject
{
    Q_OBJECT

    public:
        SlaveTest() {}

        enum NodeType {
            DirectoryNode,
            SymlinkNode,
            FileNode,
            InvalidNode
        };

    private slots:
        void initTestCase();
        void cleanupTestCase();

        void listRootDir();
        void listSubDir();

        void copyFileToStash();
        void copyStashToFile();
        void copyStashToStash();
        void copySymlinkFromStash();

        void moveToFileFromStash();
        void moveToStashFromStash();

        //void renameFileInStash(); //currently fails

        void delRootFile();
        void delFileInDirectory();
        void delDirectory();

        void statRoot();
        void statFileInRoot();
        void statDirectoryInRoot();
        void statSymlinkInRoot();
        void statFileInDirectory();
    private:
        QString tmpDirPath() const;
        void statUrl(const QUrl &url, KIO::UDSEntry &entry);
        void stashFile(const QString &realPath, const QString &stashPath);
        void stashSymlink(const QString &realPath, const QString &stashPath);
        void stashDirectory(const QString &path);
        void stashCopy(const QUrl &src, const QUrl &dest);
        void createDirectory();
        void moveFromStash(const QUrl &src, const QUrl &dest);
        void deleteFromStash(const QUrl &url);
};

#endif
