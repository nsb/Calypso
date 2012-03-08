/****************************************************************
 *  Kommute is distributed under the following license:
 *
 *  Copyright (C) 2008 defnax
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA  02110-1301, USA.
 ****************************************************************/

#ifndef CONNECTIONSLISTDELEGATE_H
#define CONNECTIONSLISTDELEGATE_H

#include <QAbstractItemDelegate>

class QModelIndex;
class QPainter;
class QStyleOptionProgressBarV2;
class QProgressBar;
class QApplication;

class ConnectionsListDelegate: public QAbstractItemDelegate {

    Q_OBJECT

    public:
        ConnectionsListDelegate(QObject *parent=0);
        ~ConnectionsListDelegate();
        void paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const;
        QSize sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index) const;

    public:
        // Defines for Connections list list columns
        enum ConnectionColumn
        {
            CICON = 0,
            ADDRESS,
            PORT,
            SENT,
            QUEUED,
            DROPPED,
            NUMBER_OF_CONNECTION_COLUMNS
        };

        static const char* connectionColumnNames[NUMBER_OF_CONNECTION_COLUMNS];

    private:

    public slots:

    signals:
};
#endif

