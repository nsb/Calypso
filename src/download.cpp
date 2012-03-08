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
/***************************************************************************
 * explanation for firstChunk1, nextChunk1, firstChunk2, nextChunk2        *
 * , firstChunk3, nextChunk3 :                                             *
 *                                                                         *
 * each pair indicates a segment of the file being downloaded :            *
 *   firstChunkx is the first chunk of this segment downloaded             *
 *   firstChunkx has the value -1 if no chunk has been downloaded for this *
 *   segment.                                                              *
 *   nextChunkx is the next chunk to download for this segment             *
 *                                                                         *
 * we made sworming by downloading two segments of the file :              *
 * the first segment is the beginning, to allow preview of the file        *
 * the second segment is placed randomly in the file                       *
 * when the second segment reach end of file, we transfer the second       *
 * to the third. Then we start downloading the second segment at a random  *
 * place between first and third segment.                                  *
 *                                                                         *
 * So :                                                                    *
 *  first segment is always at beginning of file.                          *
 *  third segment is always at end of file                                 *
 *                                                                         *
 * at creation :                                                           *
 *   nextChunk1 = -1, firstChunk2 = -1 , nextChunk2 = -1, firstChunk3 = -1 *
 *                                                                         *
 *                                                                         *
 ***************************************************************************/
#include "download.h"
#include "downloadfileevent.h"
#include "downloadstatusevent.h"
#include "downloadthread.h"
#include "kommuteutils.h"
#include "kommutedefs.h"
#include "mutelayer.h"
#include "DLListDelegate.h"
#include "xprogressbar.h"
#include "settings.h"

#include <QEvent>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QModelIndex>
#include <QFile>
#include <QTextStream>
#include <QMutexLocker>
#include <QDebug>

extern class Settings *settings;
extern char *DEFAULT_INCOMINGHASH_DIR;

Download **Downloads=NULL;
int maxDownloads=0;
int nbDownloads=0;

void Download::initMaxDownloadLimit()
{
    if(nbDownloads >= maxDownloads)
    {
        Downloads = (Download**)realloc(Downloads,(maxDownloads+50)*sizeof(Download**));
        maxDownloads += 50;
    }

    Downloads[nbDownloads] = this;
    nbDownloads++;
}

void Download::initTaskGraphPainterWidget (qint64 fsize, qint64 fblock)
{
    taskGraphPainterWidget = new TaskGraphPainterWidget();
    taskGraphPainterWidget->setData (fsize, fblock);
}

void Download::initDownloadSegments ()
{
    for (int i = 0; i < SEGMENT_SIZE; ++i)
        download_segments.append (TaskGraphPainterWidget::NotDownload ());
}

void Download::init(QString fname, qulonglong fsize, QString fhash, QString fhost)
{
    initDownloadSegments ();

    // fill in download segments.
    updateDownloadSegments ();

    // block size is 16k.
    initTaskGraphPainterWidget (fsize, BYTE_PER_CHUNK);

    initMaxDownloadLimit();

    int c = model->rowCount();
    model->insertRow(c);

    model->setHeaderData(c,Qt::Vertical, (quintptr)this);
    model->setData(model->index(c,DLListDelegate::NAME), fname);
    model->setData(model->index(c,DLListDelegate::HASH), fhash);
    model->setData(model->index(c,DLListDelegate::PTR), (quintptr)this);

    sources = new SourceQueue (fhash.toLocal8Bit ().data (), fhost.toLocal8Bit ().data (), fname.toLocal8Bit ().data ());
    setSources(sources->getSourceCount());
    setProgress( 0, fsize );
    updateProgress();
    setRouteQuality( routeQuality );
    setStatus( DownloadDefs::DOWNLOADSTATUS_QUEUED );
    setHost( fhost);
    setPriority(priority);
    writeHash();
}

Download::Download(QStandardItemModel *model, QString fname, qulonglong fsize
        ,int iNextChunk1, int iFirstChunk2, int iNextChunk2, int iFirstChunk3
        ,QString fhash, QString fhost)
