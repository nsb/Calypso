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
#include <QString>

class MuteLayer {

public:
    static int muteLayerStart(QString seed, QString settingsDir, int port);
    static int muteLayerStop();

    // Get the connected hosts list
    static int muteLayerGetConnectedHostList(char ***outHostAddresses,
                                             int **outHostPorts,
                                             int **outSentMessageCounts,
                                             int **outQueuedMessageCounts,
                                             int **outDroppedMessageCounts);

    static char muteLayerGetCurrentConnectionAttempt( char **outHostAddress, int *outPort );

    static int muteLayerGetUploadStatus( int **outUploadIDs,
                                         char ***outHostAddresses,
                                         char ***outFilePaths,
                                         int **outChunksInFile,
                                         int **outLastChunks,
                                         unsigned long **outFirstChunkTimes,
                                         unsigned long **outLastChunkTimes );

    static void muteLayerSearch(char *inSearchString,
                                char (*inResultHandler)( char *, char *, unsigned long, char *,void * ),
                                void *inExtraHandlerArgument,
                                int inTimeoutInMilliseconds = 10000, char *address=NULL );

    static int muteLayerGetTargetNumberOfConnections();
    static void muteLayerSetTargetNumberOfConnections(int value);

    static int muteLayerGetMaxNumberOfConnections();
    static void muteLayerSetMaxNumberOfConnections(int value);

    static void muteLayerInterruptDownload( int inDownloadID );

    static char *muteLayerGetSharingPath();
    static QString getMnemonic( QString inVirtualAddress );

    static char *getBestSource( int inDownloadID );
    static char isDownloadHibernating( int inDownloadID );
    static int getSourceCount( int inDownloadID );
    static char * getLocalVirtualAddress();
};
