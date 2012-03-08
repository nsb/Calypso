/***************************************************************************
 *   Copyright (C) 2009 by defnax                                          *
 *                                                                         *
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

#include <QList>
#include <QtDebug>
#include <QContextMenuEvent>
#include <QMenu>
#include <QVBoxLayout>
#include <QIcon>
#include <QStandardItemModel>
#include <QModelIndex>
#include <QHeaderView>
#include <QDir>
#include <QFileInfo>

#include "kommute.h"
#include "kommutedefs.h"

#include "download.h"
#include "DLListDelegate.h"

#include "properities/DetailsDialog.h"

#include "kommuteutils.h"
#include "mutelayer.h"
#include "settings.h"
#include "misc.h"
#include "Utils.h"

extern class Settings *settings;

void Kommute::initDownloadsPage()
{

    DLListModel = new QStandardItemModel(0, DLListDelegate::NUMBER_OF_DONLOAD_COLUMNS);

    initDownloadList ();

    downloadsTreeView->setModel(DLListModel);

    // dobule click download item to pause / resume.
    connect(downloadsTreeView, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(changeDownloadState(const QModelIndex&)));

    DLDelegate = new DLListDelegate(downloadsTreeView);

    downloadsTreeView->setItemDelegate(DLDelegate);
    downloadsTreeView->hideColumn(DLListDelegate::PTR);
    downloadsTreeView->setSortingEnabled(true);
    downloadsTreeView->setRootIsDecorated(false);

    /* Set header resize modes and initial section sizes */
    QHeaderView * _dlheader = downloadsTreeView->header () ;
    const int resizeValueOfDLList[DLListDelegate::NUMBER_OF_DONLOAD_COLUMNS] =
        {20, 200, 60, 80, 60, 160, 80, 60 , 75, 60, 75, 80, 200, 60, 270, 20};

    for (int i = 0; i < DLListDelegate::NUMBER_OF_DONLOAD_COLUMNS; ++i)
    {
        if (i == DLListDelegate::SYMBOL)
        {
            _dlheader->setResizeMode (i, QHeaderView::Custom);
        }
        else
        {
            _dlheader->setResizeMode (i, QHeaderView::Interactive);
        }
        _dlheader->resizeSection (i, resizeValueOfDLList[i]);
    }

    connect (downloadsTreeView, SIGNAL (grabFriendRequested (QString,ulong,QString,QString)),
        this, SLOT (addFriendToSearchView (QString, unsigned long, QString, QString)));

    connect (downloadsTreeView, SIGNAL (downloadFileRequested (QString, unsigned long, QString, QString,int,int,int,int)),
        this, SLOT (downloadFileRequested (QString, unsigned long, QString, QString, int, int, int, int)));

    connect (downloadsTreeView, SIGNAL (clicked (const QModelIndex&)),
        this, SLOT (setTaskGraphPainterWidget (const QModelIndex&)));

    connect (uploadsList, SIGNAL(grabFriendRequested (QString, unsigned long, QString, QString)),
        this, SLOT (addFriendToSearchView (QString, unsigned long, QString, QString)));
}

void Kommute::retranslateDownloadList ()
{
    for (int i = 0; i < DLListModel->rowCount(); ++i)
    {
        Download *dl = (Download *) (DLListModel->data (DLListModel->index (i, DLListDelegate::PTR)).value <quintptr > ());

        QModelIndex priorityIndex = DLListModel->index(i, DLListDelegate::PRIORITY);
        DLListModel->setData (priorityIndex, tr (DownloadDefs::priorityNames[dl->getPriority ()]));

        QModelIndex statusIndex = DLListModel->index(i, DLListDelegate::STATUS);
        DLListModel->setData(statusIndex, tr(DownloadDefs::downloadStatusName[dl->getStatus ()]));

        dl->setRouteQuality (dl->getRouteQuality ());
    }
}

void Kommute::initDownloadList ()
{
    for (int i = 0; i < DLListDelegate::NUMBER_OF_DONLOAD_COLUMNS; ++i)
    {
        DLListModel->setHeaderData(i, Qt::Horizontal, DLListDelegate::tr(DLListDelegate::downloadColumnNames[i]));
    }
}

void Kommute::changeDownloadState(const QModelIndex& index)
{
    Download *dl = (Download *) (index.model ()->
        data (index.model ()->index (index.row (), DLListDelegate::PTR)).value <quintptr > ());

    if (dl->getStatus () == DownloadDefs::DOWNLOADSTATUS_PAUSED)
    {
        dl->setStatus (DownloadDefs::DOWNLOADSTATUS_DOWNLOADING);
    }
    else if (dl->getStatus () == DownloadDefs::DOWNLOADSTATUS_DOWNLOADING)
    {
        dl->setStatus (DownloadDefs::DOWNLOADSTATUS_PAUSED);
    }
}

