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
#include "uploadlistitem.h"
#include "kommutedefs.h"
#include "kommuteutils.h"
#include "MUTE/fileShare.h"

UploadListItem::UploadListItem( QTreeWidget *ulListView, QString filePath, QString host, int uploadID )
 : QTreeWidgetItem( ulListView ), filePath( filePath ), host( host ), m_uploadID( uploadID )
{
  char *sharedPath = muteShareGetSharingPath ();
  QString sharedDirectory (sharedPath);

  if(filePath.startsWith(sharedDirectory))
  {
    QString fileName(filePath);
        fileName.remove (0, sharedDirectory.length () + 1);
    setText( 0, fileName );
  }
  else
    setText( 0, filePath );
    setText( 1, host );
    m_active = true; // Set active as default
}


UploadListItem::~UploadListItem()
{
}

int UploadListItem::uploadID()
{
    return m_uploadID;
}

void UploadListItem::setProgress( int lastChunkSent, int chunksInFile,  UploadDefs::UploadStatus status )
{  
    switch( status )
    {
        case UploadDefs::UPLOADSTATUS_DONE:
            setText( 3, (UploadDefs::uploadStatusName[UploadDefs::UPLOADSTATUS_DONE]) );
            m_active = false;
            break;
        case UploadDefs::UPLOADSTATUS_FAILED:
            setText( 3, (UploadDefs::uploadStatusName[UploadDefs::UPLOADSTATUS_FAILED]) );
            m_active = false;
            break;
        case UploadDefs::UPLOADSTATUS_STALLED:
            setText( 3, (UploadDefs::uploadStatusName[UploadDefs::UPLOADSTATUS_STALLED]) );
            m_active = true;
            break;
        case UploadDefs::UPLOADSTATUS_STARTING:
            setText( 3, (UploadDefs::uploadStatusName[UploadDefs::UPLOADSTATUS_STARTING]) );
            m_active = true;
            break;
        case UploadDefs::UPLOADSTATUS_UPLOADING:
            setText( 3, (UploadDefs::uploadStatusName[UploadDefs::UPLOADSTATUS_UPLOADING]) );
            setText( 2, KommuteUtils::convertSize(lastChunkSent) + " / " + KommuteUtils::convertSize(chunksInFile));
            m_active = true;
            break;
        default:
            break;
    }
}

bool UploadListItem::active()
{
    return m_active;
}
