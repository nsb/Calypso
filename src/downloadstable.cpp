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
#include <QHeaderView>
#include <QClipboard>
#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QMessageBox>
#include <QDesktopServices>
#include <QUrl>

#include "downloadstable.h"
#include "download.h"
#include "DLListDelegate.h"
#include "KommuteLinkAnalyzer.h"
#include "settings.h"
#include "properities/DetailsDialog.h"
#include "misc.h"

extern class Settings *settings;

const char* DownloadsTable::itemNames[NUMBER_OF_ITEMS] =
{
    QT_TR_NOOP ("Pause"),
    QT_TR_NOOP ("Stop"),
    QT_TR_NOOP ("Resume"),
    QT_TR_NOOP ("Cancel"),           // add separator behind
    QT_TR_NOOP ("Open file"),
    QT_TR_NOOP ("Preview"),
    QT_TR_NOOP ("Open folder"),
    QT_TR_NOOP ("Details..."),      // add separator behind
    QT_TR_NOOP ("Clear Completed"),  // add separator behind
    QT_TR_NOOP ("Copy link"),
    QT_TR_NOOP ("Paste link"),       // add separator behind
    QT_TR_NOOP ("Grab friend")
};

const char* DownloadsTable::resourcePath[NUMBER_OF_ITEMS] =
{
    ":/resources/pause.png",
    ":/resources/stop.png",
    ":/resources/start.png",
    ":/resources/delete.png",
    ":/resources/fileopen.png",
    ":/resources/preview.png",
    ":/resources/folderopen.png",
    ":/resources/fileinfo.png",
    ":/resources/deleteall.png",
    ":/resources/kmlink.png",
    ":/resources/kmlinkpaste.png",
    ":/resources/grabfriend.png"
};

const char* DownloadsTable::slotName[NUMBER_OF_ITEMS] =
{
    SLOT (pause ()),
    SLOT (stop ()),
    SLOT (resume ()),
    SLOT (cancel ()),
    SLOT (openFile ()),
    SLOT (previewmedia ()),
    SLOT (openFolder ()),
    SLOT (showDetailsDialog ()),
    SLOT (clearComplete ()),
    SLOT (copyLink ()),
    SLOT (pasteLink ()),
    SLOT (grab ())
};

const char* DownloadsTable::prioritySlotName[DownloadDefs::NUMBER_OF_PRIORITY] =
{
    SLOT(setPriorityHigh()),
    SLOT(setPriorityNormal()),
    SLOT(setPriorityLow()),
    SLOT(setPriorityNormal())
};

static const char* PRIORITY_MENU = QT_TRANSLATE_NOOP("DownloadsTable", "Priority (Download)");
static const char* PRIORITY_RESOURCE_PATH = ":/resources/filepriority.png";

DownloadsTable::DownloadsTable (QWidget * parent):QTreeView (parent)
{
  //verticalHeader ()->hide ();
}

DownloadsTable::~DownloadsTable ()
{
}

void DownloadsTable::addAction(QMenu& menu, const QString& name, const QString& resource, const char* slot)
{
    QAction *action = menu.addAction (name);
    action->setIcon (QIcon (resource));
    connect (action, SIGNAL (triggered ()), this, slot);
}

void DownloadsTable::createContextMenu()
{
    QMenu popMenu (this);
    QMenu* subMenu = popMenu.addMenu(QIcon(PRIORITY_RESOURCE_PATH), tr(PRIORITY_MENU));

    for (int i = 0; i < DownloadDefs::NUMBER_OF_PRIORITY; ++i)
    {
        addAction(*subMenu, DownloadDefs::tr(DownloadDefs::priorityNames[i]), DownloadDefs::priorityResourcePath[i], prioritySlotName[i]);
    }

    popMenu.addSeparator();

    for (int i = 0; i < NUMBER_OF_ITEMS; ++i)
    {
        addAction(popMenu, tr(itemNames[i]), resourcePath[i], slotName[i]);

        if (CANCEL == i || DETAILS == i || CLEAR == i || PASTE_LINK == i)
        {
            popMenu.addSeparator ();
        }
    }

    popMenu.setMouseTracking (true);
    popMenu.exec (QCursor::pos ());

}

bool DownloadsTable::itemSelected() const
{
    QModelIndexList lst = selectedIndexes ();
    return (lst.count() > 0);
}

void DownloadsTable::contextMenuEvent (QContextMenuEvent * e)
{
   createContextMenu();
}

