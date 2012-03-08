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

#include "kommuteutils.h"

QString KommuteUtils::convertSize( unsigned long size )
{
    float fsize;
    QString s;
    // Giga-byte
    if ( size >= 1073741824 )
    {
        fsize = static_cast<float>(size) / static_cast<float>(1073741824);
        if ( fsize > 1024 ) // Tera-byte
            s = QString("%1 TB").arg( formatNumber(fsize / static_cast<float>(1024), 1) );
        else
            s = QString( "%1 GB" ).arg( formatNumber(fsize, 1) );
    }
    // Mega-byte
    else if ( size >= 1048576 )
    {
        fsize = static_cast<float>(size) / static_cast<float>(1048576);
        s = QString( "%1 MB" ).arg( formatNumber(fsize, 1) );
    }
    // Kilo-byte
    else if ( size > 1024 )
    {
        fsize = static_cast<float>(size) / static_cast<float>(1024);
        s = QString( "%1 KB" ).arg( formatNumber(fsize, 1) );
    }
    // Just byte
    else
    {
        fsize = static_cast<float>(size);
        s = QString( "%1 B" ).arg( formatNumber(fsize, 0) );
    }
    return s;
}

QString KommuteUtils::convertSize( int size )
{
    return convertSize(static_cast<unsigned long>(size));
}

QString KommuteUtils::formatNumber(double num, int precision)
{
    bool neg = num < 0;
    if (precision == -1) precision = 2;
    QString res = QString::number(neg?-num:num, 'f', precision);
    int pos = res.indexOf('.');
    if (pos == -1) pos = res.length();
    else res.replace(pos, 1, ".");

    while (0 < (pos -= 3))
     res.insert(pos, "."); // thousand sep

    // How can we know where we should put the sign?
    //res.prepend(neg?negativeSign():positiveSign());

    return res;
}

QString KommuteUtils::formatNumber(const QString &numStr)
{
    return formatNumber(numStr.toDouble());
}
