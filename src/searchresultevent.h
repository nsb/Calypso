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
#ifndef SEARCHRESULTEVENT_H
#define SEARCHRESULTEVENT_H

#include <QEvent>
#include <QString>

/**
@author Niels Sandholt Busch
*/
class SearchResultEvent : public QEvent
{
public:
    SearchResultEvent(QString inFilePath,
                      qint64 inFileLength,
                      QString inFileHash,
                      QString inFileHostVirtualAddress);

    ~SearchResultEvent();

    qint64 inFileLength;
    QString inFilePath, inFileHash, inFileHostVirtualAddress;
};

#endif
