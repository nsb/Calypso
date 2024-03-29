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

#ifndef DLLISTDELEGATE_H
#define DLLISTDELEGATE_H

#include <QAbstractItemDelegate>

class QModelIndex;
class QPainter;
class QStyleOptionProgressBarV2;
class QProgressBar;
class QApplication;


class DLListDelegate: public QAbstractItemDelegate {

    Q_OBJECT

    public:
        DLListDelegate(QObject *parent=0);
        ~DLListDelegate();
        void paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const;
        QSize sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index) const;

    public:
        // Defines for download list list columns
        enum DownloadColumn
        {
            SYMBOL = 0,
            NAME,
            SIZE,
            COMPLETED,
            DLSPEED,
            PROGRESS,
            PRIORITY,
            SOURCES,
            STATUS,
            ROUTEQUALITY,
            REMAINING,
            DONE,
            HOST,
            CHUNKS,
            HASH,
            PTR,
            NUMBER_OF_DONLOAD_COLUMNS
        };

        static const char* downloadColumnNames[NUMBER_OF_DONLOAD_COLUMNS];

    private:

    public slots:

    signals:
};
#endif

