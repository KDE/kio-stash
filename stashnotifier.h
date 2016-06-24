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

#ifndef STASHNOTIFIER_H
#define STASHNOTIFIER_H

#include <QtDBus>
#include <QStringList>

#include <kdedmodule.h>

class KDirWatch;

class StashNotifier : public KDEDModule
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.kio.StashNotifier")

private:
    KDirWatch *dirWatch;
    QStringList m_List;
    QString processString(const QString &path);

public:
    StashNotifier(QObject* parent, const QList<QVariant>&);
    ~StashNotifier();

Q_SIGNALS:
    Q_SCRIPTABLE void listChanged();

public Q_SLOTS:
    Q_SCRIPTABLE void addPath(const QString &path);
    Q_SCRIPTABLE void removePath(const QString &path);
    Q_SCRIPTABLE QStringList fileList();

private Q_SLOTS:
    void dirty(const QString &path);
    void created(const QString &path);
    void displayList(); //for internal purposes
};

#endif
