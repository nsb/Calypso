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

#ifndef WEBHOSTCACHESDOWNLOAD_H
#define WEBHOSTCACHESDOWNLOAD_H

#define DEBUG

#include <QStringList>
#include <QBuffer>
#include <QHttp>
#include <QThread>
#include <QSemaphore>

/**
@author Andy Gebauer - feivelda@googlemail.com
*/

class WebHostCachesDownload : public QThread
{
    Q_OBJECT
    
public:
    WebHostCachesDownload( QStringList oldData, QStringList downloadLocations );
    ~WebHostCachesDownload();

    void run();
    bool isDownloading();
    void stop();

    QStringList getWebHostCaches();
    void postLocalAddress();
    void post1LocalAddress (QString stringUrl,int port);
    void getSeedNodes();

private slots:
    void httpRequestFinished( int ID, bool error );

private:
    bool running;
    unsigned long mTimeToWaitBetweenAddressPostsInSeconds;
    unsigned long mTimeOfLastAddressPostInSeconds;
    QStringList webHostCachesList; // list of web caches
    QStringList list; // list of download locations for webcaches ( default = http://kommute.sourceforge.net/mcaches.txt)
    QHttp *http;
    QByteArray *byteArray;
    QBuffer *buffer;
    QSemaphore *semaphore;
    int downloadID;
    int myIP_ID;
    int release_ID;
    QList<int> seedIDs;
    QList<int> postIDs;
    QList<int> downloadIDs;

    // for debugging
    #ifdef DEBUG
    int standardListCount;
    int listCount;
    #endif

    void download(QString file);
    void parseBuffer();
    void parseSeedBuffer();
    void prepareList( QStringList oldData );
    void sortList();
};

#endif