void Kommute::setTaskGraphPainterWidget (const QModelIndex& index)
{
    Download *dl = (Download *) (index.model ()->
        data (index.model ()->index (index.row (), DLListDelegate::PTR)).value <quintptr > ());

    if (!dl)
        return;

    // call takeWidget befire setWidget to avoid destruction of the widget
    QWidget* lastWidget = taskGraphWidget->takeWidget ();

    QWidget* currentWidget = dl->getTaskGraphicWidget ();
    taskGraphWidget->setWidget (currentWidget);

    taskGraphWidget->show ();
}


void Kommute::downloadFileRequested(QString fname,
                                    unsigned long fsize,
                                    QString fhash,
                                    QString fhost,
                                    int firstChunk1,int lastChunk1,
                                    int firstChunk2,int lastChunk2)
{
    for (int i = 0; i < DLListModel->rowCount(); ++i)
    {
        if(DLListModel->data(DLListModel->index(i,DLListDelegate::HASH)).toString() == fhash)
        {
            fprintf(stderr,"file is already in download list\n");

            // add host+fname to sources
            Download *dl =
                (Download *)(DLListModel->data(DLListModel->index(i,DLListDelegate::PTR)).value<quintptr>());
            FileSource *newSource =
                new FileSource (strdup (fhost.toLocal8Bit().data()),
                strdup(fname.toLocal8Bit().data()));

            dl->getSources()->addSource (newSource);
            delete newSource;
            return;
        }
    }

    Download *dl = new Download( static_cast<QStandardItemModel*>(DLListModel),
                    fname,
                    (qulonglong) fsize,
                    firstChunk1 ,lastChunk1,
                    firstChunk2 ,lastChunk2,
                    fhash,
                    fhost );
    dl->start();

}

//void Kommute::hideEvent( QHideEvent *e )
//{
 //   emit visible(false);
//}

// void MultiView::cancelDownload( qint64 id )
// {
//     Download *dl = dlMap[id];
//     dl->cancel();
//     dl->delHash();
// }
char *DEFAULT_INCOMINGHASH_DIR ="MUTE_incoming_hashes";

void Kommute::reloadList()
{
    /* TODO : load list of previous downloads */
    // Mute internal hash put in temp folder which set manually.
    QString fullPath(settings->getTempDirectory());
    fullPath += "/";
    fullPath += DEFAULT_INCOMINGHASH_DIR;

    QDir dir;
    dir.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
    dir.setSorting(QDir::Time );
    if (!dir.cd(fullPath))
    {
      // create dir if not exists
      dir.mkdir(fullPath);
      return;
    }
    QFileInfoList list= dir.entryInfoList();
    QFileInfo fileInfo;
    for ( int i=0 ; i< list.size() ; ++i )
    {
       fileInfo = list.at(i);
       Download *dl = new Download( static_cast<QStandardItemModel*>(DLListModel),fileInfo);
       dl->start();
       /*
       QFile FileHash(fileInfo.absoluteFilePath());
       if ( !FileHash.open( QIODevice::ReadOnly))
       {
	       fprintf(stderr,"can't open file %s\n",fileInfo.absoluteFilePath().toLocal8Bit().data());
            return ;
       }
       QTextStream Stream ( & FileHash );
       QString fname,s_fsize,fhost;
       int firstChunk1;
       int firstChunk2;
       int lastChunk1;
       int lastChunk2;
       fname=Stream.readLine();
       s_fsize=Stream.readLine();
       fhost=Stream.readLine();
       Stream >>firstChunk1;
       Stream >>lastChunk1;
       Stream >>firstChunk2;
       Stream >>lastChunk2;
    unsigned long fsize=s_fsize.toULong();
       downloadFileRequested(fname,
		          fsize,
                          fileInfo.fileName(),
			  fhost
			  ,firstChunk1,lastChunk1
			  ,firstChunk2,lastChunk2
			  );
*/
    }
}

void Kommute::updateProgress(int value)
{
	for(int i = 0; i <= DLListModel->rowCount(); i++) {
		if(selection->isRowSelected(i, QModelIndex())) {
			editItem(i, DLListDelegate::PROGRESS, QVariant((double)value));
		}
	}
}

