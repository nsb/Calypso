/***************************************************************************
 *   Copyright (C) 2005 by Niels Sandholt Busch                            *
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
#include <errno.h>
#include "downloadthread.h"
#include "downloadfileevent.h"
#include "downloadstatusevent.h"
#include "kommutedefs.h"
#include "mutelayer.h"
#include "statistic.h"
#include "settings.h"
#include "defaultsettings.h"
#include "MUTE/messageRouter.h"
#include "MUTE/fileShare.h"
//#include "MUTE/stringUtils.h"
#include "minorGems/util/stringUtils.h"
#include "minorGems/network/web/URLUtils.h"
#include "minorGems/formats/encodingUtils.h"
#include "applog.h"

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QDataStream>
#include <QtDebug>
#include <QMutex>
#include <QSemaphore>
#include <QCryptographicHash>

#define BUFFER_TO_READ 10240    // 10k per buffer
#define TIME_TO_SLEEP 100

#define makeRand(max) (int)(((double)((max))*(double)qrand()) / ((double)RAND_MAX + 1.0) )

extern class Settings *settings;

const int DEFAULT_DOWNLOADTHREAD_TIMEOUT = 5000;
extern QMutex muteDownloadDataLock;
extern long muteNumActiveDownloads;
extern QVector < long >*muteDownloadQueue;
extern char *muteShareVirtualAddress;

extern long muteMaxSimultaneousDownloads ;
long actualsemQueueLimit =
  DEFAULT_SETTINGS_STANDARDMAXSIMULTANEOUSDOWNLOADS;
QSemaphore semQueue (DEFAULT_SETTINGS_STANDARDMAXSIMULTANEOUSDOWNLOADS);

DownloadThread::DownloadThread (QObject * eventReceiver, Download * inDl)
: QThread ()
, running (false)
, forceDequeue (false)
, eventReceiver (eventReceiver)
, dl (inDl)
, currentSize (0)
, downloadId (0)
, saveFile (NULL)
, partialFilePath (0)
, mutex()
, mSearchID(NULL)
, muteHandlerID(-1)
, paused(false)
{
    long newLimit = settings->getMaxSimultaneousDownloadsSetting ();

    if (newLimit != actualsemQueueLimit )
    {
        muteDownloadDataLock.lock ();
        newLimit = settings->getMaxSimultaneousDownloadsSetting ();
        if(newLimit <= 0) newLimit=100000;
        semQueue.release (newLimit - actualsemQueueLimit);
        actualsemQueueLimit = newLimit;
        muteDownloadDataLock.unlock ();
    }
}

DownloadThread::~DownloadThread ()
{
    if (muteHandlerID != -1)
    {
        muteRemoveMessageHandler (muteHandlerID);
        muteHandlerID = -1;
    }

    terminate ();
    wait (3000);

    if(mSearchID)
    {
      delete [] mSearchID;
      mSearchID=NULL;
    }
    if (partialFilePath)
        free (partialFilePath);

    for (int j = 0; j < MAX_SIM_CHUNKS; ++j)
    {
        if (fileChunk[j].mChunkData)
        {
            delete [] fileChunk[j].mChunkData;
            fileChunk[j].mChunkData = NULL;
        }
        if (fileChunk[j].mVirtualAddress)
            delete [] fileChunk[j].mVirtualAddress;
    }
}

void
DownloadThread::run ()
{
    running = true;
    uint seed = QDateTime::currentDateTime ().toTime_t () + QTime::currentTime ().msec ();
    qsrand (seed);

    if (!validateIncomingDirectory ())
    {
        QApplication::postEvent (eventReceiver,
            new DownloadStatusEvent (DownloadDefs::DOWNLOADSTATUS_FILETRANSFERSTOPPED));
        return;
    }

    // Check for existing partial file with old name pattern
    QString partialFile = dl->getPartialFilePath ();
    QFile file (partialFile);

    // Check for existing partial file ( resume )
    file.setFileName (partialFile);
    downloadId = -1;

    if (file.exists ())
    {
        QFileInfo fInfo (file);
        currentSize = fInfo.size ();

        if (currentSize > dl->getSize ())
        {
            file.resize (dl->getSize ());
            currentSize = dl->getSize ();
        }

        partialFilePath = strdup (partialFile.toLocal8Bit ().data ());

        // Post progress notification to download listviewitem
        QApplication::postEvent (eventReceiver,
            new DownloadFileEvent (downloadId, dl->getSize(), currentSize));
    }

    // set correct values for old downloads
    if (dl->firstChunk3 == 0 && dl->nextChunk1 == 0
        && dl->firstChunk2 == 0 && dl->nextChunk2 == 0
        && currentSize > 0)
    {
        // download from kommute release < 0.14
        int chunkDones = currentSize / Download::BYTE_PER_CHUNK;
        dl->nextChunk1 = chunkDones;
        dl->firstChunk2 = -1;
        dl->nextChunk2 = -1;
        dl->firstChunk3 = -1;
    }

    // Start the file transfer.
    char *sHost = strdup ((char *) dl->getHost ().toLocal8Bit ().constData ());
    char *sName = strdup ((char *) dl->fileName ().toLocal8Bit ().constData ());
    char *sHash = strdup ((char *) dl->getHash ().toLocal8Bit ().constData ());

    // initialize some things
    QSemaphore *downloadSemaphore = new QSemaphore ();
    char queued = true;

    // initialize fileChunk structs
    for (int j = 0; j < MAX_SIM_CHUNKS; ++j)
    {
        fileChunk[j].mChunkNumber = -1;
        fileChunk[j].mSemaphore = downloadSemaphore;
        fileChunk[j].mDataReceived = false;
        fileChunk[j].mChunkData = NULL;
        fileChunk[j].mVirtualAddress = NULL;
        fileChunk[j].sources = dl->getSources();
    }

    int chunkCount = 1 + (dl->getSize () - 1) / Download::BYTE_PER_CHUNK;

    if (dl->nextChunk1 < 0)
        dl->nextChunk1 = 0;

    int lastChunk1 = dl->nextChunk1 - 1;    // last chunk requested for first part
    int lastChunk2 = dl->nextChunk2 - 1;    // last chunk requested for second part

    // start with no string
    char *currentSourceAddress = NULL;
    char *currentSourceFilePath = NULL;
    muteHandlerID = muteAddMessageHandler (DownloadThread::muteDownloadHandler, (void *) this);
int first=1;
    lastSearchTime.start();
    // send a search for hash_xxx
    sendSearch();
    while (running)
    {
        // is the download finished ?
        if (((dl->nextChunk1 >= chunkCount))
            || ( dl->firstChunk3 > 0 && (dl->nextChunk1 >= dl->firstChunk3)))
        {
            // calculate hash
            printf ("hashing %s\n", dl->fileName ().toLocal8Bit ().data ());

            QCryptographicHash hash (QCryptographicHash::Sha1);
            file.open (QIODevice::ReadOnly);

            QByteArray buf;
            QTime time0;
            time0.start ();

            int percentCPU = 25;

            do
            {
                buf = file.read (BUFFER_TO_READ);
                hash.addData (buf);

                //limit CPU usage by sleeping when more than 10 ms elapsed
                if (time0.elapsed () > 10)
                {
                    int toSleep = (time0.elapsed () * (100 - percentCPU)) / percentCPU;

                    while (toSleep > 10 && running)
                    {
                        msleep (TIME_TO_SLEEP);
                        toSleep -= TIME_TO_SLEEP;
                    }
                    time0.start ();
                }
            }
            while (buf.size () == BUFFER_TO_READ && running);
            file.close();

            if (running)
            {
                printf ("hashed %s\n", dl->fileName ().toLocal8Bit ().data ());
            }

            // is hash OK ?
            if (strcmp(hash.result ().toHex ().toUpper ().data (),
                dl->getHash ().toUpper ().toLocal8Bit ().data ()))
            {
                printf ("bad hash %s\n", dl->fileName ().toLocal8Bit ().data ());
                QApplication::postEvent (eventReceiver,
                       new DownloadStatusEvent(DownloadDefs::DOWNLOADSTATUS_FILETRANSFERHASHMISMATCH));
            }
            else
            {
                if (saveFile != NULL)
                {
                    fclose (saveFile);
                    saveFile = NULL;
                }

                QApplication::postEvent (eventReceiver,
                       new DownloadStatusEvent(DownloadDefs::DOWNLOADSTATUS_FILETRANSFERCOMPLETE));

                dl->delHash();

                moveCompleteFile ();

                printf ("good hash %s\n", dl->fileName ().toLocal8Bit ().data ());
            }

            running = false;

            if (!queued)
                semQueue.tryAcquire (1, TIME_TO_SLEEP);

            goto end_download;
            //return;
        }

        // is there a connection open ?
        while (running && muteGetConnectionCount () < 1)
        {
            msleep (TIME_TO_SLEEP);
        }

	if(first)
	{
          // add a random sleep time to avoid to send too many messages at connection
          msleep(TIME_TO_SLEEP + makeRand(10000));
	  first=0;
	}
	else
          msleep(TIME_TO_SLEEP );

        //  is download queued ?
        while (running && queued && !forceDequeue)
        {
            if (semQueue.tryAcquire (1, TIME_TO_SLEEP))
                queued = false;
        }

        // pause by user?
        if (paused)
        {
            pauseMutex.lock();
            pauseCondition.wait(&pauseMutex);
            pauseMutex.unlock();
        }

        if (!lastSearchTime.isValid() || lastSearchTime.elapsed() > 300000)
        {
            dl->getSources()->findNewBest();
	    lastSearchTime.start();
            // send a search for hash_xxx
            sendSearch();
            // TODO : send a file info request
            //
        }

	// try to join parts if possible
	dl->cleanOtherChunks();
        if( dl->nextChunk2 <= dl->nextChunk1 )
        {
            // strange, should not happen
            dl->nextChunk2 = -1;
            dl->firstChunk2 = -1;
            lastChunk2 = -1;
        }

        // verify if first part has join second segment
        if (dl->firstChunk2 >= 0 && dl->nextChunk1 >= dl->firstChunk2
            && dl->nextChunk2 > dl->nextChunk1)
        {
            dl->nextChunk1 = dl->nextChunk2;
            lastChunk1 = lastChunk2;
            lastChunk2 = -1;
            dl->nextChunk2 = -1;
            dl->firstChunk2 = -1;
        }

        // verify if second part has raised end of file or third segment
        int chunkCount = 1 + (dl->getSize () - 1) / Download::BYTE_PER_CHUNK;

        if (dl->firstChunk2 >= 0 && (dl->nextChunk2 >= chunkCount
           || (dl->firstChunk3 > 0 && dl->nextChunk2 >= dl->firstChunk3)))
        {
            dl->firstChunk3 = dl->firstChunk2;
            dl->nextChunk2 = -1;
            dl->firstChunk2 = -1;
            lastChunk2 = -1;
        }

        //
        if (dl->nextChunk2 < 0
            && (dl->nextChunk1 < chunkCount - MAX_SIM_CHUNKS))
        {
            if (dl->firstChunk3 > 0)
            {
                dl->nextChunk2 = dl->nextChunk1 +
                    makeRand (dl->firstChunk3 - dl->nextChunk1);
            }
            else
            {
                dl->nextChunk2 = dl->nextChunk1 +
                    makeRand (chunkCount - dl->nextChunk1);
            }
            lastChunk2 = dl->nextChunk2 - 1;
        }

        //  send a file chunk request for each free fileChunk
        for (int j = 0; j < MAX_SIM_CHUNKS; ++j)
        {
            if (fileChunk[j].mDataReceived)
                continue;

            if (fileChunk[j].mChunkNumber >= 0)
            {
                // resend a message if previous is too old
                if (fileChunk[j].startTime.elapsed() < 20000)
                    continue;
            }
            else
            {
                if ((j % 2) == 0)
                {
		    // skip chunks already downloaded
                    while (dl->otherChunks.indexOf(++lastChunk1) >= 0)
			;
                    if ( (lastChunk1 < dl->firstChunk2)
			 || ( (dl->firstChunk2 < 0) && (lastChunk1 < dl->nextChunk2 )))
                    {
                        fileChunk[j].mChunkNumber = lastChunk1;
                    }
		    else
		        lastChunk1 -- ;
                }

                if (fileChunk[j].mChunkNumber < 0)
		{
		    // skip chunks already downloaded
                    while(dl->otherChunks.indexOf(++lastChunk2) >= 0)
			    ;
		    if ( lastChunk2 < chunkCount
                         && (dl->firstChunk3 <= 0 || lastChunk2 < dl->firstChunk3))
                    {
                        fileChunk[j].mChunkNumber = lastChunk2;
                    }
		    else
		        lastChunk2 -- ;
		}
            }

            if (fileChunk[j].mChunkNumber >= 0)
            {
                QMutexLocker chunkLock (&fileChunk[j].mLock);
                fileChunk[j].startTime.start();
                fileChunk[j].mSemaphore = downloadSemaphore;
                fileChunk[j].mDataReceived = false;

                if (fileChunk[j].mChunkData)
                  delete [] fileChunk[j].mChunkData;

                fileChunk[j].mChunkData = NULL;
                fileChunk[j].sources = dl->getSources();
                dl->getSources()->getNextSource (&currentSourceAddress,
                      &currentSourceFilePath);

                if (fileChunk[j].mVirtualAddress)
                    delete [] fileChunk[j].mVirtualAddress;

                fileChunk[j].mVirtualAddress = currentSourceAddress;

                // send out a request message for the chunk
                char *encodedFilePath = URLUtils::hexEncode (currentSourceFilePath);
                char *message = autoSprintf ("MessageType: FileChunkRequest\n"
                   "FilePath: %s\n" "Chunknumber: %d",
                   encodedFilePath,
                   fileChunk[j].mChunkNumber);

                delete[]encodedFilePath;
                /*TODO ? should be done in messageRouter layer.
                if (useFreshRouteForNextChunk)
                {
                muteSendMessage (muteShareVirtualAddress,
                currentSourceAddress, message,
                "FRESH_ROUTE");
                useFreshRouteForNextChunk = false;
                }
                else
                */
                muteSendMessage (muteShareVirtualAddress,
                    currentSourceAddress, message);

                delete[]currentSourceFilePath;
                delete[]message;
            }
        }

        // TODO : wait response with timeout.
        mSemaphore.tryAcquire (1,2000);
        mSemaphore.tryAcquire(mSemaphore.available());
        //msleep (2000);
        //
        // write received datas to file
        char dataswrited = false;

        for (int j = 0; j < MAX_SIM_CHUNKS; ++j)
        {
            if (fileChunk[j].mDataReceived)
            {
                QMutexLocker chunkLock (&fileChunk[j].mLock);
                fileChunk[j].mDataReceived = false;
                int inChunkNumber = fileChunk[j].mChunkNumber;
                mutex.lock ();

                // Validate saveFile stream.
                if (saveFile == NULL)
                {
                    if (!openSaveFile ())
                    {
                        mutex.unlock ();
                        //TODO : post a message
                        goto end_download;
                    }
                }

                if (currentSize < (static_cast<qulonglong>((off_t)inChunkNumber * (off_t)Download::BYTE_PER_CHUNK)))
                {
                    // add nulls to file if size is less than inChunkNumber*BYTE_PER_CHUNK
                    char * vides = (char *) calloc(1, Download::BYTE_PER_CHUNK);
                    int nbChunk=currentSize / Download::BYTE_PER_CHUNK;
                    fseeko64 (saveFile, (off_t)nbChunk * (off_t)Download::BYTE_PER_CHUNK, SEEK_SET);

                    while (nbChunk < inChunkNumber)
                    {
                        fwrite (vides, Download::BYTE_PER_CHUNK, 1, saveFile);
                        ++nbChunk;
                    }
                    free(vides);
                }

                off_t pos = inChunkNumber * Download::BYTE_PER_CHUNK;
                fseeko64 (saveFile, pos, SEEK_SET);
                int toWrite = fileChunk[j].mLengthInBytes;

                if (static_cast<qulonglong>(pos + toWrite) > dl->getSize())
                {
                    toWrite = (dl->getSize() - pos);
                    printf ("write last chunk,iclib=%d; toWrite=%d\n",
                        fileChunk[j].mLengthInBytes, toWrite);
                }

                if (toWrite > 0)
                {
                    if (!memcmp(fileChunk[j].mChunkData,"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0",20))
		    {
			//qDebug() <<"20 zeros chunk "<<inChunkNumber << dl->name ;
                        char * vides = (char *) calloc(1, Download::BYTE_PER_CHUNK);
 			if (!memcmp(fileChunk[j].mChunkData,vides,Download::BYTE_PER_CHUNK))
			{
			  qWarning() << QTime::currentTime().toString(QString::fromUtf8("hh:mm:ss")) <<" 16384 zeros chunk  "<<inChunkNumber << dl->name ;
			  // FIXME : files can have null chunks, but old buggy releases of kommute send false null chunks,  how to be sure ?
			  toWrite=0;
			}
		    }
                }

                if (toWrite > 0)
                {

                    fwrite (fileChunk[j].mChunkData, toWrite, 1, saveFile);
                    // Keep a running total of the current size of the downloaded file.
                    if (static_cast<qulonglong>(pos + toWrite) >currentSize)
                        currentSize = pos+toWrite;

                    fflush (saveFile);
                    StatLock.lock ();
                    TotBytesDownloaded += toWrite;
                    StatLock.unlock ();
                    dataswrited=true;

                    // post a message
                    QApplication::postEvent (eventReceiver,
                       new DownloadFileEvent (downloadId, dl->getSize(), currentSize));

                    if (dl->otherChunks.indexOf(fileChunk[j].mChunkNumber) < 0);
                      dl->otherChunks.append(fileChunk[j].mChunkNumber);

                    fileChunk[j].mChunkNumber = -1;
                }
                mutex.unlock ();
            }
        }

        dl->cleanOtherChunks();

        if (dataswrited)
            dl->writeHash();

        dl->getSources()->findNewBest();

        // TODO : calculate new timeout to use
        QApplication::postEvent (eventReceiver,
           new DownloadStatusEvent (DownloadDefs::DOWNLOADSTATUS_DOWNLOADING));
    }

