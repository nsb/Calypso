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
#ifndef DOWNLOADTHREAD_H
#define DOWNLOADTHREAD_H

#include <QThread>
#include <QMutex>
#include <QTime>
#include <QWaitCondition>
#include "download.h"

/**
@author Niels Sandholt Busch
*/
class DownloadThread : public QThread
{
public:
    DownloadThread(QObject *eventReceiver,
                   Download * inDl);

    ~DownloadThread();

    virtual void run();

    void dequeue();
    void stop();
    void pause();
    void resume();
    bool isPaused() {return paused;}

private:
    int downloadHandler (char *inFromAddress,
                char *inToAddress,
                char *inBody);
    static int muteDownloadHandler (char *inFromAddress,
                char *inToAddress,
                char *inBody,
                void *inExtraParam);

    bool openSaveFile();
    bool validateIncomingDirectory();
    bool moveCompleteFile ();

    #define MAX_SIM_CHUNKS 10
    ShareFileChunkWrapper fileChunk[MAX_SIM_CHUNKS];
    bool running,forceDequeue;
    QObject *eventReceiver;
//    QString filename, host, filehash;
    Download *dl;
    qulonglong currentSize;
    int downloadId;
    FILE *saveFile;
    char  * partialFilePath;
    QMutex mutex;
    char * mSearchID;
    void sendSearch ();
    QTime lastSearchTime;
    int muteHandlerID;
    QSemaphore mSemaphore;

    volatile bool paused;
    QWaitCondition pauseCondition;
    QMutex pauseMutex;
};

#endif
