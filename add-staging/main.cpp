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

