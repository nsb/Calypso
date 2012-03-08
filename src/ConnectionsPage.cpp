/***************************************************************************
 *   Copyright (C) 2008 by defnax                                          *
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

#include <QFileInfo>

#include <QList>
#include <QtDebug>
#include <QContextMenuEvent>
#include <QMenu>
#include <QAction>
#include <QVBoxLayout>
#include <QIcon>
#include <QStandardItemModel>
#include <QModelIndex>
#include <QDir>
#include <QFileInfo>
#include <QMessageBox>

#include <QContextMenuEvent>
#include <QMenu>
#include <QCursor>
#include <QPoint>
#include <QMouseEvent>
#include <QPixmap>
#include <QHeaderView>

#include "kommute.h"
#include "kommutedefs.h"
#include "connectionevent.h"
#include "connectioncountevent.h"
#include "connectionattemptevent.h"
#include "connect/ConnectDialog.h"
#include "ConnectionsListDelegate.h"
#include "download.h"
#include "uploadevent.h"
#include "uploadscountevent.h"
#include "uploadlistitem.h"
#include "uploadslist.h"
#include "kommuteutils.h"
#include "kommuteversion.h"
#include "mutelayer.h"
#include "MUTE/layers/messageRouting/messageRouter.h"

/** Constructor */
void Kommute::initConnectionsPage()
{
    
    ConnectionsListModel = new QStandardItemModel(0, ConnectionsListDelegate::NUMBER_OF_CONNECTION_COLUMNS);

    initConnectionList ();

    connectionsTreeView->setModel(ConnectionsListModel);
    CLDelegate = new ConnectionsListDelegate(connectionsTreeView);
    connectionsTreeView->setItemDelegate(CLDelegate);

    connectionsTreeView->setSortingEnabled(true);
    connectionsTreeView->setRootIsDecorated(false);
    
    /* Set header resize modes and initial section sizes */
    //QHeaderView * _cheader = connectionsTreeView->header();
    //const int resizeValue[ConnectionsListDelegate::NUMBER_OF_CONNECTION_COLUMNS] = {20, 100, 100, 100, 100, 100};

    /*_cheader->setResizeMode ( 0, QHeaderView::Custom);
    for (int i = 0; i < ConnectionsListDelegate::NUMBER_OF_CONNECTION_COLUMNS; ++i)
    {
        _cheader->resizeSection (i, resizeValue[i]);
    }*/


    /* Set header resize modes and initial section sizes */
    QHeaderView * _cheader = connectionsTreeView->header();
    _cheader->setResizeMode ( 0, QHeaderView::Custom);
    _cheader->resizeSection ( 0, 20 );
    _cheader->resizeSection ( 1, 100 );
    _cheader->resizeSection ( 2, 100 );
    _cheader->resizeSection ( 3, 100 );
    _cheader->resizeSection ( 4, 100 );
    _cheader->resizeSection ( 5, 100 );
    

    connect (addpushButton,SIGNAL(clicked()),this,SLOT(addHostcp()));
    connect(infoLog, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(displayInfoLogMenu(const QPoint&)));
    connect(connectionsTreeView, SIGNAL( customContextMenuRequested( QPoint ) ), this, SLOT( connectionstreeViewCostumPopupMenu( QPoint ) ) );

     // Set Log infos
    setLogInfo(tr("Kommute %1 started.", "e.g: Kommute v0.x started.").arg(kommuteVersion()));

    setLogInfo(tr("Welcome to Kommute Anonymous FileSharing."), QString::fromUtf8("blue"));

     

    /* Hide platform specific features */
#ifdef Q_WS_WIN

#endif
}

void Kommute::connectionstreeViewCostumPopupMenu( QPoint point )
{

    QMenu contextMnu( this );
    QMouseEvent *mevent = new QMouseEvent( QEvent::MouseButtonPress, point, Qt::RightButton, Qt::RightButton, Qt::NoModifier );
    addhostAct = new QAction(QIcon(":/resources/addhost.png"), tr( "Add Host" ), this );
    connect( addhostAct , SIGNAL( triggered() ), this, SLOT( showConnectDialog() ) );


    contextMnu.clear();
    contextMnu.addAction( addhostAct);

    contextMnu.exec( mevent->globalPos() );
}

void Kommute::initConnectionList ()
{
    for (int i = 0; i < ConnectionsListDelegate::NUMBER_OF_CONNECTION_COLUMNS; ++i)
    {
        ConnectionsListModel->setHeaderData(i, Qt::Horizontal, ConnectionsListDelegate::tr(ConnectionsListDelegate::connectionColumnNames[i]));
    }
}

int Kommute::addCLItem(QString cicon, QString address, QString port, qlonglong sent, QString queued, QString dropped )
{
    int row;
    QString sl;
    //QIcon icon(cicon);
    name.insert(0, " ");
    //sl.sprintf("%d / %d", seeds, leechs);
    row = ConnectionsListModel->rowCount();
    ConnectionsListModel->insertRow(row);

    //ConnectionsListModel->setData(ConnectionsListModel->index(row, CICON), QVariant((QIcon)icon), Qt::DecorationRole);
    ConnectionsListModel->setData(ConnectionsListModel->index(row, ConnectionsListDelegate::ADDRESS), QVariant((QString)address));
    ConnectionsListModel->setData(ConnectionsListModel->index(row, ConnectionsListDelegate::PORT), QVariant((QString)port));
    ConnectionsListModel->setData(ConnectionsListModel->index(row, ConnectionsListDelegate::SENT), QVariant((qlonglong)sent));
    ConnectionsListModel->setData(ConnectionsListModel->index(row, ConnectionsListDelegate::QUEUED), QVariant((QString)queued));
    ConnectionsListModel->setData(ConnectionsListModel->index(row, ConnectionsListDelegate::DROPPED), QVariant((QString)dropped));
    return row;
}

