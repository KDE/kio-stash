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

        void copyFileToStash();
        void copyStashToFile();
        void copyStashToStash();
        void copyFileInDirectoryFromStash();
        void copySymlinkFromStash();

        void moveFileFromStash();
        void moveStashFromStash();

        void createDirectory();
        void renameFileInStash(); //currently fails

        void delRootFile();
        void delFileInDirectory();
        void delDirectory();

        void statRoot();
        void statFileInRoot();
        void statDirectoryInRoot();
        void statSymlinkInRoot();
        void statFileInDirectory();
    private:
        void stashFile(const QString &path);
        void stashDirectory(const QString &path);
        void stashSymlink(const QString &path);
        void copyFromStash(const QUrl &src, const QUrl &dest);
        void moveFromStash(const QUrl &src, const QUrl &dest);
};
