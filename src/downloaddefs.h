/***************************************************************************
 *   Copyright (C) 2009                                                    *
 *   summitsummit@163.com                                                  *
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
#ifndef DOWNLOAD_DEFS_H
#define DOWNLOAD_DEFS_H

#include <QCoreApplication>

class DownloadDefs
{
    Q_DECLARE_TR_FUNCTIONS(DownloadDefs)

public:
    enum Priority
    {
        HIGH = 0,
        NORMAL,
        LOW,
        AUTO,
        NUMBER_OF_PRIORITY
    };

    static const char* priorityNames[NUMBER_OF_PRIORITY];
    static const char* priorityResourcePath[NUMBER_OF_PRIORITY];

    // Download status
    enum DownloadStatus
    {
        DOWNLOADSTATUS_CONNECTING = 0,
        DOWNLOADSTATUS_DOWNLOADING,
        DOWNLOADSTATUS_FILENOTFOUND,
        DOWNLOADSTATUS_FILETRANSFERFAILED,
        DOWNLOADSTATUS_FILETRANSFERSTOPPED,
        DOWNLOADSTATUS_FILETRANSFERCOMPLETE,
        DOWNLOADSTATUS_FILETRANSFERHASHMISMATCH,
        DOWNLOADSTATUS_QUEUED,
        DOWNLOADSTATUS_DEQUEUED,
        DOWNLOADSTATUS_DELAYED,
        DOWNLOADSTATUS_UNKNOWN,
        DOWNLOADSTATUS_TIMED_OUT_WILL_RETRY,
        DOWNLOADSTATUS_WAITING_FOR_SOURCES,
        DOWNLOADSTATUS_STARTING,
        DOWNLOADSTATUS_PAUSED,
        DOWNLOADSTATUS_FETCHING_INFO,
        NUMBER_OF_DOWNLOADSTATUS
    };

    static const char* downloadStatusName[NUMBER_OF_DOWNLOADSTATUS];
    static const char* downloadStatusResourcePath[NUMBER_OF_DOWNLOADSTATUS];
};

#endif

