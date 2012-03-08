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

#include <QContextMenuEvent>
#include <QMenu>
#include <QAction>
#include <QTableView>
#include <QTreeView>
#include <QModelIndex>
#include <QAbstractTableModel>
#include <QVariant>
#include <QString>
#include <QHeaderView>
#include <QClipboard>
#include <QApplication>

#include "searchresultstable.h"
#include "searchresultfiltermodel.h"
#include "KommuteLinkAnalyzer.h"

const char* SearchResultsTable::itemNames[NUMBER_OF_ITEMS] =
{
    QT_TR_NOOP("Download"),
    QT_TR_NOOP("Copy Link"),                // add separator behind
    QT_TR_NOOP("Grab Friend"),              // add separator behind
};

const char* SearchResultsTable::resourcePath[NUMBER_OF_ITEMS] =
{
    ":/resources/start.png",
    ":/resources/kmlink.png",
    ":/resources/grabfriend.png"
};

const char* SearchResultsTable::slotName[NUMBER_OF_ITEMS] =
{
    SLOT(download()),
    SLOT(copyDownloadLink()),
    SLOT(grabFriend())
};

SearchResultsTable::SearchResultsTable (QWidget * parent)
:QTreeView (parent)
{
  //verticalHeader ()->hide ();
}

SearchResultsTable::~SearchResultsTable ()
{
}

void
SearchResultsTable::contextMenuEvent (QContextMenuEvent * e)
{
    QMenu popMenu (this);

    for (int i = 0; i < NUMBER_OF_ITEMS; ++i)
    {
        QAction* action = popMenu.addAction (tr(SearchResultsTable::itemNames[i]));
        action->setIcon (QIcon (SearchResultsTable::resourcePath[i]));
        connect (action, SIGNAL (triggered ()), SearchResultsTable::slotName[i]);

        if (i != NUMBER_OF_ITEMS - 1)
        {
            popMenu.addSeparator ();
        }
    }

    popMenu.setMouseTracking (true);
    popMenu.exec (QCursor::pos ());
}

void SearchResultsTable::grabFriend ()
{
    QModelIndexList lst = selectedIndexes ();
    QModelIndex & ind = lst[0];

    for (int i = 0; i < lst.count (); i++)
    {
        if (lst[i].column () == 0)
        {
            ind = lst[i];
            QString fname = ind.sibling (ind.row (), SearchResultFilterModel::SEARCH_NAME).data ().toString ();
            unsigned long fsize = static_cast <unsigned long>(ind.sibling (ind.row (),
                    SearchResultFilterModel::SEARCH_SIZE).data ().toULongLong ());
            QString fhash = ind.sibling (ind.row (), SearchResultFilterModel::SEARCH_HASH).data ().toString ();
            QString fhost = ind.sibling (ind.row (), SearchResultFilterModel::SEARCH_HOSTHASH).data ().toString ();

            emit grabFriendRequested (fname, fsize, fhash, fhost);
        }
    }
}

void SearchResultsTable::download ()
{
  emit downloadItemsRequested();
}

void SearchResultsTable::copyDownloadLink(){

    QModelIndexList lst = selectedIndexes ();
    KommuteLinkAnalyzer analyzer;

    for (int i = 0; i < lst.count (); ++i)
    {
        if ( lst[i].column() == 0 )
        {
            QModelIndex & ind = lst[i];
            QString fhash= ind.model ()->data (ind.model ()->index (ind.row (),
                SearchResultFilterModel::SEARCH_HASH)).toString() ;
            QString fsize= ind.model ()->data (ind.model ()->index (ind.row (),
                SearchResultFilterModel::SEARCH_SIZE)).toString() ;
            QString fname= ind.model ()->data (ind.model ()->index (ind.row (),
                SearchResultFilterModel::SEARCH_NAME)).toString() ;

            analyzer.setKommuteLink (fname, fsize, fhash);
        }
    }

    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(analyzer.getKommuteLink ());
}
