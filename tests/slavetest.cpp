#include "slavetest.h"
#include "../src/ioslave/ioslave.h"

#include <QTest>
#include <QString>
#include <QDir>
#include <QFile>
#include <QTemporaryFile>
#include <QFileInfo>
#include <QDBusMessage>
#include <QDBusConnection>
#include <QStandardPaths>

#include <KIO/Job>
#include <KIO/CopyJob>
#include <KIO/DeleteJob>
#include <KFileItem>

SlaveTest::SlaveTest() : tmpFolder("SlaveTest") m_fileTestFile("TestFile"),
                        m_stashTestFile("StashFile"), m_fileTestFolder("TestSubFolder"),
                        m_stashTestFolder("StashTestFolder"), m_stastTestSymlink("StashTestSymlink"),
                        m_stashTestFileInSubDirectory("SubTestFile")
{
}

void SlaveTest::initTestCase()
{
    //qDebug() << QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)+ QString("/slavetest/");
    //enclose a check statement around this block to see if kded5 is not already there
    createTestFiles();
    const QString program = "../src/iodaemon/testdaemon";
    stashDaemonProcess = new QProcess();
    stashDaemonProcess->start(program);
}

void createTestFiles() //also find a way to reset the directory prior to use
{
    QDir tmpDir;
    tmpDir.mkdir(tmpDirPath());

    QUrl src = QUrl::toLocalFile(tmpDirPath() + m_fileTestFile); //creates a file to be tested
    QFile testFile(src);
    QVERIFY(testFile.open(QIODevice::WriteOnly));

    src = QUrl::toLocalFile(tmpDirPath() + m_stashTestFile);

    testFile(src);

    QVERIFY(testFile.open(QIODevice::WriteOnly));
    stashFile(src, QUrl("/" + m_stashTestFile));

    tmpDir.mkDir(tmpDirPath() + m_fileTestFolder);

    stashDirectory("/" + m_stashTestFolder);
}

void SlaveTest::cleanupTestCase()
{
    //delete all test files
    stashDaemonProcess->terminate();
}

QString SlaveTest::tmpDirPath() const
{
    return QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)+ tmpFolder);
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

void SlaveTest::stashCopy(const QUrl &src, const QUrl &dest)
{
    KIO::Job *job = KIO::copyAs(src, dest, KIO::HideProgressInfo);
    bool ok = job->exec();
    QVERIFY(ok);
}

void SlaveTest::moveFromStash(const QUrl &src, const QUrl &dest) //make this work
{
    KIO::Job *job = KIO::moveAs(src, dest, KIO::HideProgressInfo);
    bool ok = job->exec();
    QVERIFY(ok);
    QVERIFY(QFile::exists(dest.toString()));
}

void SlaveTest::deleteFromStash(const QUrl &url)
{
    KIO::Job *delJob = KIO::del(url, KIO::HideProgressInfo);
    bool ok = delJob->exec();
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
    KIO::ListJob *job = KIO::listDir(QUrl(QStringLiteral("stash:/myfolder")), KIO::HideProgressInfo);
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
    QVERIFYstatUrl(url, entry));
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
    QVERIFY(!item.isWritable());
    QVERIFY(!item.isHidden());
    QCOMPARE(item.text(), QStringLiteral(m_stashTestFile);
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
    QVERIFY(!item.isWritable());
    QVERIFY(!item.isHidden());
    QCOMPARE(item.text(), QStringLiteral(m_stastTestFolder));
}

void SlaveTest::statSymlinkInRoot()
{
    QUrl url(QStringLiteral("stash:/" + m_stashTestSymlink));
    stashSymlink(url.path(), url.path());
    KIO::UDSEntry entry;
    QVERIFY(statUrl(url, entry));
    KFileItem item(entry, url);
    QVERIFY(!item.isFile());
    QVERIFY(!item.isDir());
    QVERIFY(item.isLink());
    QVERIFY(item.isReadable());
    QVERIFY(!item.isWritable());
    QVERIFY(!item.isHidden());
    QCOMPARE(item.text(), QStringLiteral(m_stashTestSymlink));
}

