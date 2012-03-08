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
#include "addlogevent.h"
#include "connectionevent.h"
#include "connectioncountevent.h"
#include "connectionattemptevent.h"
#include "download.h"
#include "DLListDelegate.h"
#include "ConnectionsListDelegate.h"
#include "SFListDelegate.h"
#include "sharedfilesevent.h"
#include "properities/DetailsDialog.h"

#include "uploadevent.h"
#include "uploadscountevent.h"
#include "uploadlistitem.h"
#include "uploadslist.h"
#include "kommuteutils.h"
#include "mutelayer.h"
#include "settings.h"
#include "misc.h"
#include "Utils.h"


void Kommute::customEvent(QEvent *e)
{
    int c;
    QModelIndex index;

    bool found = false;
    switch(e->type())
    {
        case KommuteDefs::EVENTTYPE_CLEARCONNECTIONS:
        c = ConnectionsListModel->rowCount();
        ConnectionsListModel->removeRows(0,c);
            break;
        case KommuteDefs::EVENTTYPE_CONNECTION:
            c = ConnectionsListModel->rowCount();
            ConnectionsListModel->insertRow(c);
            index = ConnectionsListModel->index(c, ConnectionsListDelegate::CICON);
            ConnectionsListModel->setData(index, QIcon(QString::fromUtf8(":/resources/contact0.png")), Qt::DecorationRole);
            if (static_cast<ConnectionEvent*>(e)->queuedCount > 0)
            {
            ConnectionsListModel->setData(index, QIcon(QString::fromUtf8(":/resources/contact2.png")), Qt::DecorationRole);
            }
            if (static_cast<ConnectionEvent*>(e)->queuedCount > 100)
            {
            ConnectionsListModel->setData(index, QIcon(QString::fromUtf8(":/resources/contact3.png")), Qt::DecorationRole);
            }
            index = ConnectionsListModel->index(c, ConnectionsListDelegate::ADDRESS);
            ConnectionsListModel->setData(index, static_cast<ConnectionEvent*>(e)->address);
            index = ConnectionsListModel->index(c, ConnectionsListDelegate::PORT);
            ConnectionsListModel->setData(index, static_cast<ConnectionEvent*>(e)->port);
            index = ConnectionsListModel->index(c, ConnectionsListDelegate::SENT);
            ConnectionsListModel->setData(index, static_cast<ConnectionEvent*>(e)->sentCount);
            index = ConnectionsListModel->index(c, ConnectionsListDelegate::QUEUED);
            ConnectionsListModel->setData(index, static_cast<ConnectionEvent*>(e)->queuedCount);
            index = ConnectionsListModel->index(c, ConnectionsListDelegate::DROPPED);
            ConnectionsListModel->setData(index, static_cast<ConnectionEvent*>(e)->droppedCount);
            break;
        case KommuteDefs::EVENTTYPE_CONNECTIONCOUNT:
            emit connectionCountChanged(static_cast<ConnectionCountEvent*>(e)->connectionCount);
            break;
        case KommuteDefs::EVENTTYPE_CONNECTIONATTEMPT:
            emit currentConnectionAttempt(static_cast<ConnectionAttemptEvent*>(e)->attemptAddress,
                                          static_cast<ConnectionAttemptEvent*>(e)->attemptPort );
            break;
        case KommuteDefs::EVENTTYPE_UPLOAD:
            for (QTreeWidgetItemIterator it(uploadsList); *it; ++it )
            {
                if(static_cast<UploadEvent*>(e)->uploadID == static_cast<UploadListItem*>(*it)->uploadID())
                {
                    // Update existing upload
                    found = true;
                    static_cast<UploadListItem*>(*it)->setProgress(static_cast<UploadEvent*>(e)->lastChunkSent,
                                                                       static_cast<UploadEvent*>(e)->chunksInFile,
                                                                       static_cast<UploadDefs::UploadStatus>(static_cast<UploadEvent*>(e)->status));
                    break;
                }
            }

            if(!found)
            {
                // Create new upload entry
                new UploadListItem( uploadsList,
                                    static_cast<UploadEvent*>(e)->filePath,
                                    static_cast<UploadEvent*>(e)->host,
                                    static_cast<UploadEvent*>(e)->uploadID );
            }

            break;
        case KommuteDefs::EVENTTYPE_UPLOADSCOUNT:
            emit numberOfUploadsChanged( static_cast<UploadsCountEvent*>(e)->uploadsCount );
            break;
        case KommuteDefs::EVENTTYPE_SHAREDFILES:
        {
            c = SFListModel->rowCount();
            SFListModel->insertRow(c);

            index = SFListModel->index(c, SFListDelegate::SFNAME);
            SFListModel->setData(index, static_cast<SharedFilesEvent*>(e)->fileName);

            index = SFListModel->index(c, SFListDelegate::SFSIZE);
            SFListModel->setData(index, static_cast<unsigned long long int>(static_cast<SharedFilesEvent*>(e)->fileLength));

            index = SFListModel->index(c, SFListDelegate::SFHASH);
            SFListModel->setData(index, static_cast<SharedFilesEvent*>(e)->fileHash);

            index = SFListModel->index(c, SFListDelegate::SFFULLPATH);
            SFListModel->setData(index, static_cast<SharedFilesEvent*>(e)->filePath);

            QString ext = QFileInfo(static_cast<SharedFilesEvent*>(e)->fileName).suffix();

            KommuteDefs::FileType fileType = misc::fileType(ext);

            SFListModel->setData(SFListModel->index(c,SFListDelegate::SFNAME),
                QIcon(KommuteDefs::FileTypeResourcePath[fileType]), Qt::DecorationRole);
            SFListModel->setData(SFListModel->index(c,SFListDelegate::SFTYPE),
                KommuteDefs::tr(KommuteDefs::FileTypeName[fileType]));

            break;
        }
        case KommuteDefs::EVENTTYPE_ADDLOG:
            setLogInfo( static_cast<AddLogEvent*>(e)->logText);
            break;
        default:
            break;
    }

}


//void Kommute::hideEvent( QHideEvent *e )
//{
 //   emit visible(false);
//}