: QObject(0)
, nextChunk1( iNextChunk1 )
, firstChunk2( iFirstChunk2 )
, nextChunk2( iNextChunk2 )
, firstChunk3( iFirstChunk3 )
, name( fname )
, hash( fhash )
, host( fhost )
, dlThread( NULL )
, size( fsize )
, queueState( DownloadQueue::QUEUED )
, model( model )
, routeQuality( -1 )
, currentSize( 0 )
, nbStart( 0 )
, currentRate( 0 )
, priority(DownloadDefs::NORMAL)
, taskGraphPainterWidget(NULL)
{
    init(name, size, hash, host);
}

Download::Download(QStandardItemModel *model, const QFileInfo& fileInfo)
: QObject(0)
, dlThread( NULL )
, queueState( DownloadQueue::QUEUED )
, model( model )
, routeQuality( -1 )
, nbStart( 0 )
, currentRate( 0 )
, taskGraphPainterWidget(NULL)
{
    readHash(fileInfo);

    init(name, size, hash, host);
}

extern int muteShareChunkSize;

void Download::readHash(const QFileInfo& fileInfo)
{
    QFile FileHash(fileInfo.absoluteFilePath());

    if (!FileHash.open(QIODevice::ReadOnly))
    {
        fprintf(stderr, "can't open file %s\n", fileInfo.absoluteFilePath().toLocal8Bit().data());
        return;
    }

    hash = strdup(fileInfo.fileName().toUpper().toLocal8Bit().data());
    QTextStream Stream (&FileHash);
    QString fname, s_fsize, fhost, fpriority;
    fname = Stream.readLine();
    name = strdup(fname.toLocal8Bit().data());
    s_fsize = Stream.readLine();
    size = s_fsize.toULong();
    fhost = Stream.readLine();
    host = strdup(fhost.toLocal8Bit().data());
    fpriority = Stream.readLine();
    priority = static_cast<DownloadDefs::Priority>(fpriority.toInt());

    int firstChunk1 = -2;
    int nextChunk3 = -2;
    Stream >> firstChunk1;
    Stream >> nextChunk1;
    Stream >> firstChunk2;
    Stream >> nextChunk2;
    Stream >> firstChunk3;
    Stream >> nextChunk3;
    int oneChunk;

    while (!Stream.atEnd())
    {
        oneChunk = -1;
        Stream >> oneChunk;

        if (oneChunk > 0)
        {
            otherChunks.append(oneChunk);
        }
    }
    FileHash.close();

}

void Download::writeHash()
{
    // create file hash with contents=name of file
    // Mute internal hash put in temp folder which set manually.
    QString fullName(settings->getTempDirectory());
    // Add the incoming download path.
    fullName += "/";
    fullName += DEFAULT_INCOMINGHASH_DIR;
    fullName += "/";
    fullName += hash;
    QFile FileHash(fullName);

    if (!FileHash.open( QIODevice::WriteOnly ))
    {
        return;
    }

    QTextStream Stream ( & FileHash );
    Stream << name << endl;
    Stream << size << endl;
    Stream << host << endl;
    Stream << priority << endl;

    int firstChunk1=0;

    if(nextChunk1 == 0)
        firstChunk1 = -1;

    Stream << firstChunk1 << " " << nextChunk1 << " " ;
    Stream << firstChunk2 << " " << nextChunk2 << " " ;

    int nextChunk3 = -1;

    if (firstChunk3 > 0)
        nextChunk3 = 2 + (size - 1) / muteShareChunkSize;

    Stream << firstChunk3 << " " << nextChunk3 << endl;

    dlLock.lock();
    for (int i = 0; i < otherChunks.size(); ++i)
        Stream << otherChunks.at(i) << " ";
    dlLock.unlock();

    Stream << endl;

    //Stream << otherChunks;
    FileHash.close();
}


Download::~Download()
{
    delete taskGraphPainterWidget;
    taskGraphPainterWidget = NULL;
    if (dlThread)
    {
        dlThread->stop();
        dlThread->wait(2000);
        delete dlThread;
        dlThread = NULL;
    }

    for (int i = 0; i < nbDownloads; ++i)
    {
        if (Downloads[i] == this)
        {
            --nbDownloads;
            Downloads[i] = Downloads[nbDownloads];
            break;
        }
    }
}