void SlaveTest::statFileInDirectory()
{
    QUrl url(QStringLiteral("stash:/" + m_stashTestFolder + m_stashTestFileInSubDirectory));
    stashFile(url.path(), url.path());
    KIO::UDSEntry entry;
    QVERIFY(statUrl(url, entry));
    KFileItem item(entry, url);
    QVERIFY(item.isFile());
    QVERIFY(!item.isDir());
    QVERIFY(!item.isLink());
    QVERIFY(item.isReadable());
    QVERIFY(!item.isWritable());
    QVERIFY(!item.isHidden());
    QCOMPARE(item.text(), QStringLiteral(m_stashTestFileInSubDirectory));
}

void SlaveTest::copyFileToStash()
{
    QUrl src = QUrl::fromLocalFile(tmpDirPath() + m_fileTestFile);
    QFile testFile(src);
    QVERIFY(testFile.open(QIODevice::WriteOnly));
    QUrl dest("stash:/" + m_fileTestFile);

    stashCopy(src, dest);
    QVERIFY(testFile.exists()); //use kio::stat
    QVERIFY(QFile(dest).exists());
/*
    QUrl destDirectory("stash:/copyTestCase");
    destinationFileName = QUrl(src).fileName();
    stashCopy(src, destDirectory);
    QVERIFY(stashFile.exists());
    QVERIFY(QFile(dest + destinationFileName).exists());*/
}

void SlaveTest::copySymlinkFromStash() //create test case
{
    stashSymlink(tmpDirPath + m_fileTestFile, "/" + m_stashTestSymlink)
    QUrl src("stash:/" + m_stashTestSymlink);
    QUrl dest = QUrl::fromLocalFile(tmpDirPath + m_stashTestSymlink);
    stashCopy(src, dest);
    QVERIFY(QFile::exists(dest));
}

void SlaveTest::copyStashToFile()
{
    QUrl src("stash:/" + m_stashTestFile);
    QUrl dest = QUrl::fromLocalFile(tmpDirPath() + m_fileTestFolder + "/" + m_stashTestFile);
    stashCopy(src, dest);
    QVERIFY(QFile::exists(dest));
}

void SlaveTest::copyStashToStash()
{
    QUrl src("stash:/" + m_stashTestFile);
    QUrl dest("stash:/" + m_stashTestFolder + "/" + m_stashTestFile);
    stashCopy(src, dest);
    QVERIFY(QFile(dest).exists()); //use kio::stat
}

void SlaveTest::moveToFileFromStash()
{
    QUrl src("stash:/" + m_stashTestFile);
    QUrl dest(QUrl::fromLocalFile(tmpDirPath() + m_stashTestFile));
    moveFromStash(src, dest);
    QVERIFY(QFile(dest.toLocalFile()).exists());
    QVERIFY(!QFile(src).exists()); //use kio::stat!
    //match properties also
}

void SlaveTest::moveToStashFromStash()
{
    QUrl src("stash:/" + m_stashTestFile);
    QUrl dest("stash:/" + m_stashTestFolder + "/" + m_stashTestFile);
    moveFromStash(src, dest);
    QVERIFY(QFile(dest).exists()); //use kio::stat
    QVERIFY(!QFile(src).exists()); //use kio::stat
}

void SlaveTest::delRootFile()
{
    QUrl url(""); //url
    deleteFromStash(url);
    QCOMPARE(QFile(url).exists(), false); //use kio::stat
}

void SlaveTest::delFileInDirectory()
{
    QUrl url(""); //url
    deleteFromStash(url);
    QCOMPARE(QFile(url).exists(), false); //use kio::stat
}

void SlaveTest::delDirectory()
{
    QUrl url("/deldir");
    deleteFromStash(url);
    QCOMPARE(QFile(url).exists(), false); //use kio::stat
}

QTEST_MAIN(SlaveTest)
#include "slavetest.moc"
