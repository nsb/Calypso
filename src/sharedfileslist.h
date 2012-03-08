/***************************************************************************
 *   Copyright (C) 2006 by Niels Sandholt Busch                            *
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
#ifndef SHAREDFILESLIST_H
#define SHAREDFILESLIST_H

#include <QTreeView>
#include <QModelIndex>

class QContextMenuEvent;

/**
@author Niels Sandholt Busch
*/
class SharedFilesList : public QTreeView
{
Q_OBJECT
public:
    SharedFilesList(QWidget *parent = 0);

    ~SharedFilesList();

    void contextMenuEvent( QContextMenuEvent* );

signals:


protected slots:
    void openFile ();
    void openFolder ();
    void copyDLink ();
    void showCommentsDialog();

private:
    void addAction(QMenu& menu, const QString& name, const QString& resource, const char* slot);
    void createContextMenu();

private:
    enum Item
    {
        OPEN_FILE = 0,
        OPEN_FOLDER,
        COPY_LINK,
        COMMENT_DIALOG,
        NUMBER_OF_ITEMS
    };

    static const char* itemNames[NUMBER_OF_ITEMS];
    static const char* resourcePath[NUMBER_OF_ITEMS];
    static const char* slotName[NUMBER_OF_ITEMS];
};

#endif
