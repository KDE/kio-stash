#include <QCoreApplication>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QIODevice>
#include <QFileInfo>
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
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
    return 0;
}

