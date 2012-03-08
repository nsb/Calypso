/***************************************************************************
 *   Copyright (C) 2009                                                    *
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

#include <QStringList>
#include "KommuteLinkAnalyzer.h"

const QString KommuteLinkAnalyzer::HEADER_NAME = "kommute://file|";

KommuteLinkAnalyzer::KommuteLinkAnalyzer (const QString & url)
{
    this->url = url;
}

void KommuteLinkAnalyzer::setKommuteLink (const QString & name, const QString & size, const QString & hash)
{
    url.append(HEADER_NAME);
    url.append(name);
    url.append("|");
    url.append(size);
    url.append("|");
    url.append(hash);
    url.append("\n");
}

bool KommuteLinkAnalyzer::getFileInformation(QVector<KommuteLinkData>& linkList)
{
    if (!isValid ())
        return false;

    QStringList urlList = url.split ("\n");

    for (int i = 0, n = urlList.size(); i < n; ++i)
    {
        if (urlList[i].isEmpty ())
            continue;

        QStringList list = urlList[i].split ("|");
        linkList.push_back (KommuteLinkData (list.at (NAME), list.at (SIZE), list.at (HASH)));
    }
    return true;
}

// easy check for now.
bool KommuteLinkAnalyzer::isValid () const
{
    QStringList urlList = url.split ("\n");

    if (urlList.isEmpty ())
        return false;

    QStringList list = urlList[0].split ("|");

    return (list.size() % NUMBER_OF_PARTITIONS == 0);
}