void Download::updateDownloadSegments ()
{
    download_segments[0].setSegment (0, nextChunk1 * BYTE_PER_CHUNK);
    download_segments[1].setSegment (firstChunk2 * BYTE_PER_CHUNK, nextChunk2 * BYTE_PER_CHUNK);
    download_segments[2].setSegment (firstChunk3 * BYTE_PER_CHUNK, getSize ());
}

void Download::customEvent( QEvent *e )
{
    DownloadDefs::DownloadStatus downloadStatus;

    switch (e->type())
    {
        case KommuteDefs::EVENTTYPE_DOWNLOADFILE:
        {
            char *newHost = MuteLayer::getBestSource(static_cast<DownloadFileEvent*>(e)->downloadId);

            if (newHost && strcmp(newHost, ""))
                setHost(newHost);
            delete[] newHost;
        }

        if (currentStatus != DownloadDefs::DOWNLOADSTATUS_DOWNLOADING && blockStartSize[0] == 0)
        {
            blockStartSize[0] = static_cast<DownloadFileEvent*>(e)->currentSize ;
        }

        setProgress( static_cast<DownloadFileEvent*>(e)->currentSize, static_cast<DownloadFileEvent*>(e)->finalSize );
        break;
        case KommuteDefs::EVENTTYPE_DOWNLOADSTATUS:
        {
            downloadStatus = static_cast<DownloadStatusEvent*>(e)->getDownloadStatus();
            setStatus(downloadStatus);

            switch (downloadStatus)
            {
                // Active
                case DownloadDefs::DOWNLOADSTATUS_CONNECTING:
                    queueState = DownloadQueue::ACTIVE;
                    break;
                case DownloadDefs::DOWNLOADSTATUS_DOWNLOADING:
                    if (routeQuality < 10)
                    {
                        ++routeQuality;
                        setRouteQuality( routeQuality );
                    }
                    queueState = DownloadQueue::ACTIVE;
                    break;
                case DownloadDefs::DOWNLOADSTATUS_TIMED_OUT_WILL_RETRY:
                    if (routeQuality > 0)
                    {
                        --routeQuality;
                        setRouteQuality( routeQuality );
                    }
                    queueState = DownloadQueue::ACTIVE;
                    break;
                case DownloadDefs::DOWNLOADSTATUS_DEQUEUED:
                    queueState = DownloadQueue::ACTIVE;
                    break;
                case DownloadDefs::DOWNLOADSTATUS_WAITING_FOR_SOURCES:
                    queueState = DownloadQueue::ACTIVE;
                    // TODO: Clear the route quality and host fields
                    break;
                case DownloadDefs::DOWNLOADSTATUS_FETCHING_INFO:
                    queueState = DownloadQueue::ACTIVE;
                    break;
                case DownloadDefs::DOWNLOADSTATUS_STARTING:
                    queueState = DownloadQueue::ACTIVE;
                    break;

                // Queued
                case DownloadDefs::DOWNLOADSTATUS_QUEUED:
                    queueState = DownloadQueue::QUEUED;
                    break;

                // Inactive
                default:
                    queueState = DownloadQueue::INACTIVE;
                    emit downloadEnded( this->getHost() );
                    break;
            }
        }
        break;
        default:
            break;
    }
    updateProgress();
    updateAdvancedProgress ();
}

void Download::updateAdvancedProgress ()
{
    updateDownloadSegments ();
    taskGraphPainterWidget->setNotDownloadListClear();

    for (int i = 0, n = download_segments.size (); i < n; ++i)
    {
        const TaskGraphPainterWidget::NotDownload& segment = download_segments[i];

        if (segment.isValid ())
        {
            taskGraphPainterWidget->setNotDownloadList (i, segment.startPosition, segment.endPosition);
        }
    }

    taskGraphPainterWidget->refreshAll ();
}

