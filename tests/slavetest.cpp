 #include <QTest>

 #include "ioslave.h"

 #include <QString>
 #include <QDir>
 #include <QFileInfo>

 void SlaveTest::statRoot()
 {
     QUrl url(QStringLiteral("stash:/"));
     KIO::UDSEntry entry;
     KIO::StatJob *statJob = KIO::stat(url, KIO::HideProgressInfo);
     bool ok = statJob->exec();
     if (ok) {
         entry = statJob->statResult();
     }
     QVERIFY(item.isDir());
     QVERIFY(!item.isLink());
     QVERIFY(item.isReadable());
     QVERIFY(item.isWritable());
     QVERIFY(!item.isHidden());
     QCOMPARE(item.name(), QStringLiteral("."));
 }
