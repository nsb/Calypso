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
#ifndef SEARCHRESULTSTABLE_H
#define SEARCHRESULTSTABLE_H

#include <QTableView>
#include <QTreeView>
#include <QString>
#include <QModelIndex>

class QContextMenuEvent;

/**
@author Niels Sandholt Busch
*/
/*
class TreeItem
{
public:
    TreeItem(const QList<QVariant> &data, TreeItem *parent = 0);
    ~TreeItem();

    void appendChild(TreeItem *child);

    TreeItem *child(int row);
    int childCount() const;
    int columnCount() const;
    QVariant data(int column) const;
    int row() const;
    TreeItem *parent();

private:
    QList<TreeItem*> childItems;
    QList<QVariant> itemData;
    TreeItem *parentItem;
};

*/
class SearchResultsTable : public QTreeView
{
Q_OBJECT
public:
    SearchResultsTable(QWidget *parent = 0);

    ~SearchResultsTable();

    void contextMenuEvent( QContextMenuEvent* );

signals:
    void grabFriendRequested(QString fname,
                               unsigned long fsize,
                               QString fhash,
                               QString fhost);
    void downloadItemsRequested();

protected slots:
    void download();
    void grabFriend();
    void copyDownloadLink();
private:
    enum Item
    {
        DOWNLOAD = 0,
        COPY_LINK,
        GRAB_FRIEND,
        NUMBER_OF_ITEMS
    };

    static const char* itemNames[NUMBER_OF_ITEMS];
    static const char* resourcePath[NUMBER_OF_ITEMS];
    static const char* slotName[NUMBER_OF_ITEMS];
};

#endif