void Download::setName( QString name )
{
    model->setData(model->index(myRow(),DLListDelegate::NAME), name);
}

void Download::setHost( QString host )
{
    model->setData(model->index(myRow(),DLListDelegate::HOST), MuteLayer::getMnemonic(host) );
    this->host = host;
    writeHash();
}

void Download::setProgress( int cSize, unsigned long fSize )
{
    currentSize = cSize;

    if (fSize > 0)
        size = fSize;
}

int Download::myRow()
{
    int c = model->rowCount();

    for (int i = 0; i < c; ++i)
      if (model->data(model->index(i, DLListDelegate::HASH)).toString() == hash)
        return i;

    return -1;
}

void Download::setStatus (DownloadDefs::DownloadStatus status )
{
    int c = myRow();
    currentStatus = status;

    model->setData(model->index(c, DLListDelegate::STATUS), tr(DownloadDefs::downloadStatusName[status]));
    model->setData(model->index(c, DLListDelegate::SYMBOL),
        QIcon(DownloadDefs::downloadStatusResourcePath[status]), Qt::DecorationRole);

    // additional setting for download complete.
    if (DownloadDefs::DOWNLOADSTATUS_FILETRANSFERCOMPLETE == status)
    {
        model->setData(model->index(c, DLListDelegate::PROGRESS),
            QIcon(xProgressBar::getPixmap(QSize(160, 17), 75, false, 1)), Qt::DecorationRole);
        updateProgress();
        updateAdvancedProgress ();
        setRowColor(c, QString::fromUtf8("green"));
        delHash();
    }
}

void Download::setPriority(DownloadDefs::Priority priority)
{
    this->priority = priority;
    model->setData(model->index(myRow(),DLListDelegate::PRIORITY), DownloadDefs::tr(DownloadDefs::priorityNames[priority]));

    if (dlThread)
    {
        switch (priority)
        {
            case DownloadDefs::HIGH:
                dlThread->setPriority(QThread::HighPriority);
                break;
            case DownloadDefs::NORMAL:
                dlThread->setPriority(QThread::NormalPriority);
                break;
            case DownloadDefs::LOW:
                dlThread->setPriority(QThread::LowPriority);
                break;
            case DownloadDefs::AUTO:
                dlThread->setPriority(QThread::NormalPriority);
                break;
            default:
                break;
        }
    }
    writeHash();
}

void Download::start()
{
    if (!dlThread)
    {
        dlThread = new DownloadThread(this, this);
    }

    dlThread->start();

    setStatus( DownloadDefs::DOWNLOADSTATUS_STARTING );

    // Start downloading
    queueState = DownloadQueue::ACTIVE;
    emit downloadStarted(this->getHost());
    writeHash();
}

void Download::stop()
{
    setStatus(DownloadDefs::DOWNLOADSTATUS_FILETRANSFERSTOPPED);

    if (dlThread)
    {
        dlThread->stop();
    }

    queueState = DownloadQueue::INACTIVE;
}

void Download::cancel()
{
    stop();
    delHash();
    delIncomingHashFile();
}

void Download::pause()
{
    // File is downloading.
    if (getStatus() == DownloadDefs::DOWNLOADSTATUS_DOWNLOADING)
    {
        setStatus(DownloadDefs::DOWNLOADSTATUS_PAUSED);

        if (dlThread)
        {
            dlThread->pause();
        }

        // pause downloading
        queueState = DownloadQueue::INACTIVE;
        emit downloadPaused(this->getHost());
    }
}

void Download::resume()
{
    // File is paused.
    if (getStatus() == DownloadDefs::DOWNLOADSTATUS_PAUSED)
    {
        setStatus(DownloadDefs::DOWNLOADSTATUS_DOWNLOADING);

        if (dlThread)
        {
            dlThread->resume();
        }
        queueState = DownloadQueue::ACTIVE;
    }
    // File is stoped.
    else if (getStatus() == DownloadDefs::DOWNLOADSTATUS_FILETRANSFERSTOPPED)
    {
        start ();
    }
}

