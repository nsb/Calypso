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
#ifndef UPLOADLISTITEM_H
#define UPLOADLISTITEM_H

#include <QTreeWidgetItem>
#include "uploaddefs.h"

/**
@author Niels Sandholt Busch
*/
class UploadListItem : public QTreeWidgetItem
{
public:
    UploadListItem( QTreeWidget*, QString, QString, int );

    ~UploadListItem();

    int uploadID();
    bool active();

    void setProgress( int, int, UploadDefs::UploadStatus);

    QString filePath, host, progress;

private:
    int m_uploadID;
    bool m_active;
};

#endif
