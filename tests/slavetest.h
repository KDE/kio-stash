#ifndef SLAVETEST_H
#define SLAVETEST_H

#include <QObject>
#include <QTemporaryDir>

#include <KIO/Job>

class SlaveTest : public QObject
{
    Q_OBJECT

    public:
        SlaveTest() {}

    private slots:
        void initTestCase();
        void cleanupTestCase();

        void listRootDir();
        void listSubDir();

        void copyFileFromStash();
        void copyFileInDirectoryFromStash();
        void copyFileFromStash();
        void copySymlinkFromStash();

        void copyToStash();

        void createFile();
        void createDirectory();
        void renameFile();

        void delRootFile();
        void delFileInDirectory();
        void delDirectory();

        void statRoot();
        void statFileInRoot();
        void statDirectoryInRoot();
        void statSymlinkInRoot();
        void statFileInDirectory();

};
