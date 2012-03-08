/***************************************************************************
 *   Copyright (C) 2005 by Niels Sandholt Busch   *
 *   niels@diku.dk   *
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
#ifndef DOWNLOAD_H
#define DOWNLOAD_H

#include <QString>
#include <QObject>
#include <QPersistentModelIndex>
#include <QTime>
#include <QFileInfo>
#include <QThread>
#include "xprogressbar.h"
#include "MUTE/fileShare.h"
#include "downloaddefs.h"
#include "taskGraphPainterWidget.h"

class QEvent;
class QStandardItemModel;
class xProgressBar;

namespace DownloadQueue {
// An item in the download queue can be in four different states related to the queue:
// Delayed, Queued, Active, Inactive
enum state { DELAYED, QUEUED, ACTIVE, INACTIVE };
}

/**
@author Niels Sandholt Busch
*/
class Download : public QObject
{
    friend class DownloadThread;
    friend class DLListDelegate;

Q_OBJECT
public:
    Download( QStandardItemModel* model, QString fname, qulonglong fsize
        ,int iNextChunk1
        ,int iFirstChunk2 ,int iNextChunk2
        ,int iFirstChunk3
        , QString fhash, QString fhost);

    Download(QStandardItemModel *model, const QFileInfo& fileInfo);
    ~Download();

    // Receives events from the download thread
    void customEvent( QEvent *e );
    // Starts the download
    void start();
    // Stop the download
    void stop();
    // Cancel the download
    void cancel();
    // Pause the download
    void pause();
    // Resume the download
    void resume();
    // Set thread priority.
    void setPriority(DownloadDefs::Priority priority);
    // Force download
    void forceDequeue();
    // Delete hash file
    void delHash();
    // Delete incoming hash file.
    void delIncomingHashFile();

    // Get the download status
    DownloadQueue::state state() {return queueState;}
    // Get the fileName
    QString fileName() {return name;}
    // Get the host
    QString getHost() {return host;}
    // Get file size.
    qulonglong getSize(){return size;}
    // Get file hash.
    QString getHash(){return hash;}
    // Have chunk or not.
    bool haveChunk(int chunkNumber);
    // Set download status.
    void setStatus(DownloadDefs::DownloadStatus);
    // Get download status.
    DownloadDefs::DownloadStatus getStatus() const {return currentStatus;}
    // Set row color.
    void setRowColor(int c, QColor color);
    // Read hash from file system.
    void readHash(const QFileInfo& fileInfo);
    // Write hash to file system.
    void writeHash();
    // Set download sources.
    void setSources( int );
    // Get source queue.
    SourceQueue* getSources() {return sources;}
    // Get thread priority.
    DownloadDefs::Priority getPriority() const {return priority;}
    // Set route quality
    void setRouteQuality( int );
    // Get route quality
    int getRouteQuality () const {return routeQuality;}
    // verify otherChunk
    void cleanOtherChunks();

    QString getFilenameWithoutPath();
    QString getFileExtension();
    QString getHashPath();
    QString getPartialFilePath ();
    QString getIncomingDirectory();
    // Incoming full file path with real name.
    QString getIncomingPath();
    // Incoming full file path with hash name.
    QString getIncomingPathOld();
    QString getCompletedPath();

    TaskGraphPainterWidget* getTaskGraphicWidget () {return taskGraphPainterWidget;}

    const QVector<TaskGraphPainterWidget::NotDownload>& getDownloadSegments () const {return download_segments;}

signals:
    // Signals the item is not in state ACTIVE anymore
//     void deactivated( QTreeWidgetItem* );
    // Signals a download started
    void downloadStarted( QString );
    // Signals a download paused
    void downloadPaused(QString);
    // Signals a download ended - for any reason ( complete, failed, canceled )
    void downloadEnded( QString );

public:
    static const int BYTE_PER_CHUNK = 16384;    // 16k per chunk

private:
    void setName( QString );
    void setHost( QString );
    void setProgress( int, unsigned long );
    void updateProgress();
    void updateAdvancedProgress ();
    int myRow();
    void initMaxDownloadLimit();
    void init(QString fname, qulonglong fsize, QString fhash, QString fhost);
    void initTaskGraphPainterWidget (qint64 fsize, qint64 fblock);
    void updateDownloadSegments ();
    void initDownloadSegments ();

    int nextChunk1;//first part of file in download
    int firstChunk2,nextChunk2;//second part of file in download
    int firstChunk3;//second part of file in download


    static const int SEGMENT_SIZE = 3;
    QVector<TaskGraphPainterWidget::NotDownload> download_segments;

    QMutex dlLock;
    QVector<int> otherChunks;
    QString name, hash, host;
    SourceQueue *sources;

    class DownloadThread *dlThread;
    qulonglong size;
    DownloadQueue::state queueState;
    QStandardItemModel *model;
    //QPersistentModelIndex nameIndex, progressIndex, hostIndex, statusIndex, routeIndex, sourcesIndex;
    int routeQuality;
    qulonglong currentSize;
    DownloadDefs::DownloadStatus currentStatus;
    QTime blockStartTime[10];
    qulonglong blockStartSize[10];
    int nbStart;
    int currentRate;

    TaskGraphPainterWidget *taskGraphPainterWidget;

    DownloadDefs::Priority priority;
};

#endif
