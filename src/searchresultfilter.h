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
#ifndef SEARCH_RESULT_FILTER_H
#define SEARCH_RESULT_FILTER_H

#include <QString>
#include <QMap>

class SearchResultFilter
{
public:
    SearchResultFilter();

    const QString& getValueFromName(const QString& name) const;
    void setValueFromName(const QString& name, const QString& value);

    bool isSearchRegExpMatch(const QString& fileName) const;
    bool isFileSizeMatch(const QString& fileSizeStr) const;
    bool isSourceNumMatch(const QString& sourceNumStr) const;
    bool isFileTypeMatch(const QString& fileType) const;

    enum FilterName
    {
        REG_EXP,
        MIN_FILE_SIZE,
        MAX_FILE_SIZE,
        MIN_SOURCE_NUM,
        FILE_TYPE,
        NUMBER_OF_FILTERS
    };

    static const char* FilterNames[NUMBER_OF_FILTERS];

private:
    QMap<QString, QString> filterKeyValueMap_;   // contains filter's name value pairs.
};
#endif
