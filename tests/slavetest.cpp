 #include <QTest>

 #include "ioslave.h"

 #include <QString>
 #include <QDir>
 #include <QFileInfo>

bool SlaveTest::statUrl(const QUrl &url, KIO::UDSEntry &entry)
{
    KIO::StatJob *statJob = KIO::stat(url, KIO::HideProgressInfo);
    bool ok = statJob->exec();
    if (ok) {
        entry = statJob->statResult();
    }
}

bool SlaveTest::stashCopy(const QUrl &src, const QUrl &dest)
{
    KIO::CopyJob *job = 0;
    job = KIO::copy(src, dest, KIO::HideProgressInfo);
    bool ok = job->exec();
    QVERIFY(ok);
    return ok;
}

bool SlaveTest::moveFromStash(const QUrl &src, const QUrl &dest) //make this work
{
    QUrl src
    KIO::Job *job = KIO::moveAs(src, dest, KIO::HideProgressInfo);
    bool ok = job->exec();
    QVERIFY(ok);
    QVERIFY(QFile::exists(dest));
    return ok;
}

void SlaveTest::statRoot()
{
    QUrl url(QStringLiteral("stash:/"));
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
    QUrl url(QStringLiteral("stash:/stashfile"));
    stashFile(url.path());
    KIO::UDSEntry entry;
    QVERIFY(statUrl(url, entry));
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
    stashFolder(url.path());
    KIO::UDSEntry entry;
    QVERIFY(statUrl(url, entry));
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
    QVERIFY(statUrl(url, entry));
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
    QVERIFY(statUrl(url, entry));
    KFileItem item(entry, url);
    QVERIFY(item.isFile());
    QVERIFY(!item.isDir());
    QVERIFY(!item.isLink());
    QVERIFY(item.isReadable());
    QVERIFY(!item.isWritable());
    QVERIFY(!item.isHidden());
    QCOMPARE(item.text(), QStringLiteral("testfile"));

}