void Download::delHash()
{
    //JMT : delete file hash
    QString fullName = getHashPath();
    QFile fileHash ( fullName );
    fileHash.remove();
}

void Download::delIncomingHashFile()
{
    // delete hash file in incoming folder.
    QString fullName = getIncomingPath();
    QFile fileHash(fullName);
    fileHash.remove();
}

void Download::setRouteQuality( int quality )
{
    QString str;

    if (quality < 0)
    {
        str = tr("Not connected");
    }
    else if (quality <= 3)
    {
        str = tr("Low");

    }
    else if (quality > 3 && quality < 7)
    {
        str = tr("Medium");
    }
    else
    {
        str = tr("High");
    }

    model->setData(model->index(myRow(), DLListDelegate::ROUTEQUALITY), str);
}


void Download::updateProgress( )
{
    float fractionDone = 0;
    int chunkdone = 0;
    int chunkCount = 1 + (size - 1) / 16384;

    if (size > 0)
    {
        chunkdone = nextChunk1;
        if(chunkdone <= 0) chunkdone=0;

        if (firstChunk2 > 0)
            chunkdone += (nextChunk2 - firstChunk2);

        if (firstChunk3 > 0)
            chunkdone += (chunkCount + 1 - firstChunk3);

        dlLock.lock();
        for (int i = 0; i < otherChunks.size(); ++i)
        {
            int chunk = otherChunks.at(i);
            // don't count chunks already counted
            if (  chunk >= nextChunk1
		  && (chunk <firstChunk2
                      || (nextChunk2 >0 && chunk >= nextChunk2 && chunk < firstChunk3)
                      || (nextChunk2 >0 && chunk >= nextChunk2 && firstChunk3 < 0)
                      || (firstChunk2 < 0 && chunk < firstChunk3 )))
                chunkdone ++ ;
        }
        dlLock.unlock();

        fractionDone = (float)chunkdone / (float)chunkCount;
    }
    else
    {
     // ???
     chunkCount=1;
     fractionDone=0;
    }

    qulonglong sizeDone = chunkdone * 16384;

    if ( chunkdone >= chunkCount ) 
    {
      sizeDone = size;
      chunkdone = chunkCount;
      fractionDone = 1.0;
    }
    else if (firstChunk3 > 0) // last chunk received
    {
        int sizeOfLastChunk= size % 16384;
        if (sizeOfLastChunk > 0)
            sizeDone += sizeOfLastChunk - 16384;
    }

    int percentDone = (int)(fractionDone * 100);

    if (nbStart == 0)
    {
        blockStartTime[0] = QTime::currentTime();
        blockStartSize[0] = sizeDone;
        nbStart=1;
    }

    long timeDelta = blockStartTime[nbStart-1].msecsTo( QTime::currentTime() );
    //printf("updateProgress: delta=%d,nb=%d,oldBlock=%d,currSize=%d\n"
    //,timeDelta,nbStart
    //,blockStartSize[0],currentSize);

    if (timeDelta > 2000) // 2 second has passed since last progress update
    {
        if(nbStart>=10)
        {
            for(int i=0 ; i< 9 ; i++)
            {
                blockStartTime[i]=blockStartTime[i+1];
                blockStartSize[i]=blockStartSize[i+1];
            }
            nbStart=9;
        }
        blockStartTime[nbStart] = QTime::currentTime();
        blockStartSize[nbStart++] = sizeDone;
        qulonglong sizeDelta = sizeDone - blockStartSize[0];
        timeDelta = blockStartTime[0].msecsTo( blockStartTime[nbStart-1]);

        if (timeDelta > 0)
        {
            currentRate = (sizeDelta*1000)/timeDelta;
            //printf("updateProgress: sdelta=%d,timeD=%d\n"
            //,sizeDelta,timeDelta);
        }
	else
	    currentRate=0;

        if (timeDelta >= 20000 && sizeDelta == 0)
            currentRate=0;
    }

    model->setData(model->index(myRow(),DLListDelegate::PROGRESS),  percentDone );
    model->setData(model->index(myRow(),DLListDelegate::DLSPEED), currentRate );
    model->setData(model->index(myRow(),DLListDelegate::SIZE), size);
    model->setData(model->index(myRow(),DLListDelegate::COMPLETED), sizeDone);

    if (currentRate > 0.01)
    {
      qulonglong remaining =(size-sizeDone)/currentRate;
      model->setData(model->index(myRow(),DLListDelegate::REMAINING), QString::number(remaining));
    }
    else
      model->setData(model->index(myRow(),DLListDelegate::REMAINING), QString::number(0));

    model->setData(model->index(myRow(),DLListDelegate::DONE), percentDone);

    QString chunks = QString( "%1/%2" )
                            .arg( QString::number( chunkdone ) )
                            .arg( QString::number( chunkCount ) );
    model->setData(model->index(myRow(),DLListDelegate::CHUNKS), chunks);
}