end_download:
    free (sHost);
    free (sName);
    free (sHash);
    delete downloadSemaphore;

    if (saveFile != NULL)
    {
        fclose (saveFile);
        saveFile = NULL;
    }

    running = false;
}

void
DownloadThread::sendSearch ()
{
    if(mSearchID)
      delete [] mSearchID;
    mSearchID = muteGetUniqueName ();
    //  char * hash = dl->hash.toLocal8Bit().data();
    char *searchString = autoSprintf ("hash_%s", dl->hash.toLocal8Bit().data());
    char *encodedSearchString = URLUtils::hexEncode (searchString);

    char *message = autoSprintf ("MessageType:\tSearchRequest\n"
                                 "SearchID: %s\n" "SearchString: %s",
                                 mSearchID,
                                 encodedSearchString);
    char *hashSeed = muteGetForwardHashSeed ();
    char *forwardFlag = autoSprintf ("FORWARD_%s", hashSeed);
    delete[] hashSeed;
    muteSendMessage (muteShareVirtualAddress,
                     "ALL", message, forwardFlag);
    delete[] forwardFlag;
    delete[] message;
    delete[] encodedSearchString;
    delete[] searchString;
}

void
DownloadThread::stop ()
{
    if (running)
    {
        running = false;
        muteShareInterruptDownload (downloadId);
    }
}

