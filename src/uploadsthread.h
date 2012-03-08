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
#ifndef UPLOADSTHREAD_H
#define UPLOADSTHREAD_H

#include <QThread>
#include "uploaddefs.h"

/**
@author Niels Sandholt Busch
*/
class UploadsThread : public QThread
{
public:
    UploadsThread(QObject*);

    ~UploadsThread();

    void run();
    void stop();

private:
    bool running;
    QObject *eventReceiver;
    int numberOfUploads;
    UploadDefs::UploadStatus getStatus( int chunksInFile,
                    int lastChunkSent,
                    unsigned long firstChunkTime,
                    unsigned long lastChunkTime );
};

#endif
