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
#include "uploadsthread.h"
#include "uploadevent.h"
#include "uploadscountevent.h"

#include "mutelayer.h"

#include <QApplication>
#include <QMap>
#include <time.h>

const int   DEFAULT_UPTHREAD_SLEEP = 3;

typedef QMap<int, int> UploadsMap;

UploadsThread::UploadsThread( QObject *eventReceiver )
 : QThread(), running( false ), eventReceiver( eventReceiver ), numberOfUploads( 0 )
{
}

UploadsThread::~UploadsThread()
{
    wait();
}

void UploadsThread::run()
{
    running = true;

    int *uploadIDs;
    char **hostAddresses;
    char **filePaths;
    int *chunksInFile;
    int *lastChunksSent;
    unsigned long *firstChunkTimes;
    unsigned long *lastChunkTimes;
    int numUploads, inactiveUploads = 0;
    int i;

    UploadsMap ulMap;
    UploadsMap::Iterator ulIt;
    while( running )
    {
        numUploads = MuteLayer::muteLayerGetUploadStatus( &uploadIDs,
                                               &hostAddresses,
                                               &filePaths,
                                               &chunksInFile,
                                               &lastChunksSent,
                                               &firstChunkTimes,
                                               &lastChunkTimes );
        for(i=0;i<numUploads;i++)
        {
//             ulIt = ulMap.find( uploadIDs[ i ] );
            if( ulMap.contains( uploadIDs[ i ] ) && !ulMap.value( uploadIDs[ i ] ))
//             if( ulIt != ulMap.end() && !ulIt.data() )
            {
                continue; // Inactive upload
            }
            else
            {
                // Existing active upload or new upload
                int status = getStatus(chunksInFile[ i ],
                                       lastChunksSent[ i ],
                                       firstChunkTimes[ i ],
                                       lastChunkTimes[ i ]);

                switch( status )
                {
                    case UploadDefs::UPLOADSTATUS_DONE:
                        ulMap[ uploadIDs[ i ] ] = false;
                        inactiveUploads++;
                        break;
                    case UploadDefs::UPLOADSTATUS_FAILED:
                        ulMap[ uploadIDs[ i ] ] = false;
                        inactiveUploads++;
                        break;
                    case UploadDefs::UPLOADSTATUS_STALLED:
                        ulMap[ uploadIDs[ i ] ] = true;
                        break;
                    case UploadDefs::UPLOADSTATUS_STARTING:
                        ulMap[ uploadIDs[ i ] ] = true;
                        break;
                    case UploadDefs::UPLOADSTATUS_UPLOADING:
                        ulMap[ uploadIDs[ i ] ] = true;
                        break;
                    default:
                        break;
                }

                QApplication::postEvent( eventReceiver,
                                         new UploadEvent( filePaths[ i ],
                                                          hostAddresses[ i ],
                                                          chunksInFile[ i ],
                                                          lastChunksSent[ i ],
                                                          uploadIDs[ i ],
                                                          status ) );
            }
        }

        if( numberOfUploads != numUploads - inactiveUploads )
        {
            QApplication::postEvent( eventReceiver, new UploadsCountEvent( numUploads - inactiveUploads ) );
            numberOfUploads = numUploads - inactiveUploads;
        }

        for(i=0;i<numUploads;i++)
        {
            delete [] hostAddresses[i];
            delete [] filePaths[i];
        }
        delete [] uploadIDs;
        delete [] hostAddresses;
        delete [] filePaths;
        delete [] chunksInFile;
        delete [] lastChunksSent;
        delete [] firstChunkTimes;
        delete [] lastChunkTimes;

        QThread::sleep( DEFAULT_UPTHREAD_SLEEP );
    }
}

void UploadsThread::stop()
{
  if (running)
  {
    running = false;
  }
}

UploadDefs::UploadStatus UploadsThread::getStatus( int chunksInFile,
                               int lastChunkSent,
                               unsigned long firstChunkTime,
                               unsigned long lastChunkTime )
{
    // Get the current time in seconds using the API-required time(NULL) call.
    unsigned long currentSeconds = time( NULL ); // TODO: Replace with QTime
    // Determine the upload's current status.
    unsigned long elapsedSeconds = currentSeconds - lastChunkTime;
    int numChunksSent = lastChunkSent + 1;

    if( numChunksSent == chunksInFile ) {
        // All chunks uploaded: done.
        return UploadDefs::UPLOADSTATUS_DONE;
    } else if( elapsedSeconds >= 900 ) {
        // No progress for 15 minutes: failed.
        return UploadDefs::UPLOADSTATUS_FAILED;
    } else if( elapsedSeconds >= 60 ) {
        // No progress for 1 minute: stalled.
        return UploadDefs::UPLOADSTATUS_STALLED;
    } else {
        return UploadDefs::UPLOADSTATUS_UPLOADING;
    }

}

