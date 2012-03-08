/****************************************************************
 *  Kommute is distributed under the following license:
 *
 *  Copyright (C) 2008, defnax
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

#include "kommute.h"
#include "sharedfileslist.h"
#include "SFListDelegate.h"
#include "settings.h"
#include "kommutedefs.h"
#include "misc.h"

#include <QAction>
#include <QTreeView>
#include <QList>
#include <QtDebug>
#include <QContextMenuEvent>
#include <QMenu>
#include <QCursor>
#include <QPoint>
#include <QMouseEvent>
#include <QPixmap>
#include <QHeaderView>
#include <QModelIndex>
#include <QStandardItemModel>
#include <QDesktopServices>
#include "Utils.h"

extern class Settings *settings;

/** Constructor */
void Kommute::initSharedFilesPage()
{
    SFListModel = new QStandardItemModel(0, SFListDelegate::NUMBER_OF_SFCOLUMNS);

    initSFList ();

    initSFTreeWidget ();

    //sharedfilesTreeView->setModel(SFListModel);
    SFDelegate = new SFListDelegate(sharedfilesTreeView);
    sharedfilesTreeView->setItemDelegate(SFDelegate);

    proxyModel = new QSortFilterProxyModel;
    proxyModel->setDynamicSortFilter(true);
    proxyModel->setSourceModel(SFListModel);
    sharedfilesTreeView->setModel(proxyModel);
    sharedfilesTreeView->setSortingEnabled(true);
    sharedfilesTreeView->setRootIsDecorated(false);
    sharedfilesTreeView->hideColumn(SFListDelegate::SFFULLPATH);

    /* Set header resize modes and initial section sizes */
    QHeaderView * _sfheader = sharedfilesTreeView->header();
    _sfheader->resizeSection ( SFListDelegate::SFNAME, 270 );
    _sfheader->resizeSection ( SFListDelegate::SFSIZE, 75 );
    _sfheader->resizeSection ( SFListDelegate::SFTYPE, 75 );
    _sfheader->resizeSection ( SFListDelegate::SFHASH, 240 );

    connect(filterlineEdit, SIGNAL(textChanged(const QString &)),this, SLOT(filterRegExpChanged()));
    connect(clearfilterButton, SIGNAL(clicked()),this, SLOT(clearfilter()));
    
    connect( filterlineEdit, SIGNAL( textChanged(const QString &)), this, SLOT(toggleclearButton()));

    connect(sharedfilesTreeView, SIGNAL(doubleClicked(const QModelIndex&)),
        this, SLOT(openFile(const QModelIndex&)));

    connect(sharedfilesTreeWidget, SIGNAL(itemPressed(QTreeWidgetItem*, int)),
        this, SLOT(fileListRegExpChanged(QTreeWidgetItem*, int)));
        
    clearfilterButton->hide();    

#if defined(Q_WS_WIN)

#endif

#if defined(Q_WS_X11)

#endif
}

void Kommute::initSFList ()
{
    for (int i = 0; i < SFListDelegate::NUMBER_OF_SFCOLUMNS; ++i)
    {
        SFListModel->setHeaderData(i, Qt::Horizontal, SFListDelegate::tr(SFListDelegate::SFColumnNames[i]));
    }
}

void Kommute::initSFTreeWidget ()
{
    QStringList pathList = settings->getOtherSharedPathSetting();

    qDebug () << "list size: " << pathList.count ();
    
    sharedfilesTreeWidget->topLevelItem(0)->child(0)->setData (0, Qt::UserRole, settings->getTempDirectory ());

    sharedfilesTreeWidget->topLevelItem(0)->child(1)->setData (0, Qt::UserRole, settings->getIncomingDirectory ());

    for (int i = 0; i < pathList.count (); ++i)
    {
        QTreeWidgetItem* children = new QTreeWidgetItem(sharedfilesTreeWidget->topLevelItem(0)->child(2));
        children->setText (0, pathList [i]);
        children->setData (0, Qt::UserRole, pathList [i]);
        sharedfilesTreeWidget->topLevelItem(0)->child(2)->addChild (children);
    }
}

