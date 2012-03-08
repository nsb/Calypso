/****************************************************************
 *  kommute is distributed under the following license:
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

#ifndef SFLISTDELEGATE_H
#define SFLISTDELEGATE_H

#include <QAbstractItemDelegate>

class QModelIndex;
class QPainter;
class QStyleOptionProgressBarV2;
class QProgressBar;
class QApplication;


class SFListDelegate: public QAbstractItemDelegate {

    Q_OBJECT

    public:
        SFListDelegate(QObject *parent=0);
        ~SFListDelegate();
        void paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const;
        QSize sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index) const;

    public:
        // Defines for Shared Files list columns
        enum SFColumn
        {
            SFNAME = 0,
            SFSIZE,
            SFTYPE,
            SFHASH,
            SFFULLPATH,
            NUMBER_OF_SFCOLUMNS,
        };

        static const char* SFColumnNames[NUMBER_OF_SFCOLUMNS];
    private:

    public slots:

    signals:
};
#endif

