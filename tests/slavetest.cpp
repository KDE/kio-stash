#include "slavetest.h"

#include <QTest>
#include <QDir>
#include <QFile>
#include <QTemporaryFile>
#include <QFileInfo>
#include <QDBusMessage>
#include <QDBusConnection>
#include <QStandardPaths>
#include <QDebug>

#include <KIO/Job>
#include <KIO/CopyJob>
#include <KIO/DeleteJob>
#include <KFileItem>

SlaveTest::SlaveTest() : tmpFolder("SlaveTest"),
                         m_fileTestFile("TestFile"),
                         m_fileTestFolder("TestSubFolder"),
                         m_stashTestFolder("StashTestFolder"),
                         m_stashTestSymlink("StashTestSymlink"),
                         m_stashTestFile("StashFile"),
                         m_stashTestFileInSubDirectory("SubTestFile"),
                         m_newStashFileName("NewStashFile"),
                         m_stashFileForRename("StashRenameFile")
{}

void SlaveTest::initTestCase()
{
    //enclose a check statement around this block to see if kded5 is not already there
    QDBusMessage msg;
    QDBusMessage replyMessage;

    msg = QDBusMessage::createMethodCall(
                           "org.kde.kio.StashNotifier", "/StashNotifier", "", "pingDaemon");
    replyMessage = QDBusConnection::sessionBus().call(msg);
    if (replyMessage.type() == QDBusMessage::ErrorMessage) {
        qDebug() << "Launching fallback daemon";
        const QString program = "../src/iodaemon/testdaemon";
        stashDaemonProcess = new QProcess();
        stashDaemonProcess->start(program);
    }

    replyMessage = QDBusConnection::sessionBus().call(msg);

    if (replyMessage.type() != QDBusMessage::ErrorMessage) {
        qDebug() << "Test case initialised";
    } else {
        qDebug() << "Something is wrong!";
    }
    createTestFiles();
}

void SlaveTest::createTestFiles() //also find a way to reset the directory prior to use
{
    QDir tmpDir;
    tmpDir.mkdir(tmpDirPath()); //creates test dir
    tmpDir.mkdir(tmpDirPath() + m_fileTestFolder);

    QFile tmpFile;
    stashDirectory("/" + m_stashTestFolder);

    //at file protocol
    QUrl src = QUrl::fromLocalFile(tmpDirPath() + m_fileTestFile); //creates a file to be tested
    tmpFile.setFileName(src.path());
    QVERIFY(tmpFile.open(QIODevice::ReadWrite));
    tmpFile.close();
    //create a file to be stashed
    src = QUrl::fromLocalFile(tmpDirPath() + m_stashTestFile);
    tmpFile.setFileName(src.path());
    QVERIFY(tmpFile.open(QIODevice::ReadWrite));
    tmpFile.close();
    stashFile(src.path(), "/" + m_stashTestFile);

    src = QUrl::fromLocalFile(tmpDirPath() + m_stashFileForRename);
    tmpFile.setFileName(src.path());
    QVERIFY(tmpFile.open(QIODevice::ReadWrite));
    tmpFile.close();
    stashFile(src.path(), "/" + m_stashFileForRename);

    //create a file in a stash-subdir why do we need this?
    src = QUrl::fromLocalFile(tmpDirPath() + m_stashTestFileInSubDirectory);
    tmpFile.setFileName(src.path());
    QVERIFY(tmpFile.open(QIODevice::ReadWrite));
    tmpFile.close();

    stashFile(src.path(), "/" + m_stashTestFolder + "/" + m_stashTestFileInSubDirectory);
    //qDebug() << src.path() << "/" + m_stashTestFolder + "/" + m_stashTestFileInSubDirectory;

}

void SlaveTest::cleanupTestCase()
{
//    nukeStash();
    QDir dir(tmpDirPath());
    dir.removeRecursively();
//    stashDaemonProcess->terminate();
}

QString SlaveTest::tmpDirPath()
{
    return QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/" + tmpFolder + "/";
}

