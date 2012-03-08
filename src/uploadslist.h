/***************************************************************************
 *   Copyright (C) 2005 by Niels Sandholt Busch                            *
 *   niels@diku.dk                                                         *
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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef UPLOADSSLIST_H
#define UPLOADSSLIST_H

#include <QTreeWidget>

/**
@author Niels Sandholt Busch
*/
class UploadsList : public QTreeWidget
{
Q_OBJECT

public:
    UploadsList(QWidget *parent = 0, const char *name = 0 );

    ~UploadsList();

    void contextMenuEvent( QContextMenuEvent* );

signals:
    void downloadCanceled( qint64 );
    void grabFriendRequested(QString fname,
                             unsigned long fsize,
                             QString fhash,
                             QString fhost);

protected slots:
    void cancelDownload();
    void grabFriend();
private:
    QTreeWidgetItem* listItem;

};

#endif