void
DownloadThread::pause()
{
    paused = true;
}

void
DownloadThread::resume()
{
    pauseCondition.wakeAll();
    paused = false;
}

int
DownloadThread::downloadHandler (char *inFromAddress,
    char *inToAddress, char *inBody)
{

    SimpleVector < char *> *tokens = tokenizeString (inBody);
    int numTokens = tokens->size ();

    if (numTokens >= 6)
    {
        char *typeToken = *(tokens->getElement (1));

        if (strcmp ("FileChunk", typeToken) == 0)
        {
            char *encodedPath = *(tokens->getElement (3));
            char *filePath = URLUtils::hexDecode (encodedPath);
            char *chunkNumberString = *(tokens->getElement (5));
            errno = 0;
            int chunkNumber = strtol (chunkNumberString, (char **) NULL, 10);

            if (errno)
                goto no_utility;

            // search for this chunk number :
            for (int j = 0; j < MAX_SIM_CHUNKS; ++j)
            {
                if (chunkNumber != fileChunk[j].mChunkNumber)
                    continue;

                // break if we have downloaded this chunk
                if (fileChunk[j].mDataReceived)
                    break;

                // search now for this source
                int numSources = dl->getSources()->getSourceCount ();
                int index = 0;
                FileSource *otherSource;

                for (index = 0; index < numSources; index++)
                {
                    otherSource = dl->getSources()->getSource (index);

                    if (!strcmp (inFromAddress, otherSource->mVirtualAddress)
                      && !strcmp (filePath, otherSource->mFilePath))
                    break;
                }


                if (index >= numSources)
                    goto no_utility;

                // this chunk is for us!!
		otherSource->setLastReceipt();
		otherSource->mChunkTimeInMilliseconds = fileChunk[j].startTime.elapsed();
                dl->getSources()->findNewBest();

                char *lengthString = *(tokens->getElement (7));
                // default to 0
                int length = strtol (lengthString, (char **) NULL, 10);
                QMutexLocker chunkLock (&fileChunk[j].mLock);

                fileChunk[j].mLengthInBytes = length;

                char *encodedData = *(tokens->getElement (9));
                int dataLength;

                unsigned char *decodedData = base64Decode (encodedData, &dataLength);

                if (decodedData != NULL)
                {
                    if (dataLength == length)
                    {
                      if (!memcmp(decodedData,"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0",20))
		      {
			//qDebug() <<"20 zeros chunk "<<inChunkNumber << dl->name ;
                        char * vides = (char *) calloc(1, Download::BYTE_PER_CHUNK);
 			if (!memcmp(decodedData,vides,Download::BYTE_PER_CHUNK))
			{
			  qWarning() <<"16384 zeros chunk "<<chunkNumber << dl->name ;
			  // FIXME : files can have null chunks, but old buggy releases of kommute send false null chunks,  how to be sure ?
                          delete[]decodedData;
			  decodedData=NULL;
		          otherSource->mChunkTimeInMilliseconds = -1;
                          dl->getSources()->findNewBest();
			}
		      }
		      if( decodedData)
		      {
                        if( fileChunk[j].mChunkData)
                            delete [] fileChunk[j].mChunkData;

                        fileChunk[j].mChunkData = decodedData;

                        // we have now received this chunk, so we
                        // should never overwrite it again if another
                        // message comes in
                        fileChunk[j].mDataReceived = true;
			qDebug() << "chunk " << chunkNumber << "received from " << inFromAddress;
                        mSemaphore.release();
		      }
                    }
                    else
                    {
                      AppLog::error ("fileShare -- chunk handler",
                         "Data length incorrect");

                      delete[]decodedData;
                    }
                }
                else
                {
                    AppLog::error ("fileShare -- chunk handler",
                        "Failed to decode data (base64 format bad?)");
                }
            }
	    delete []filePath;
        }
        else if (strcmp ("FileInfo", typeToken) == 0)
        {
            //TODO
        }
        else if (strcmp ("SearchResults", typeToken) == 0)
        {
            char *searchID = *(tokens->getElement (3));

            if (mSearchID && strcmp (searchID, mSearchID) == 0)
            {
                char *resultCountString = *(tokens->getElement (5));
                int resultCount = strtol(resultCountString, (char **)NULL, 10);
                int firstResultToken = 7;
                int lastResultToken = firstResultToken + 3 * resultCount;

                if (numTokens >= lastResultToken)
                {
                    mutex.lock ();

                    for (int i = firstResultToken; i < lastResultToken - 2; i += 3)
                    {
                        char *encodedFileName = *(tokens->getElement (i));
                        char *filePath = URLUtils::hexDecode (encodedFileName);
                        FileSource *newSource = new FileSource (stringDuplicate (inFromAddress),
                            filePath);
                        dl->getSources()->addSource (newSource);
                        dl->setSources(dl->getSources()->getSourceCount());
                        delete newSource;
                    }
                    mutex.unlock ();
                }
            }
        }
    }

no_utility:
    for (int i = 0; i < numTokens; ++i)
    {
      delete [] (*(tokens->getElement (i)));
    }

  delete tokens;

  // no utility generated
  return 0;
}

