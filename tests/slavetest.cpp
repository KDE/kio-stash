#include <QTest>
#include "slavetest.h"
#include "../src/ioslave/ioslave.h"

#include <QString>
#include <QDir>
#include <QFileInfo>

#include <KIO/Job>
#include <KIO/CopyJob>
#include <KIO/DeleteJob>
#include <KJob>
#include <KFileItem>

void SlaveTest::initTestCase()
{

}

void SlaveTest::cleanupTestCase()
{

}

void SlaveTest::stashFile(const QString &path)
{

}

void SlaveTest::stashDirectory(const QString &path)
{

}

void SlaveTest::stashSymlink(const QString &path)
{

}

void SlaveTest::statUrl(const QUrl &url, KIO::UDSEntry &entry)
{
    KIO::StatJob *statJob = KIO::stat(url, KIO::HideProgressInfo);
    bool ok = statJob->exec();
    if (ok) {
        entry = statJob->statResult();
    }
    QVERIFY(ok);
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

}

void SlaveTest::listSubDir()
{

}

void SlaveTest::createDirectory() //find ways for finding files
{
    QUrl url("stash:/testfolder");
    KIO::SimpleJob *job = KIO::mkdir(url);
    bool ok = job->exec();
    QVERIFY(QFile::exists(url.toString()));
    QCOMPARE(ok, true);
}

void SlaveTest::statRoot()
{
    QUrl url(QStringLiteral("stash:/"));
    KIO::UDSEntry entry;
    statUrl(url, entry);
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
    QUrl url(QStringLiteral("stash:/stashfile"));
    stashFile(url.path());
    KIO::UDSEntry entry;
    statUrl(url, entry);
    KFileItem item(entry, url);
    QVERIFY(item.isFile());
    QVERIFY(!item.isDir());
    QVERIFY(!item.isLink());
    QVERIFY(item.isReadable());
    QVERIFY(!item.isWritable());
    QVERIFY(!item.isHidden());
    QCOMPARE(item.text(), QStringLiteral("stashfile"));
}

void SlaveTest::statDirectoryInRoot()
{
    QUrl url(QStringLiteral("stash:/stashfolder"));
    stashDirectory(url.path());
    KIO::UDSEntry entry;
    statUrl(url, entry);
    KFileItem item(entry, url);
    QVERIFY(!item.isFile());
    QVERIFY(item.isDir());
    QVERIFY(!item.isLink());
    QVERIFY(item.isReadable());
    QVERIFY(!item.isWritable());
    QVERIFY(!item.isHidden());
    QCOMPARE(item.text(), QStringLiteral("stashfolder"));
}

void SlaveTest::statSymlinkInRoot()
{
    QUrl url(QStringLiteral("stash:/stashsymlink"));
    stashSymlink(url.path());
    KIO::UDSEntry entry;
    statUrl(url, entry);
    KFileItem item(entry, url);
    QVERIFY(!item.isFile());
    QVERIFY(!item.isDir());
    QVERIFY(item.isLink());
    QVERIFY(item.isReadable());
    QVERIFY(!item.isWritable());
    QVERIFY(!item.isHidden());
    QCOMPARE(item.text(), QStringLiteral("stashsymlink"));
}

void SlaveTest::statFileInDirectory()
{
    QUrl url(QStringLiteral("stash:/stashtestfolder/testfile"));
    stashFile(url.path());
    KIO::UDSEntry entry;
    statUrl(url, entry);
    KFileItem item(entry, url);
    QVERIFY(item.isFile());
    QVERIFY(!item.isDir());
    QVERIFY(!item.isLink());
    QVERIFY(item.isReadable());
    QVERIFY(!item.isWritable());
    QVERIFY(!item.isHidden());
    QCOMPARE(item.text(), QStringLiteral("testfile"));

}

void SlaveTest::copyFileToStash()
{
    QUrl src("file:/");
    QUrl dest("stash:/");
    stashCopy(src, dest);
    QVERIFY(QFile(dest.toString()).exists());
    QCOMPARE(src.fileName(), dest.fileName());
}

void SlaveTest::copySymlinkFromStash() //create test case
{
    QUrl src("stash:/");
    QUrl dest("file:/");
    stashCopy(src, dest);
    QVERIFY(QFile(dest.toString()).exists());
    QCOMPARE(src.fileName(), dest.fileName());
}

void SlaveTest::copyStashToFile()
{
    QUrl src("stash:/");
    QUrl dest("file:/");
    stashCopy(src, dest);
    QVERIFY(QFile(dest.toString()).exists());
    QCOMPARE(src.fileName(), dest.fileName());
}

void SlaveTest::copyStashToStash()
{
    QUrl src("stash:/");
    QUrl dest("file:/");
    stashCopy(src, dest);
    QVERIFY(QFile(dest.toString()).exists());
    QCOMPARE(src.fileName(), dest.fileName());

}

void SlaveTest::moveToFileFromStash()
{
    QUrl src("stash:/");
    QUrl dest("file:/");
    moveFromStash(src, dest);
    QVERIFY(QFile(dest.toString()).exists());
    QVERIFY(!QFile(src.toString()).exists());
    //match properties also
}

void SlaveTest::moveToStashFromStash()
{
    QUrl src("stash:/");
    QUrl dest("stash:/");
    moveFromStash(src, dest);
    QVERIFY(QFile(dest.toString()).exists());
    QVERIFY(!QFile(src.toString()).exists());
}

void SlaveTest::delRootFile()
{
    QUrl url("");
    deleteFromStash(url);
    QCOMPARE(QFile(url.toString()).exists(), false);
}

void SlaveTest::delFileInDirectory()
{
    QUrl url("");
    deleteFromStash(url);
    QCOMPARE(QFile(url.toString()).exists(), false);
}

void SlaveTest::delDirectory()
{
    QUrl url("");
    deleteFromStash(url);
    QCOMPARE(QFile(url.toString()).exists(), false);
}

QTEST_MAIN(SlaveTest)
#include "slavetest.moc"