int Kommute::addItem(const QString& symbol, const QString& name, qlonglong fileSize,
    double progress, const QString& priority, double dlspeed,
    const QString& sources, const QString& status, const QString& routequality,
    const QString& host, const QString& hash, const QString& ptr,
    qlonglong completed, qlonglong remaining)
{
	int row;
	QString sl;
	//QIcon icon(symbol);
	//name.insert(0, " ");
	//sl.sprintf("%d / %d", seeds, leechs);
	row = DLListModel->rowCount();
	DLListModel->insertRow(row);

	//DLListModel->setData(DLListModel->index(row, SYMBOL), QVariant((QIcon)icon), Qt::DecorationRole);
	DLListModel->setData(DLListModel->index(row, DLListDelegate::NAME), QVariant((QString)name), Qt::DisplayRole);
	DLListModel->setData(DLListModel->index(row, DLListDelegate::SIZE), QVariant((qlonglong)fileSize));
	DLListModel->setData(DLListModel->index(row, DLListDelegate::PROGRESS), QVariant((double)progress));
  DLListModel->setData(DLListModel->index(row, DLListDelegate::PRIORITY), QVariant(priority));
	DLListModel->setData(DLListModel->index(row, DLListDelegate::DLSPEED), QVariant((double)dlspeed));
	DLListModel->setData(DLListModel->index(row, DLListDelegate::SOURCES), QVariant((QString)sources));
	DLListModel->setData(DLListModel->index(row, DLListDelegate::STATUS), QVariant((QString)status));
  DLListModel->setData(DLListModel->index(row, DLListDelegate::ROUTEQUALITY), QVariant((QString)routequality));
  DLListModel->setData(DLListModel->index(row, DLListDelegate::HOST), QVariant((QString)host));
  DLListModel->setData(DLListModel->index(row, DLListDelegate::HASH), QVariant((QString)hash));
  DLListModel->setData(DLListModel->index(row, DLListDelegate::PTR), QVariant((QString)ptr));
	DLListModel->setData(DLListModel->index(row, DLListDelegate::COMPLETED), QVariant((qlonglong)completed));
	DLListModel->setData(DLListModel->index(row, DLListDelegate::REMAINING), QVariant((qlonglong)remaining));
	return row;
}

void Kommute::delItem(int row)
{
	DLListModel->removeRow(row, QModelIndex());
}

void Kommute::editItem(int row, int column, QVariant data)
{
    if (DLListDelegate::SYMBOL == column || DLListDelegate::PTR == column)
        return;

    if (DLListDelegate::NAME == column)
    {
        DLListModel->setData(DLListModel->index(row, DLListDelegate::NAME), data, Qt::DisplayRole);
    }
    else
    {
        DLListModel->setData(DLListModel->index(row, column), data);
    }
}

double Kommute::getProgress(int row, QStandardItemModel *model)
{
    return model->data(model->index(row, DLListDelegate::PROGRESS), Qt::DisplayRole).toDouble();
}

double Kommute::getSpeed(int row, QStandardItemModel *model)
{
    return model->data(model->index(row, DLListDelegate::DLSPEED), Qt::DisplayRole).toDouble();
}

QString Kommute::getFileName(int row, QStandardItemModel *model)
{
    return model->data(model->index(row, DLListDelegate::NAME), Qt::DisplayRole).toString();
}

QString Kommute::getStatus(int row, QStandardItemModel *model)
{
    return model->data(model->index(row, DLListDelegate::STATUS), Qt::DisplayRole).toString();
}

qlonglong Kommute::getFileSize(int row, QStandardItemModel *model)
{
    bool ok = false;
    return model->data(model->index(row, DLListDelegate::SIZE), Qt::DisplayRole).toULongLong(&ok);
}

QString Kommute::getRouteQuality(int row, QStandardItemModel *model)
{
    return model->data(model->index(row, DLListDelegate::ROUTEQUALITY), Qt::DisplayRole).toString();
}

QString Kommute::getHost(int row, QStandardItemModel *model)
{
    return model->data(model->index(row, DLListDelegate::HOST), Qt::DisplayRole).toString();
}

QString Kommute::getHash(int row, QStandardItemModel *model)
{
    return model->data(model->index(row, DLListDelegate::HASH), Qt::DisplayRole).toString();
}

QString Kommute::getPtr(int row, QStandardItemModel *model)
{
    return model->data(model->index(row, DLListDelegate::PTR), Qt::DisplayRole).toString();
}

qlonglong Kommute::getTransfered(int row, QStandardItemModel *model)
{
    bool ok = false;
    return model->data(model->index(row, DLListDelegate::COMPLETED), Qt::DisplayRole).toULongLong(&ok);
}

qlonglong Kommute::getRemainingTime(int row, QStandardItemModel *model)
{
    bool ok = false;
    return model->data(model->index(row, DLListDelegate::REMAINING), Qt::DisplayRole).toULongLong(&ok);
}


// display Details of selected items
void Kommute::detailsSelection(){
    QModelIndexList selectedIndexes = downloadsTreeView->selectionModel()->selectedIndexes();
    QModelIndex index;

    foreach(index, selectedIndexes)
    {
        if(index.column() == DLListDelegate::NAME)
        {
            showDetails(index);
        }
    }
}

// Show File Details dialog
void Kommute::showDetails(const QModelIndex &index) {
  int row = index.row();
  QString hash = DLListModel->data(DLListModel->index(row, DLListDelegate::HASH)).toString();
  DetailsDialog *details = new DetailsDialog(this);
  details->show();
}

// return the row of in data model
int Kommute::getRowFromHash(QString hash) const{
    unsigned int nbRows = DLListModel->rowCount();

    for(unsigned int i=0; i<nbRows; ++i)
    {
        if(DLListModel->data(DLListModel->index(i, DLListDelegate::HASH)) == hash)
        {
            return i;
        }
    }
    return -1;
}