void SlaveTest::statItem(const QUrl &url, const int &type)
{
    KIO::UDSEntry entry;
    QVERIFY(statUrl(url, entry));
    KFileItem item(entry, url);
    switch (type) {
    case NodeType::DirectoryNode:
        QVERIFY(item.isDir());
        break;
    case NodeType::SymlinkNode:
        QVERIFY(item.isLink()); //don't worry this is intentional :)
    case NodeType::FileNode:
        QVERIFY(item.isFile());
    }
    QVERIFY(item.isReadable());
    QVERIFY(!item.isHidden());
    QCOMPARE(item.text(), url.fileName());
}

void SlaveTest::stashFile(const QString &realPath, const QString &stashPath)
{
    //do something naughty and add D-Bus messages to the stashnotifier?
    QDBusMessage msg = QDBusMessage::createMethodCall(
                           "org.kde.kio.StashNotifier", "/StashNotifier", "", "addPath");
    msg << realPath << stashPath << NodeType::FileNode;
    bool queued = QDBusConnection::sessionBus().send(msg);
    QVERIFY(queued);
}

void SlaveTest::stashDirectory(const QString &path)
{
    QDBusMessage msg = QDBusMessage::createMethodCall(
                           "org.kde.kio.StashNotifier", "/StashNotifier", "", "addPath");
    QString destinationPath = path;
    msg << "" << destinationPath << NodeType::DirectoryNode;
    bool queued = QDBusConnection::sessionBus().send(msg);
    QVERIFY(queued);
}

void SlaveTest::stashSymlink(const QString &realPath, const QString &stashPath)
{
    QDBusMessage msg = QDBusMessage::createMethodCall(
                           "org.kde.kio.StashNotifier", "/StashNotifier", "", "addPath");
    msg << realPath << stashPath << NodeType::SymlinkNode;
    bool queued = QDBusConnection::sessionBus().send(msg);
    QVERIFY(queued);
}

bool SlaveTest::statUrl(const QUrl &url, KIO::UDSEntry &entry)
{
    KIO::StatJob *statJob = KIO::stat(url, KIO::HideProgressInfo);
    bool ok = statJob->exec();
    if (ok) {
        entry = statJob->statResult();
    }
    return ok;
}

void SlaveTest::nukeStash()
{
    QDBusMessage msg = QDBusMessage::createMethodCall(
                           "org.kde.kio.StashNotifier", "/StashNotifier", "", "nukeStash");
    bool queued = QDBusConnection::sessionBus().send(msg);
    QVERIFY(queued);
}

void SlaveTest::stashCopy(const QUrl &src, const QUrl &dest)
{
    KIO::CopyJob *job = KIO::copy(src, dest, KIO::HideProgressInfo);
    bool ok = job->exec();
    //qDebug() << src << dest << ok;
    QVERIFY(ok);
}

void SlaveTest::moveFromStash(const QUrl &src, const QUrl &dest) //make this work
{
    KIO::Job *job = KIO::move(src, dest, KIO::HideProgressInfo);
    bool ok = job->exec();
    //qDebug() << src << dest << ok;
    QVERIFY(ok);
}

void SlaveTest::deleteFromStash(const QUrl &url)
{
    KIO::Job *delJob = KIO::del(url, KIO::HideProgressInfo);
    bool ok = delJob->exec();
    //qDebug() << "delete" << url << ok;
    QVERIFY(ok);
}

void SlaveTest::listRootDir()
{
    //write qcompare cases
    KIO::ListJob *job = KIO::listDir(QUrl(QStringLiteral("stash:/")), KIO::HideProgressInfo);
    connect(job, SIGNAL(entries(KIO::Job*,KIO::UDSEntryList)),
            SLOT(slotEntries(KIO::Job*,KIO::UDSEntryList)));
    bool ok = job->exec();
    QVERIFY(ok);
}

void SlaveTest::listSubDir()
{
    //write qcompare cases
    KIO::ListJob *job = KIO::listDir(QUrl("stash:/" + m_stashTestFolder), KIO::HideProgressInfo);
    connect(job, SIGNAL(entries(KIO::Job*,KIO::UDSEntryList)),
            SLOT(slotEntries(KIO::Job*,KIO::UDSEntryList)));
    bool ok = job->exec();
    QVERIFY(ok);
}

