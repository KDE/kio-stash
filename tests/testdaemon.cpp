#include "../src/iodaemon/stashnotifier.h"
#include <QCoreApplication>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    StashNotifier *stashDaemon = new StashNotifier(0, QVariantList());
    return app.exec();
}