void Kommute::delCLItem(int row)
{
    ConnectionsListModel->removeRow(row, QModelIndex());
}

void Kommute::editCLItem(int row, int column, QVariant data)
{
    if (column == ConnectionsListDelegate::CICON)
        return;

    ConnectionsListModel->setData(ConnectionsListModel->index(row, column), data);
}

/*
void ConnectionsPage::customEvent(QEvent *e)
{
    int c;
    QModelIndex index;

    bool found = false;
    switch(e->type())
    {
        case EVENTTYPE_CLEARCONNECTIONS:
	    c = ConnectionsListModel->rowCount();
	    ConnectionsListModel->removeRows(0,c);
            break;
        case EVENTTYPE_CONNECTION:
            c = ConnectionsListModel->rowCount();
            ConnectionsListModel->insertRow(c);
            index = ConnectionsListModel->index(c, 0);
            ConnectionsListModel->setData(index, QIcon(QString::fromUtf8(":/resources/connect_to_network.png")), Qt::DecorationRole);
            index = ConnectionsListModel->index(c, 1);
            ConnectionsListModel->setData(index, static_cast<ConnectionEvent*>(e)->address);
            index = ConnectionsListModel->index(c, 2);
            ConnectionsListModel->setData(index, static_cast<ConnectionEvent*>(e)->port);
            index = ConnectionsListModel->index(c, 3);
            ConnectionsListModel->setData(index, static_cast<ConnectionEvent*>(e)->sentCount);
            index = ConnectionsListModel->index(c, 4);
            ConnectionsListModel->setData(index, static_cast<ConnectionEvent*>(e)->queuedCount);
            index = ConnectionsListModel->index(c, 5);
            ConnectionsListModel->setData(index, static_cast<ConnectionEvent*>(e)->droppedCount);
            break;
        case EVENTTYPE_CONNECTIONCOUNT:
            emit connectionCountChanged(static_cast<ConnectionCountEvent*>(e)->connectionCount);
            break;
        case EVENTTYPE_CONNECTIONATTEMPT:
            emit currentConnectionAttempt(static_cast<ConnectionAttemptEvent*>(e)->attemptAddress,
                                          static_cast<ConnectionAttemptEvent*>(e)->attemptPort );
            break;
        default:
            break;
    }

}
*/

// Update Log Info information
#define LOGMAXLINES 200

void Kommute::setLogInfo(QString info, QColor color) {
    static unsigned int nbLines = 0;
    ++nbLines;
    
    // Check log size, clear it if too big
    if(nbLines > LOGMAXLINES ) 
    {
        QString oldText=infoLog->toHtml();

        // remove 5 lines
        int posFirst = oldText.indexOf("<p ");
        int toRemove = oldText.indexOf("</p>", posFirst+5)-posFirst+4;
        toRemove = oldText.indexOf("</p>", posFirst+toRemove)-posFirst+4;
        toRemove = oldText.indexOf("</p>", posFirst+toRemove)-posFirst+4;
        toRemove = oldText.indexOf("</p>", posFirst+toRemove)-posFirst+4;
        toRemove = oldText.indexOf("</p>", posFirst+toRemove)-posFirst+4;
        oldText.remove(posFirst,toRemove);
        nbLines -= 5 ;
        infoLog->setHtml(oldText);
    }
    infoLog->append(QString::fromUtf8("<font color='grey'>")+ QTime::currentTime().toString(QString::fromUtf8("hh:mm:ss")) + QString::fromUtf8("</font> - <font color='") + color.name() +QString::fromUtf8("'><i>") + info + QString::fromUtf8("</i></font>"));
    infoLog->moveCursor(QTextCursor::End);
}

void Kommute::on_actionClearLog_triggered() {
    infoLog->clear();
}

void Kommute::displayInfoLogMenu(const QPoint& pos) {
    // Log Menu
    QMenu myLogMenu(this);
    myLogMenu.addAction(actionClearLog);
    // XXX: Why mapToGlobal() is not enough?
    myLogMenu.exec(mapToGlobal(pos)+QPoint(20,450));
}

/** Shows Connect Dialog */
void Kommute::showConnectDialog()
{
    static ConnectDialog *connectdialog = new ConnectDialog(this);
    connectdialog->show();
}

void Kommute::showConnectionAttempt( QString attemptAddress, int attemptPort )
{
    setLogInfo( tr("Trying to connect to ") + attemptAddress + ":" + QString::number( attemptPort ) + "...", 5000 );
}


void Kommute::addHostcp()
{
    QString text = addhostlineEdit->text();
    
    if(text.isEmpty())
    {
        /* error message */
        QMessageBox::warning(this, tr("Kommute"),
                   tr("Please Add a IP or Address"),
                   QMessageBox::Ok, QMessageBox::Ok);
                   
        return; //Don't Add a empty Host!!
    }

    printf("add host %s to host list\n",addhostlineEdit->text().toLatin1().data());
    setLogInfo(tr("Add Host ")+addhostlineEdit->text()+":"+ QString::number(portspinBox->value() ) +tr(" to host list."),5000);
    muteAddHost( addhostlineEdit->text().toLatin1().data() , portspinBox->value());

    addhostlineEdit->clear();
}