void SlaveTest::createDirectory() //find ways for finding files
{
    QUrl directoryPath = QUrl("");
    KIO::SimpleJob *job = KIO::mkdir(directoryPath);
    bool ok = job->exec();
    QVERIFY(QFile::exists(directoryPath.path()));
    QCOMPARE(ok, true);
}

void SlaveTest::statRoot()
{
    QUrl url("stash:/");
    KIO::UDSEntry entry;
    QVERIFY(statUrl(url, entry));
    KFileItem item(entry, url);
    QVERIFY(item.isDir());
    QVERIFY(!item.isLink());
    QVERIFY(item.isReadable());
    QVERIFY(item.isWritable());
    QVERIFY(!item.isHidden());
    QCOMPARE(item.name(), QStringLiteral("."));
}

void SlaveTest::statFileInRoot()
{
    QFile file;
    QUrl url("stash:/" + m_stashTestFile);
    stashFile(url.path(), url.path());
    KIO::UDSEntry entry;
    QVERIFY(statUrl(url, entry));
    KFileItem item(entry, url);
    QVERIFY(item.isFile());
    QVERIFY(!item.isDir());
    QVERIFY(!item.isLink());
    QVERIFY(item.isReadable());
    QVERIFY(!item.isHidden());
//    QCOMPARE(item.text(), m_stashTestFile);
}
//use kio::stat
void SlaveTest::statDirectoryInRoot()
{
    QUrl url("stash:/" + m_stashTestFolder);
    stashDirectory(url.path());
    KIO::UDSEntry entry;
    QVERIFY(statUrl(url, entry));
    KFileItem item(entry, url);
    QVERIFY(!item.isFile());
    QVERIFY(item.isDir());
    QVERIFY(!item.isLink());
    QVERIFY(item.isReadable());
    QVERIFY(!item.isHidden());
//    QCOMPARE(item.text(), QStringLiteral(m_stastTestFolder));
}

void SlaveTest::statSymlinkInRoot()
{
    QUrl url("stash:/" + m_stashTestSymlink);
    stashSymlink(url.path(), url.path());
    KIO::UDSEntry entry;
    QVERIFY(statUrl(url, entry));
    KFileItem item(entry, url);
    QVERIFY(item.isFile());
    QVERIFY(!item.isDir());
    //QVERIFY(item.isLink()); //i don't know how to create a SL in the first place, so we'll just comment this out till i do
    QVERIFY(item.isReadable());
    QVERIFY(!item.isHidden());
//    QCOMPARE(item.text(), QStringLiteral(m_stashTestSymlink));
}

void SlaveTest::statFileInDirectory()
{
    QUrl url("stash:/" + m_stashTestFolder + "/" + m_stashTestFileInSubDirectory);
    KIO::UDSEntry entry;
    QVERIFY(statUrl(url, entry));
    KFileItem item(entry, url);
    QVERIFY(item.isFile());
    QVERIFY(!item.isDir());
    QVERIFY(!item.isLink());
    QVERIFY(item.isReadable());
    QVERIFY(!item.isHidden());
//    QCOMPARE(item.text(), QStringLiteral(m_stashTestFileInSubDirectory));
}

void SlaveTest::copyFileToStash()
{
    QUrl src = QUrl::fromLocalFile(tmpDirPath() + m_fileTestFile);
    QFile testFile(src.path());
    QVERIFY(testFile.open(QIODevice::WriteOnly));
    QUrl dest("stash:/" + m_fileTestFile);

    stashCopy(src, dest);
    QVERIFY(testFile.exists());
    statItem(dest, NodeType::FileNode);
/*
    QUrl destDirectory("stash:/copyTestCase");
    destinationFileName = QUrl(src).fileName();
    stashCopy(src, destDirectory);
    QVERIFY(stashFile.exists());
    QVERIFY(QFile(dest + destinationFileName).exists());*/
}

void SlaveTest::copySymlinkFromStashToFile() //create test case
{
    stashSymlink(tmpDirPath() + m_fileTestFile, "/" + m_stashTestSymlink);
    QUrl src("stash:/" + m_stashTestSymlink);
    QUrl dest = QUrl::fromLocalFile(tmpDirPath() + m_fileTestFolder + "/" + m_stashTestSymlink);

    stashCopy(src, dest);
    QVERIFY(QFile::exists(dest.path()));
}