void DownloadsTable::previewmedia ()
{
    QModelIndexList lst = selectedIndexes ();

    for (int i = 0; i < lst.count (); i++)
    {
        if (lst[i].column () == 0)
        {
            QModelIndex & ind = lst[i];
            QString fname= ind.model ()->data (ind.model ()->index (ind.row (), DLListDelegate::NAME)).toString() ;
            
            QString extension = fname.split(QString::fromUtf8(".")).last().toUpper();
          
            /* check if the file is a media file */
            if (!misc::isPreviewable(extension))
            {
          		QMessageBox::warning(this, tr("Preview impossible"),tr("Sorry, we can't preview this file"),
              QMessageBox::Ok, QMessageBox::Ok); 
              return;
            }    

            if (!QFile::exists( settings->getTempDirectory() + "/" + fname))
                continue;

            fname = settings->getTempDirectory() + "/" + fname;
              QString fullPath = QString ("file:///%1").arg (fname);
              QDesktopServices::openUrl(fullPath);
        }
    }
}

void DownloadsTable::openFile ()
{
    QModelIndexList lst = selectedIndexes ();

    for (int i = 0; i < lst.count (); i++)
    {
        if (lst[i].column () == 0)
        {
            QModelIndex & ind = lst[i];
            QString fname= ind.model ()->data (ind.model ()->index (ind.row (), DLListDelegate::NAME)).toString() ;

    	    if (!QFile::exists( settings->getTempDirectory() + "/" + fname))
                continue;

    		fname = settings->getTempDirectory() + "/" + fname;
            QString fullPath = QString ("file:///%1").arg (fname);
            QDesktopServices::openUrl(fullPath);
        }
    }
}

void DownloadsTable::openFolder ()
{
/*
    QModelIndexList lst = selectedIndexes ();

    for (int i = 0; i < lst.count (); i++)
    {
        if (lst[i].column () == 0)
        {
            QModelIndex & ind = lst[i];
            QString fname = ind.model ()->data (ind.model ()->index (ind.row (), DLListDelegate::NAME)).toString() ;

    	    if( QFile::exists( settings->getTempDirectory () + "/" + fname))
            {
                QString fullPath = QString ("file:///%1").arg (settings->getTempDirectory ());
                QDesktopServices::openUrl(fullPath);
            }
        }
    }
*/
    QString fullPath = QString ("file:///%1").arg (settings->getTempDirectory ());
    QDesktopServices::openUrl(fullPath);
}

void DownloadsTable::setPriorityHigh()
{
    QModelIndexList lst = selectedIndexes ();

    for (int i = 0; i < lst.count (); i++)
    {
        if (lst[i].column () == 0)
        {
            QModelIndex& ind = lst[i];
            Download* dl = (Download *) (ind.model ()->
                data (ind.model ()->index (ind.row (), DLListDelegate::PTR)).value <quintptr > ());
            dl->setPriority(DownloadDefs::HIGH);
        }
    }
}

void DownloadsTable::setPriorityNormal()
{
    QModelIndexList lst = selectedIndexes ();

    for (int i = 0; i < lst.count (); i++)
    {
        if (lst[i].column () == 0)
        {
            QModelIndex & ind = lst[i];
            Download *dl = (Download *) (ind.model ()->
                data (ind.model ()->index (ind.row (), DLListDelegate::PTR)).value <quintptr > ());
            dl->setPriority(DownloadDefs::NORMAL);
        }
    }
}

void DownloadsTable::setPriorityLow()
{
    QModelIndexList lst = selectedIndexes ();

    for (int i = 0; i < lst.count (); i++)
    {
        if (lst[i].column () == 0)
        {
            QModelIndex & ind = lst[i];
            Download *dl = (Download *) (ind.model ()->
                data (ind.model ()->index (ind.row (), DLListDelegate::PTR)).value <quintptr > ());
            dl->setPriority(DownloadDefs::LOW);
        }
    }
}

void DownloadsTable::stopAll ()
{
    selectAll ();

    QModelIndexList lst = selectedIndexes ();

    for (int i = 0; i < lst.count (); i++)
    {
        if (lst[i].column () == 0)
        {
            QModelIndex & ind = lst[i];
            Download *dl = (Download *) (ind.model ()->
                data (ind.model ()->index (ind.row (), DLListDelegate::PTR)).value <quintptr > ());
            dl->stop ();
            delete dl;
        }
    }
    removeSelectedRows();
}