int
DownloadThread::muteDownloadHandler (char *inFromAddress,
                                     char *inToAddress,
                                     char *inBody,
                                     void *inExtraParam)
{
    return static_cast <DownloadThread *>(inExtraParam)->downloadHandler (inFromAddress,
        inToAddress, inBody);
}

void
DownloadThread::dequeue ()
{
  forceDequeue = true;
}

bool DownloadThread::openSaveFile ()
{
    // Open this file for writing.
    saveFile = fopen (dl->getPartialFilePath ().toLocal8Bit ().constData (), "r+b");

    if (saveFile)
    {
        return true;
    }

    printf(" erreur ouverture fichier, errno=%d\n",errno);
    saveFile = fopen (dl->getPartialFilePath ().toLocal8Bit ().constData (), "w+b");

    return (saveFile != NULL);
}

bool DownloadThread::validateIncomingDirectory ()
{
    QString incomingPath = dl->getIncomingDirectory ();
    QDir incomingDir (incomingPath);

    if (!incomingDir.exists ())
    {
        // Download directory does not exist, so make it.
        if (!incomingDir.mkdir (incomingPath))
        {
            return false;
        }
    }
    return true;
}

bool DownloadThread::moveCompleteFile ()
{

    QString incomingFile = dl->getIncomingPath ();
    int postFix = 0;
    QString incomingFile2=incomingFile;
    QFile fictemp(dl->getPartialFilePath ());
    // search a correct name
    while ( fictemp.exists ( incomingFile2))
    {
        int insertPoint = incomingFile.lastIndexOf ('.');
        incomingFile2=incomingFile;
        // file without extension
        if (-1 == insertPoint)
            incomingFile2.append (QString ("_%1").arg (++postFix));
        else
            incomingFile2.insert (insertPoint, QString ("_%1").arg (++postFix));

        // 100 times to break out.
        if (postFix >= 100)
            return false;
    }
    // move file from temp folder to incoming folder.
    fictemp.rename ( incomingFile2);
    if(fictemp.exists(dl->getPartialFilePath ()) && !fictemp.exists(incomingFile2))
    {
      qWarning() <<"rename don't work : copy file "<<fictemp.fileName()<<" to :"<<incomingFile2;
      fictemp.unsetError();
      fictemp.copy ( incomingFile2);
    }
    if(fictemp.exists(dl->getPartialFilePath ()) && fictemp.exists(incomingFile2))
    {
      msleep (1000);
      fictemp.remove(dl->getPartialFilePath ());
    }

    return (fictemp.exists(incomingFile2));
}