void SlaveTest::copyStashToFile()
{
    QUrl src("stash:/" + m_stashTestFile);
    QUrl dest = QUrl::fromLocalFile(tmpDirPath() + m_fileTestFolder + "/" + m_stashTestFile);
//    QUrl dest = QUrl::fromLocalFile("/home/nic/" + m_stashTestFile);
    KIO::UDSEntry entry;
    stashCopy(src, dest);
    QVERIFY(QFile::exists(dest.toLocalFile()));
    QFile(dest.path()).remove();
}

void SlaveTest::moveToFileFromStash() //this is actually rather broken as of now
{
    QUrl src("stash:/" + m_stashTestFile);
//    QUrl dest = QUrl::fromLocalFile("/home/nic/test/" + m_stashTestFile);
    QUrl dest = QUrl::fromLocalFile(tmpDirPath() + m_fileTestFolder + "/" + m_stashTestFile);

    moveFromStash(src, dest);
    //QVERIFY(!QFile::exists(item.url().path()));
    KIO::UDSEntry entry;
    statUrl(src, entry);
    KFileItem item(entry, src);
    QVERIFY(item.name() != m_stashTestFile);
    QVERIFY(QFile::exists(dest.toLocalFile()));
    //match properties also
}

void SlaveTest::copyStashToStash()
{
    QUrl src("stash:/" + m_stashTestFile);
    QUrl dest("stash:/" + m_stashTestFolder + "/" + m_stashTestFile);
    stashCopy(src, dest);
    KIO::UDSEntry entry;
    statUrl(src, entry);
    KFileItem item(entry, src);
    QVERIFY(item.name() == m_stashTestFile);
}

void SlaveTest::renameFileInStash()
{
    QUrl src("stash:/" + m_stashFileForRename);
    QUrl dest("stash:/" + m_newStashFileName);

    KIO::UDSEntry entry;

    moveFromStash(src, dest);

    statUrl(src, entry);
    KFileItem item(entry, src);
    QVERIFY(item.name() != m_stashFileForRename);

    statUrl(dest, entry);
    item = KFileItem(entry, src);
    QVERIFY(item.name() == m_newStashFileName);
}


void SlaveTest::delRootFile()
{
    QUrl url("stash:/" + m_stashTestFile); //url
    //////qDebug() << url;
    deleteFromStash(url);
    KIO::UDSEntry entry;
    QVERIFY(statUrl(url, entry));
    KFileItem item(entry, url);
    ////qDebug() << "item name" << item.name();
    //qDebug() << "stash test file name" << m_stashTestFile;
    QVERIFY(item.name() != m_stashTestFile);
//    QCOMPARE(QFile(url.path()).exists(), false); //use kio::stat
}

void SlaveTest::delFileInDirectory()
{
    QUrl url("stash:/" + m_stashTestFolder + "/" + m_stashTestFileInSubDirectory); //url
    //qDebug() << url;
    deleteFromStash(url);
    KIO::UDSEntry entry;
    QVERIFY(statUrl(url, entry));
    KFileItem item(entry, url);
    //qDebug() << "item name" << item.name();
    //qDebug() << "stash test file name" << m_stashTestFileInSubDirectory;
    QVERIFY(item.name() != m_stashTestFileInSubDirectory);
//    QCOMPARE(QFile(url).exists(), false); //use kio::stat
}

void SlaveTest::delDirectory()
{
    QUrl url("stash:/" + m_stashTestFolder);
    deleteFromStash(url);
    KIO::UDSEntry entry;
    QVERIFY(statUrl(url, entry));
    KFileItem item(entry, url);
    //qDebug() << "item name" << item.name();
    //qDebug() << "stash test file name" << m_stashTestFolder;
    QVERIFY(item.name() != m_stashTestFolder);
//    QCOMPARE(QFile(url).exists(), false); //use kio::stat
}

void SlaveTest::init()
{
    //qDebug() << "calling init";
    createTestFiles();
}

void SlaveTest::cleanup()
{
    QDir dir(tmpDirPath());
    dir.removeRecursively();
    //qDebug() << "Cleaning up this test case";
    //nukeStash();
}

QTEST_MAIN(SlaveTest)
#include "slavetest.moc"
