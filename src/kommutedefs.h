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
#ifndef KOMMUTEDEFS_H
#define KOMMUTEDEFS_H

#include <QEvent>
#include <QCoreApplication>

class KommuteDefs
{
    Q_DECLARE_TR_FUNCTIONS(KommuteDefs)

public:

    enum FileType
    {
        ALL,
        PICTURE,
        VIDEO,
        AUDIO,
        ARCHIVE,
        PROGRAM,
        CD_IMAGE,
        DOCUMENT,
        UNKNOWN,
        NUMBER_OF_FILETYPES
    };

    static const char* FileTypeName[NUMBER_OF_FILETYPES];
    static const char* FileTypeResourcePath[NUMBER_OF_FILETYPES];

    // Custom event types
    enum EventType
    {
        EVENTTYPE_SEARCHRESULT = (QEvent::User + 1),
        EVENTTYPE_DOWNLOADFILE,
        EVENTTYPE_DOWNLOADSTATUS,
        EVENTTYPE_CONNECTION,
        EVENTTYPE_CONNECTIONCOUNT,
        EVENTTYPE_UPLOAD,
        EVENTTYPE_UPLOADSCOUNT,
        EVENTTYPE_CONNECTIONATTEMPT,
        EVENTTYPE_CLEARCONNECTIONS,
        EVENTTYPE_SHAREDFILES,
        EVENTTYPE_ADDLOG,
        NUMBER_OF_EVENTTYPE
    };
};

#endif