void Kommute::retranslateSFList ()
{
    for (int i = 0; i < SFListModel->rowCount (); ++i)
    {
        QString fileExt = QFileInfo (SFListModel->data (SFListModel->index (i, SFListDelegate::SFNAME)).toString ()).suffix ();
        QModelIndex fileTypeIndex = SFListModel->index (i, SFListDelegate::SFTYPE);
        SFListModel->setData(fileTypeIndex, KommuteDefs::tr (KommuteDefs::FileTypeName[misc::fileType (fileExt)]));
    }
}

int Kommute::addSFItem(QString sfsymbol, QString sfname, qlonglong sffileSize, QString sfhash)
{
    int row;
    //QIcon icon(sfsymbol);
    name.insert(0, " ");
    row = SFListModel->rowCount();
    SFListModel->insertRow(row);

    //SFListModel->setData(SFListModel->index(row, SFICON), QVariant((QString)sfsymbol), Qt::DecorationRole);
    SFListModel->setData(SFListModel->index(row, SFListDelegate::SFNAME), QVariant((QString)sfname), Qt::DisplayRole);
    SFListModel->setData(SFListModel->index(row, SFListDelegate::SFSIZE), QVariant((qlonglong)sffileSize));
    SFListModel->setData(SFListModel->index(row, SFListDelegate::SFHASH), QVariant((QString)sfhash));
    return row;
}

void Kommute::delSFItem(int row)
{
    SFListModel->removeRow(row, QModelIndex());
}

void Kommute::editSFItem(int row, int column, QVariant data)
{
    switch(column) {
        case SFListDelegate::SFNAME:
            SFListModel->setData(SFListModel->index(row, SFListDelegate::SFNAME), data, Qt::DisplayRole);
            break;
        case SFListDelegate::SFSIZE:
            SFListModel->setData(SFListModel->index(row, SFListDelegate::SFSIZE), data);
            break;
        case SFListDelegate::SFHASH:
            SFListModel->setData(SFListModel->index(row, SFListDelegate::SFHASH), data);
            break;
    }
}

QString Kommute::getSFFileName(int row, QStandardItemModel *model)
{
    return model->data(model->index(row, SFListDelegate::SFNAME), Qt::DisplayRole).toString();
}

qlonglong Kommute::getSFFileSize(int row, QStandardItemModel *model)
{
    bool ok = false;
    return model->data(model->index(row, SFListDelegate::SFSIZE), Qt::DisplayRole).toULongLong(&ok);
}

QString Kommute::getSFHash(int row, QStandardItemModel *model)
{
    return model->data(model->index(row, SFListDelegate::SFHASH), Qt::DisplayRole).toString();
}

void Kommute::filterRegExpChanged()
{
    proxyModel->setFilterKeyColumn (SFListDelegate::SFNAME);
    QRegExp regExp(filterlineEdit->text(), Qt::CaseInsensitive);
    proxyModel->setFilterRegExp(regExp);
}

void Kommute::clearfilter()
{
    filterlineEdit->clear();
    filterlineEdit->setFocus();
}

void Kommute::openFile (const QModelIndex & index)
{
    const QString& name = index.model ()->data (index.model ()->index (index.row (), SFListDelegate::SFFULLPATH)).toString ();
    QString fullPath = QString ("file:///%1").arg (name);
    QDesktopServices::openUrl (fullPath);
}

void Kommute::fileListRegExpChanged(QTreeWidgetItem * item, int column)
{
    proxyModel->setFilterKeyColumn (SFListDelegate::SFFULLPATH);

    QString path = QDir (item->data (0, Qt::UserRole).toString ()).absolutePath ();

    qDebug () << "filter:" << path;

    QRegExp regExp(path, Qt::CaseSensitive, QRegExp::FixedString);
    proxyModel->setFilterFixedString (path);
}

/* toggle clearButton */
void Kommute::toggleclearButton()
{
    QString text = filterlineEdit->text();
    
    if (text.isEmpty())
    {
     clearfilterButton->hide();
    }
    else
    {
      clearfilterButton->show();
    }
    
}