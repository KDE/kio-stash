#include "kio_staging.h"
#include <QDebug>
#include <QMimeType>
#include <QMimeDatabase>
#include <QDir>
#include <QUrl>
#include <QFile>
#include <QCoreApplication>

class KIOPluginForMetaData : public QObject
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.kio.slave.staging" FILE "staging.json")
};

extern "C" {
    int Q_DECL_EXPORT kdemain(int argc, char **argv)
    {
        qDebug() << "out";
        QCoreApplication app(argc, argv);
        Staging slave(argv[2], argv[3]);
        slave.dispatchLoop();
        return 0;
    }
}

Staging::Staging(const QByteArray &pool, const QByteArray &app) : KIO::ForwardingSlaveBase("staging", pool, app)
{
    qDebug() << "Finished crying";
}

bool Staging::rewriteUrl(const QUrl &url, QUrl &newUrl) //don't fuck around with this
{
    if (url.scheme() != QLatin1String("file")) {
        return false;
    }
    newUrl = url;
    return true;
}

void Staging::listDir(const QUrl &url)
{
    KIO::ForwardingSlaveBase::listDir(QUrl("file:///home/nic/gsoc-2016"));
}

void Staging::rename(const QUrl &src, const QUrl &dest, KIO::JobFlags flags)
{
    KIO::ForwardingSlaveBase::rename(src, dest, flags);
}

#include "kio_staging.moc"
