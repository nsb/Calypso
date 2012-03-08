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
#include <QTreeView>
#include <QHeaderView>
#include <QClipboard>
#include <QApplication>
#include <QDebug>
#include <QDesktopServices>

#include "SFListDelegate.h"
#include "hashthread.h"
#include "sharedfileslist.h"
#include "properities/CommentsDialog.h"
#include "KommuteLinkAnalyzer.h"
#include "settings.h"
#include "Utils.h"

extern class Settings *settings;

const char* SharedFilesList::itemNames[NUMBER_OF_ITEMS] =
{
    QT_TR_NOOP("Open File"),
    QT_TR_NOOP("Open Folder"),
    QT_TR_NOOP("Copy Link"),
    QT_TR_NOOP("Comments..."),
};

const char* SharedFilesList::resourcePath[NUMBER_OF_ITEMS] =
{
    ":/resources/fileopen.png",
    ":/resources/folderopen.png",
    ":/resources/kmlink.png",       // add seperator
    ":/resources/filecomments.png"
};

const char* SharedFilesList::slotName[NUMBER_OF_ITEMS] =
{
    SLOT(openFile()),
    SLOT(openFolder()),
    SLOT(copyDLink()),
    SLOT(showCommentsDialog()),
};

SharedFilesList::SharedFilesList (QWidget * parent):QTreeView (parent)
{
}

SharedFilesList::~SharedFilesList ()
{
}

void SharedFilesList::addAction(QMenu& menu, const QString& name, const QString& resource, const char* slot)
{
    QAction *action = menu.addAction (name);
    action->setIcon (QIcon (resource));
    connect (action, SIGNAL (triggered ()), this, slot);
}

void SharedFilesList::createContextMenu()
{
    QMenu popMenu (this);

    for (int i = 0; i < NUMBER_OF_ITEMS; ++i)
    {
        addAction(popMenu, tr(itemNames[i]), resourcePath[i], slotName[i]);

        if (COPY_LINK == i)
            popMenu.addSeparator ();
    }

    popMenu.setMouseTracking (true);
    popMenu.exec (QCursor::pos ());

}

void SharedFilesList::contextMenuEvent (QContextMenuEvent * e)
{
    createContextMenu ();
}

void SharedFilesList::openFile()
{
    QModelIndexList lst = selectedIndexes ();

    for (int i = 0; i < lst.count (); i++)
    {
        if (lst[i].column () == 0)
        {
            QModelIndex & ind = lst[i];
            QString path= ind.model ()->data (ind.model ()->index (ind.row (), SFListDelegate::SFFULLPATH )).toString() ;
            QString name= ind.model ()->data (ind.model ()->index (ind.row (), SFListDelegate::SFNAME)).toString() ;
            QString fullPath = QString ("file:///%1%2").arg (path).arg (name);
            QDesktopServices::openUrl(fullPath);
        }
    }
}

void SharedFilesList::openFolder()
{
    QModelIndexList lst = selectedIndexes ();

    for (int i = 0; i < lst.count (); i++)
    {
        if (lst[i].column () == 0)
        {
            QModelIndex & ind = lst[i];
            QString path= ind.model ()->data (ind.model ()->index (ind.row (), SFListDelegate::SFFULLPATH )).toString() ;
            QString name= ind.model ()->data (ind.model ()->index (ind.row (), SFListDelegate::SFNAME)).toString() ;
            QString fullPath = QString ("file:///%1%2").arg (path).arg ("/");
            QDesktopServices::openUrl(fullPath);
        }
    }
}

void SharedFilesList::showCommentsDialog()
{
    static CommentsDialog *commentsdlg = new CommentsDialog();
    QModelIndexList lst = selectedIndexes ();
    KommuteLinkAnalyzer analyzer;

    for (int i = 0; i < lst.count (); i++)
    {
        if (lst[i].column () == 0)
        {
            QModelIndex& ind = lst[i];
            QString hash = ind.model ()->data (ind.model ()->index (ind.row (), SFListDelegate::SFHASH)).toString() ;
            QString size= ind.model ()->data (ind.model ()->index (ind.row (), SFListDelegate::SFSIZE )).toString() ;
            QString fname= ind.model ()->data (ind.model ()->index (ind.row (), SFListDelegate::SFNAME )).toString() ;

            SHASH *Share = getShare(hash.toLatin1().data());
            commentsdlg->setComm(*Share->comment);
            
            // kommute link(s) Tab
            analyzer.setKommuteLink (fname, size, hash);
            commentsdlg->setLink(analyzer.getKommuteLink ());
            
            commentsdlg->exec();
            *(Share->comment)=commentsdlg->getComm();
            break;
        }
    }
}

void SharedFilesList::copyDLink ()
{
    QModelIndexList lst = selectedIndexes ();
    KommuteLinkAnalyzer analyzer;

    for (int i = 0; i < lst.count (); i++)
    {
        if (lst[i].column () == 0)
        {
            QModelIndex & ind = lst[i];
            QString hash= ind.model ()->data (ind.model ()->index (ind.row (), SFListDelegate::SFHASH )).toString() ;
            QString size= ind.model ()->data (ind.model ()->index (ind.row (), SFListDelegate::SFSIZE )).toString() ;
            QString fname= ind.model ()->data (ind.model ()->index (ind.row (), SFListDelegate::SFNAME )).toString() ;

            analyzer.setKommuteLink (fname, size, hash);
        }
    }

    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(analyzer.getKommuteLink ());
}
