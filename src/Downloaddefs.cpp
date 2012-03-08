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

#include "downloaddefs.h"
#include <QtGlobal>

const char* DownloadDefs::priorityNames[DownloadDefs::NUMBER_OF_PRIORITY] =
{
    QT_TR_NOOP("High"),
    QT_TR_NOOP("Normal"),
    QT_TR_NOOP("Low"),
    QT_TR_NOOP("Auto")
};

const char* DownloadDefs::priorityResourcePath[DownloadDefs::NUMBER_OF_PRIORITY] =
{
    ":/resources/highpriority.png",
    ":/resources/autopriority.png",
    ":/resources/lowpriority.png",
    ":/resources/questionpriority.png"
};

const char* DownloadDefs::downloadStatusName[DownloadDefs::NUMBER_OF_DOWNLOADSTATUS] =
{
    QT_TR_NOOP("Connecting"),
    QT_TR_NOOP("Downloading"),
    QT_TR_NOOP("File not found"),
    QT_TR_NOOP("Failed"),
    QT_TR_NOOP("Stopped"),
    QT_TR_NOOP("Complete"),
    QT_TR_NOOP("Corrupted"),
    QT_TR_NOOP("Queued"),
    QT_TR_NOOP("Dequeued"),
    QT_TR_NOOP("Delayed"),
    QT_TR_NOOP("Unknown"),
    QT_TR_NOOP("Time out retry"),
    QT_TR_NOOP("Waiting for sources"),
    QT_TR_NOOP("Starting"),
    QT_TR_NOOP("Paused"),
    QT_TR_NOOP("Fetching file info")
};

const char* DownloadDefs::downloadStatusResourcePath[DownloadDefs::NUMBER_OF_DOWNLOADSTATUS] =
{
    ":/resources/connecting.png",
    ":/resources/download16.png",
    ":/resources/notfound-corrupt.png",
    ":/resources/notfound-corrupt.png",
    ":/resources/stop.png",
    ":/resources/complete.png",
    ":/resources/notfound-corrupt.png",
    ":/resources/queued.png",
    "",                                 // do not find resource of dequeue
    ":/resources/delayed.png",
    "",                                 // do not find resource of unknown
    "",                                 // do not find resource of wait for retry
    ":/resources/waitingforsources.png",
    ":/resources/start.png",
    ":/resources/pause.png",
    ":/resources/fileinfo.png"
};

