#include <QCoreApplication>
#include <QtDBus/QtDBus>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QDBusMessage m;
    if (QString(argv[1]) == "-a") {
        m = QDBusMessage::createMethodCall("org.kde.StagingNotifier", "/StagingNotifier","","watchDir");
    } else if (QString(argv[1]) == "-d") {
        m = QDBusMessage::createMethodCall("org.kde.StagingNotifier", "/StagingNotifier","","removeDir");
    }
    m << argv[2];
    bool queued = QDBusConnection::sessionBus().send(m);
    return 0;
}

/*
#include <QTextStream>
#include <QDebug>
#include <string>
#include <QIODevice>
#include <QFileInfo>
#include <QFile>
QString filename("/tmp/staging-files");
QFile file(filename);
if (!file.open(QFile::WriteOnly | QFile::Text| QIODevice::Append)) {
    qDebug() << "I/O error";
    return -1;
}
QTextStream url(&file);
if (QFileInfo(QFile(argv[1])).exists()) {
    url << argv[1] << "\n";
    qDebug() << argv[1];
} else {
    qDebug() << argv[1] << " does not exist.";
}
file.flush();
file.close();
*/