void Download::setSources( int sources )
{
    model->setData(model->index(myRow(),DLListDelegate::SOURCES), QString::number( sources ));
}

void Download::forceDequeue()
{
    if(dlThread)
    {
        setStatus( DownloadDefs::DOWNLOADSTATUS_STARTING );
        dlThread->dequeue();
    }
    else
    {
        start();
    }
}

// Set the color of a row in data model
void Download::setRowColor(int c, QColor color)
{
    unsigned int nbColumns = model->columnCount()-1;

    for (unsigned int i = 0; i < nbColumns; ++i)
    {
        model->setData(model->index(c, i), QVariant(color), Qt::ForegroundRole);
    }
}


bool Download::haveChunk(int chunkNumber)
{
    if (chunkNumber < 0)
        return false;

    int chunksInFile = 1 + (size-1) / muteShareChunkSize;

    if (chunkNumber >= chunksInFile)
        return false;

    if (chunkNumber < nextChunk1)
        return true;

    if (firstChunk3 > 0 && chunkNumber >= firstChunk3)
        return true;

    if (firstChunk2 >= 0 && chunkNumber >= firstChunk2 && chunkNumber < nextChunk2)
        return true;

    return false;
}

QString Download::getFileExtension ()
{
  int lastDot = fileName ().lastIndexOf (".");
  return fileName ().right (fileName ().length () - lastDot - 1);
}

QString Download::getFilenameWithoutPath ()
{
  int lastSlash = fileName ().lastIndexOf ("/");
  return fileName ().right (fileName ().length () - lastSlash - 1);
}

QString Download::getHashPath()
{
    // Mute internal hash put in temp folder which set manually.
    QString fullName(settings->getTempDirectory());

    fullName += "/";
    fullName += DEFAULT_INCOMINGHASH_DIR;
    fullName += "/";
    fullName += hash;
    return fullName;
}

// Keep this interface no change, 'cause incoming folder can be changed manually now.
QString Download::getIncomingDirectory ()
{
    return settings->getIncomingDirectory();
}

QString Download::getPartialFilePath ()
{
    QString fullName = settings->getTempDirectory ();
    fullName += "/";
    fullName += getFilenameWithoutPath ();
    return fullName;
}

QString Download::getIncomingPath ()
{
    QString fullName = getIncomingDirectory ();
    fullName += "/";
    fullName += getFilenameWithoutPath ();
    return fullName;
}

QString Download::getIncomingPathOld ()
{
  QString fullName = getIncomingDirectory ();
  fullName += "/";
  fullName += hash;
  fullName += ".";
  fullName += getFileExtension ();
  return fullName;
}

QString Download::getCompletedPath ()
{
  return getIncomingPath ();
}

void Download::cleanOtherChunks()
{
 QMutexLocker locker(&dlLock);
        int indexChunk;

        while ((indexChunk=otherChunks.indexOf(nextChunk1)) >= 0)
        {
            ++nextChunk1;
            otherChunks.remove(indexChunk);
        }

        while ((indexChunk=otherChunks.indexOf(nextChunk2)) >= 0)
        {
            if (firstChunk2 < 0)
                firstChunk2=nextChunk2;

            ++nextChunk2;
            otherChunks.remove(indexChunk);
        }
}