void DownloadsTable::stop()
{
    QModelIndexList lst = selectedIndexes ();

    for (int i = 0; i < lst.count (); i++)
    {
        if (lst[i].column () == 0)
        {
            QModelIndex & ind = lst[i];
            Download *dl = (Download *) (ind.model ()->
                data (ind.model ()->index (ind.row (), DLListDelegate::PTR)).value <quintptr > ());
            dl->stop();
        }
    }
}

void DownloadsTable::cancel ()
{
    QMessageBox::StandardButton ret = QMessageBox::question(this, tr("Kommute"), tr("Are you sure that you want to cancel and delete"
        "selected file(s)?"),
        QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel);

    if (QMessageBox::Cancel == ret)
        return;

    QModelIndexList lst = selectedIndexes ();

    for (int i = 0; i < lst.count (); i++)
    {
        if (lst[i].column () == 0)
        {
            QModelIndex & ind = lst[i];
            Download *dl = (Download *) (ind.model ()->
                data (ind.model ()->index (ind.row (), DLListDelegate::PTR)).value <quintptr > ());
            dl->cancel ();
	    delete dl;
        }
    }
    removeSelectedRows();
}

void DownloadsTable::resume ()
{
    QModelIndexList lst = selectedIndexes ();
    for (int i = 0; i < lst.count (); ++i)
    {
        if (lst[i].column () == 0)
        {
            QModelIndex & ind = lst[i];
            Download *dl =
                (Download *)(ind.model ()->data (ind.model ()->index (ind.row (), DLListDelegate::PTR)).value <quintptr > ());
            printf ("resume %d:%s\n", i,
            dl->fileName ().toLocal8Bit ().data ());
            dl->resume();
        }
    }
}

void DownloadsTable::pause()
{
    QModelIndexList lst = selectedIndexes();
    for (int i = 0; i < lst.count(); ++i)
    {
        if (lst[i].column() == 0)
        {
            QModelIndex &ind = lst[i];
            Download *dl =
                (Download *)(ind.model()->data(ind.model()->index(ind.row(), DLListDelegate::PTR)).value<quintptr>());
            printf ("pause %d:%s\n", i,
            dl->fileName().toLocal8Bit().data());
            dl->pause();
        }
    }
}

void DownloadsTable::removeSelectedRows(bool checkCompleteOnly)
{
    QModelIndexList lst = selectedIndexes();
    int selectedRows = lst.count();

    if (selectedRows <= 0)
        return;

    for (int i = selectedRows - 1; i >= 0; --i)
    {
        if (lst[i].column () == 0)
        {
            QModelIndex &index = lst[i];

            // check complete files only means remove complete rows.
            if (checkCompleteOnly)
            {
                Download *dl =
                    (Download *)(index.model()->data(index.model()->index(index.row(), DLListDelegate::PTR)).value<quintptr>());
                if (DownloadDefs::DOWNLOADSTATUS_FILETRANSFERCOMPLETE != dl->getStatus ())
                {
                    continue;
                }
            }
            model ()->removeRow (index.row ());
        }
    }
}

void DownloadsTable::clearComplete ()
{
    selectAll ();

    bool doesClearComplete = true;
    removeSelectedRows (doesClearComplete);

    clearSelection ();
}

void DownloadsTable::grab ()
{
    QModelIndexList lst = selectedIndexes ();

    for (int i = 0; i < lst.count (); i++)
    {
        if (lst[i].column () == 0)
        {
            QModelIndex & ind = lst[i];
            Download *dl = (Download *) (ind.model ()->
                data (ind.model ()->index (ind.row (), DLListDelegate::PTR)).value < quintptr > ());
            QString fname = dl->fileName ();
            unsigned long fsize = dl->getSize ();
            QString fhash = dl->getHash ();
            QString fhost = dl->getHost ();
            emit grabFriendRequested (fname, fsize, fhash, fhost);
        }
    }
}

