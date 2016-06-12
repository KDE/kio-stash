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

 #ifndef STAGINGNOTIFIER_H
 #define STAGINGNOTIFIER_H

 #include <kdedmodule.h>
 #include <QtDBus>
 #include <QList>
 #include <QUrl>

 class KDirWatch;

 class StagingNotifier : public KDEDModule
 {
     Q_OBJECT
     Q_CLASSINFO("D-Bus Interface", "org.kde.StagingNotifier")

 private:
     KDirWatch *dirWatch;
     QList<QUrl> m_List;
     void updateList();
     void loadUrlList();

 public:
     StagingNotifier(QObject* parent, const QList<QVariant>&);

 public slots:
     Q_SCRIPTABLE Q_NOREPLY void watchDir(const QString &path);

 private slots:
     void dirty(const QString &path);
     void deleted(const QString &path);
     void created(const QString &path);
 };

 #endif
