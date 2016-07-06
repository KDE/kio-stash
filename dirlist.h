#ifndef DIRLIST_H
#define DIRLIST_H

#include <QtCore>
#include <QDBusArgument>
//Convert to class idk?
namespace dirListDBus { // FIXME: add the streaming operators here
    struct dirList
    {
        QString filePath;
        QString source;
        int type;
        dirList()
        {
        }
        ~dirList()
        {
        }
        dirList(const dirList &obj)
        {
            filePath = obj.filePath;
            source = obj.source;
            type = obj.type;
        }
    };
}

#endif
