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
#ifndef DOWNLOADSTABLE_H
#define DOWNLOADSTABLE_H

#include <QTableView>
#include <QTreeView>
#include <QModelIndex>

#include "downloaddefs.h"

class QContextMenuEvent;
class Download;

/**
@author Niels Sandholt Busch
*/
class DownloadsTable : public QTreeView
{
Q_OBJECT
public:
    DownloadsTable(QWidget *parent = 0);

    ~DownloadsTable();

    void contextMenuEvent( QContextMenuEvent* );
    void stopAll();

    virtual void keyPressEvent(QKeyEvent *) ;

signals:
    void downloadCanceled( qint64 );
    void grabFriendRequested(QString fname,
                             unsigned long fsize,
                             QString fhash,
                             QString fhost);
    void downloadFileRequested( QString, unsigned long, QString, QString ,int,int,int,int);

protected slots:
    // menu slots.
    void resume();
    void pause();
    void stop();
    void cancel();
    void clearComplete();
    void grab();
    void copyLink();
    void pasteLink();
    void previewmedia();
    void openFile ();
    void openFolder ();
    void showDetailsDialog();
    // priority (sub)menu slots.
    void setPriorityHigh();
    void setPriorityNormal();
    void setPriorityLow();


private:
    enum Item
    {
        PAUSE = 0,
        STOP,
        RESUME,
        CANCEL,
        PREVIEW_MEDIA,
        OPEN_FILE,
        OPEN_FOLDER,
        DETAILS,
        CLEAR,
        COPY_LINK,
        PASTE_LINK,
        GRAB_FRIEND,
        NUMBER_OF_ITEMS
    };

    static const char* itemNames[NUMBER_OF_ITEMS];
    static const char* resourcePath[NUMBER_OF_ITEMS];
    static const char* slotName[NUMBER_OF_ITEMS];

    static const char* prioritySlotName[DownloadDefs::NUMBER_OF_PRIORITY];

private:
    void addAction(QMenu& menu, const QString& name,
        const QString& resource, const char* actionFun);

    void createContextMenu();
    bool itemSelected() const;
    void removeSelectedRows(bool checkCompleteOnly = false);
};

#endif