void DownloadsTable::copyLink ()
{
    QModelIndexList lst = selectedIndexes ();
    KommuteLinkAnalyzer analyzer;

    for (int i = 0; i < lst.count (); i++)
    {
        if ( lst[i].column() == 0 )
        {
            QModelIndex & ind = lst[i];
            QString fhash= ind.model ()->data (ind.model ()->index (ind.row (), DLListDelegate::HASH )).toString() ;
            QString fsize= ind.model ()->data (ind.model ()->index (ind.row (), DLListDelegate::SIZE)).toString() ;
            QString fname= ind.model ()->data (ind.model ()->index (ind.row (), DLListDelegate::NAME)).toString() ;

            analyzer.setKommuteLink (fname, fsize, fhash);
        }
    }

    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(analyzer.getKommuteLink ());
}

void DownloadsTable::pasteLink ()
{
    QClipboard *clipboard = QApplication::clipboard();
    KommuteLinkAnalyzer analyzer (clipboard->text ());

    if (!analyzer.isValid ())
        return;

    QVector<KommuteLinkData> linkList;
    analyzer.getFileInformation (linkList);

    for (int i = 0, n = linkList.size (); i < n; ++i)
    {
        const KommuteLinkData& linkData = linkList[i];
        downloadFileRequested(linkData.getName (), linkData.getSize ().toInt (),
            linkData.getHash (), "", -1, -1, -1, -1);
    }
}

void DownloadsTable::showDetailsDialog()
{
    static DetailsDialog *detailsdlg = new DetailsDialog();
    
    QModelIndexList lst = selectedIndexes ();
    KommuteLinkAnalyzer analyzer;

    for (int i = 0; i < lst.count (); i++)
    {
        if (lst[i].column () == 0)
        {
            QModelIndex& ind = lst[i];
            QString fhash = ind.model ()->data (ind.model ()->index (ind.row (), DLListDelegate::HASH )).toString() ;
            QString fsize = ind.model ()->data (ind.model ()->index (ind.row (), DLListDelegate::SIZE )).toString() ;
            QString fname = ind.model ()->data (ind.model ()->index (ind.row (), DLListDelegate::NAME )).toString() ;
            QString fstatus = ind.model ()->data (ind.model ()->index (ind.row (), DLListDelegate::STATUS)).toString() ;
            QString fpriority = ind.model ()->data (ind.model ()->index (ind.row (), DLListDelegate::PRIORITY)).toString() ;
            QString fsources = ind.model ()->data (ind.model ()->index (ind.row (), DLListDelegate::SOURCES)).toString() ;
            QString fchunks = ind.model ()->data (ind.model ()->index (ind.row (), DLListDelegate::CHUNKS)).toString() ;
            
            qulonglong filesize = ind.model ()->data (ind.model ()->index (ind.row (), DLListDelegate::SIZE)).toULongLong() ;
            double fdatarate = ind.model ()->data (ind.model ()->index (ind.row (), DLListDelegate::DLSPEED)).toDouble() ;            
            qulonglong fcompleted = ind.model ()->data (ind.model ()->index (ind.row (), DLListDelegate::COMPLETED)).toULongLong() ;
            qulonglong fremaining = ind.model ()->data (ind.model ()->index (ind.row (), DLListDelegate::REMAINING)).toULongLong() ;
            
            int fprogress = ind.model ()->data (ind.model ()->index (ind.row (), DLListDelegate::DONE)).toInt() ;
            
            qulonglong fremainingsize = filesize - fcompleted;
            
            // Set Details.. Window Title
            detailsdlg->setWindowTitle(tr("Details: ") + fname);

            // General GroupBox
            detailsdlg->setHash(fhash);
            detailsdlg->setFileName(fname);
            detailsdlg->setSize(filesize);
            detailsdlg->setStatus(fstatus);
            detailsdlg->setPriority(fpriority);
            detailsdlg->setType(QFileInfo(fname).suffix());
            
            // Transfer GroupBox
            detailsdlg->setSources(fsources);
            detailsdlg->setDatarate(fdatarate);
            detailsdlg->setCompleted(fcompleted);
            detailsdlg->setRemaining(fremaining);
            detailsdlg->setRemainingSize(fremainingsize);
            detailsdlg->setChunks(fchunks);
            detailsdlg->setProgress(fprogress);
            
            // kommute link(s) Tab
            analyzer.setKommuteLink (fname, fsize, fhash);
            detailsdlg->setLink(analyzer.getKommuteLink ());
            
            detailsdlg->show();
            break;
        }
    }
}

void DownloadsTable::keyPressEvent(QKeyEvent *e)
{
	if(e->key() == Qt::Key_Delete)
	{
		cancel() ;
		e->accept() ;
	}
	else
		QTreeView::keyPressEvent(e) ;
}
