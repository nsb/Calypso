/***************************************************************************
 *   Copyright (C) 2005 by Niels Sandholt Busch   *
 *   niels@diku.dk   *
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
#include "uploadslist.h"
#include "uploadlistitem.h"
#include "uploadsthread.h"

#include <QMenu>
#include <QCursor>
#include <QContextMenuEvent>
#include <QHeaderView>

UploadsList::UploadsList(QWidget *parent, const char *name)
 : QTreeWidget(parent)
{
    headerItem()->setText(0, tr("File Name"));
    headerItem()->setText(1, tr("Host"));
    headerItem()->setText(2, tr("Progress"));
    headerItem()->setText(3, tr("Status"));

    /* Set header resize modes and initial section sizes */
    QHeaderView * _uheader = header();
    _uheader->resizeSection ( 0, 200 );
    _uheader->resizeSection ( 1, 250 );
    _uheader->resizeSection ( 2, 160 );
    _uheader->resizeSection ( 3, 100 );

    setAlternatingRowColors(false);
    setSortingEnabled(true);
    setRootIsDecorated(false);
}


UploadsList::~UploadsList()
{
/*    for (QTreeWidgetItemIterator it(this); *it; ++it )
        delete *it;*/
}

void UploadsList::contextMenuEvent( QContextMenuEvent* e)
{
    listItem = itemAt(e->pos());

    if( listItem != 0 )
    {
        QMenu popMenu(this);
        QAction *actionDownload = popMenu.addAction(tr("Cancel"));
        actionDownload->setIcon(QIcon(":/delete.png"));
        connect(actionDownload, SIGNAL(triggered()), this, SLOT(cancelDownload()));

        popMenu.addSeparator ();

        QAction *actionGrab = popMenu.addAction(tr("Grab Friend"));
        actionGrab->setIcon(QIcon(":/grabfriend.png"));
        connect(actionGrab, SIGNAL(triggered()), this, SLOT(grabFriend()));
        setCurrentItem(listItem);
        popMenu.setMouseTracking(true);
        popMenu.exec(QCursor::pos());
    }
}

void UploadsList::cancelDownload( )
{
}

void UploadsList::grabFriend( )
{
    QString fname = listItem->text(0);
    QString fhost = listItem->text(1);
    unsigned long fsize = 0;
    QString fhash = "";
    emit grabFriendRequested(fname, fsize, fhash, fhost);
}
