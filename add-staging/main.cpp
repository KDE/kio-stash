/***************************************************************************
*   Copyright (C) 2016 by Arnav Dhamija <arnav.dhamija@gmail.com>         *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program; if not, write to the                         *
*   Free Software Foundation, Inc.,                                       *
*   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA            *
***************************************************************************/

//Driver application for KDED of the GSoC 2016 File Staging project for Dolphin

#include <QCoreApplication>
#include <QtDBus/QtDBus>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QDBusInterface *interface = new QDBusInterface("org.kde.kio.StashNotifier", "/StashNotifier","",QDBusConnection::sessionBus());
    QList<QVariant> args;
    args.append(argv[1]);
    args.append(argv[2]);
    args.append(QString(argv[3]).toInt());
    interface->callWithArgumentList(QDBus::NoBlock, "addPath", args);

    /*if (QString(argv[1]) == "-a") {
        m = QDBusMessage::createMethodCall("org.kde.kio.StashNotifier", "/StashNotifier","","addPath");
        m << argv[2] << argv[3] << argv[4];
        bool queued = QDBusConnection::sessionBus().send(m);
    } else if (QString(argv[1]) == "-d") {
        m = QDBusMessage::createMethodCall("org.kde.kio.StashNotifier", "/StashNotifier","","removePath");
        m << argv[2];
        bool queued = QDBusConnection::sessionBus().send(m);
    }*/

    return 0;
}
