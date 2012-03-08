/***************************************************************************
 *   Copyright (C) 2008                                                    *
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
 **************************************************************************/

#include "kommutedefs.h"
#include "searchresultfilter.h"
#include <limits.h>
#include <QRegExp>

#ifdef DEBUG
#include <QDebug>
#endif

const char* SearchResultFilter::FilterNames[SearchResultFilter::NUMBER_OF_FILTERS] =
{
    QT_TR_NOOP("RegExp"),
    QT_TR_NOOP("MinFileSize"),
    QT_TR_NOOP("MaxFileSize"),
    QT_TR_NOOP("MinSourceNumber"),
    QT_TR_NOOP("FileType")
};

SearchResultFilter::SearchResultFilter()
{
    for (int i = 0; i < NUMBER_OF_FILTERS; ++i)
        filterKeyValueMap_.insert(FilterNames[i], "");
}

const QString& SearchResultFilter::getValueFromName(const QString& name) const
{
    QMap<QString, QString>::const_iterator cit = filterKeyValueMap_.find(name);
    Q_ASSERT(cit != filterKeyValueMap_.constEnd());
    return *cit;
}

void SearchResultFilter::setValueFromName(const QString& name, const QString& value)
{
    QMap<QString, QString>::iterator it = filterKeyValueMap_.find(name);
    Q_ASSERT(it != filterKeyValueMap_.constEnd());
    *it = value;
}

bool SearchResultFilter::isSearchRegExpMatch(const QString& fileName) const
{
    const QString& searchRegExp = getValueFromName(FilterNames[REG_EXP]);
    QRegExp regExp(searchRegExp, Qt::CaseInsensitive);
    return fileName.contains(regExp);
}

bool SearchResultFilter::isFileSizeMatch(const QString& fileSizeStr) const
{
    const QString& minFileSizeStr = getValueFromName(FilterNames[MIN_FILE_SIZE]);
    const QString& maxFileSizeStr = getValueFromName(FilterNames[MAX_FILE_SIZE]);
    qulonglong minFileSize = (minFileSizeStr.isEmpty()) ? 0 : (minFileSizeStr.toULongLong() << 20);
    qulonglong maxFileSize = (maxFileSizeStr.isEmpty()) ? ULLONG_MAX : (maxFileSizeStr.toULongLong() << 20);
    qulonglong fileSize = fileSizeStr.toULongLong();

    return (fileSize >= minFileSize && fileSize <= maxFileSize);
}

bool SearchResultFilter::isSourceNumMatch(const QString& sourceNumStr) const
{
    const QString& minSourceNumStr = getValueFromName(FilterNames[MIN_SOURCE_NUM]);
    uint minSourceNum = minSourceNumStr.isEmpty() ? 0 : minSourceNumStr.toUInt();
    uint sourceNum = sourceNumStr.toUInt();
    return sourceNum >= minSourceNum;
}

bool SearchResultFilter::isFileTypeMatch(const QString& searchFileType) const
{
    const QString& fileType = getValueFromName(FilterNames[FILE_TYPE]);
    return fileType.isEmpty()
        || (KommuteDefs::tr(KommuteDefs::FileTypeName[KommuteDefs::ALL]) == fileType)
        || (searchFileType == fileType);
}
