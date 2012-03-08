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

#include <QDir>

#include "MUTE/layers/messageRouting/messageRouter.h"
#include "MUTE/fileShare.h"

#include "minorGems/util/SettingsManager.h"
#include "applog.h"

#include "mutelayer.h"
#include "kommutedefs.h"

int MuteLayer::muteLayerStart(QString seed, QString settingsDir, int port)
{   
    // Initialize the SettingsManager with config directory
    SettingsManager::setDirectoryName( (char*) settingsDir .toLocal8Bit().constData());
    muteSeedRandomGenerator((char*) seed.toLatin1().constData());
    muteStart( port );
    muteShareStart();
    return 0;
}

int MuteLayer::muteLayerStop()
{
    muteShareStop();
    muteStop();
    return 0;
}

int MuteLayer::muteLayerGetConnectedHostList(char ***addresses,
                                             int **ports,
                                             int **sentCounts,
                                             int **queuedCounts,
                                             int **droppedCounts)
{
    return muteGetConnectedHostList(addresses,
                                    ports,
                                    sentCounts,
                                    queuedCounts,
                                    droppedCounts);
}

char MuteLayer::muteLayerGetCurrentConnectionAttempt(char **currentAttemptAddress,
                                                     int *currentAttemptPort)
{
    return muteGetCurrentConnectionAttempt(currentAttemptAddress,
                                           currentAttemptPort);
}

int MuteLayer::muteLayerGetUploadStatus( int **outUploadIDs,
                                char ***outHostAddresses,
                                char ***outFilePaths,
                                int **outChunksInFile,
                                int **outLastChunks,
                                unsigned long **outFirstChunkTimes,
                                unsigned long **outLastChunkTimes )
{

    return muteShareGetUploadStatus( outUploadIDs,
                                     outHostAddresses,
                                     outFilePaths,
                                     outChunksInFile,
                                     outLastChunks,
                                     outFirstChunkTimes,
                                     outLastChunkTimes );
}

void MuteLayer::muteLayerSearch( char *inSearchString,
                                 char (*inResultHandler)( char *, char *, unsigned long, char *,void * ),
                                 void *inExtraHandlerArgument,
                                 int inTimeoutInMilliseconds ,char * address)
{

    muteShareSearch( inSearchString,
                     inResultHandler,
                     inExtraHandlerArgument,
                     inTimeoutInMilliseconds ,address);
}

int MuteLayer::muteLayerGetTargetNumberOfConnections()
{
    return muteGetTargetNumberOfConnections();
}

void MuteLayer::muteLayerSetTargetNumberOfConnections(int value)
{
    muteSetTargetNumberOfConnections(value);
}



int MuteLayer::muteLayerGetMaxNumberOfConnections()
{
    return muteGetMaxNumberOfConnections();
}

void MuteLayer::muteLayerSetMaxNumberOfConnections(int value)
{
    muteSetMaxNumberOfConnections(value);
}


void MuteLayer::muteLayerInterruptDownload( int inDownloadID )
{
    muteShareInterruptDownload( inDownloadID );
}

char* MuteLayer::muteLayerGetSharingPath()
{
    return muteShareGetSharingPath();
}

QString MuteLayer::getMnemonic( QString inVirtualAddress )
{
   char *mnemo=muteGetMnemonic( (char*) inVirtualAddress.toLatin1().data());
   QString ret=QString::fromLatin1( mnemo);
   delete [] mnemo;
   return ret;
}

char MuteLayer::isDownloadHibernating( int inDownloadID )
{
    return muteShareIsDownloadHibernating( inDownloadID );
}

char *MuteLayer::getBestSource( int inDownloadID )
{
    return muteShareGetBestSource( inDownloadID );
}

int MuteLayer::getSourceCount( int inDownloadID )
{
    return muteShareGetSourceCount( inDownloadID );
}
char * MuteLayer::getLocalVirtualAddress()
{
    return muteShareGetLocalVirtualAddress();
}

