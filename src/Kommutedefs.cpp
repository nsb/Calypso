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

#include "kommutedefs.h"

const char* KommuteDefs::FileTypeName[KommuteDefs::NUMBER_OF_FILETYPES] =
{
    QT_TR_NOOP(" All"),
    QT_TR_NOOP(" Picture"),
    QT_TR_NOOP(" Video"),
    QT_TR_NOOP(" Audio"),
    QT_TR_NOOP(" Archive"),
    QT_TR_NOOP(" Program"),
    QT_TR_NOOP(" CD-Image"),
    QT_TR_NOOP(" Document"),
    QT_TR_NOOP(" Unknown")
};

const char* KommuteDefs::FileTypeResourcePath[KommuteDefs::NUMBER_OF_FILETYPES] =
{
    ":/resources/FileTypeAny.png",
    ":/resources/FileTypePicture.png",
    ":/resources/FileTypeVideo.png",
    ":/resources/FileTypeAudio.png",
    ":/resources/FileTypeArchive.png",
    ":/resources/FileTypeProgram.png",
    ":/resources/FileTypeCDImage.png",
    ":/resources/FileTypeDocument.png",
    ":/resources/FileTypeAny.png"
};

