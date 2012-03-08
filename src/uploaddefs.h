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
#ifndef UPLOADDEFS_H
#define UPLOADDEFS_H

#include <QCoreApplication>

class UploadDefs
{
    Q_DECLARE_TR_FUNCTIONS(UploadDefs)

public:
    // Upload status
    enum UploadStatus
    {
        UPLOADSTATUS_DONE = 0,
        UPLOADSTATUS_FAILED,
        UPLOADSTATUS_STALLED,
        UPLOADSTATUS_STARTING,
        UPLOADSTATUS_UPLOADING,
        NUMBER_OF_UPLOADSTATUS
    };
    
    static const char* uploadStatusName[NUMBER_OF_UPLOADSTATUS];

};

#endif

