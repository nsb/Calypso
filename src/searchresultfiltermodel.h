/***************************************************************************
 *   Copyright (C) 2008
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

#ifndef SEARCH_RESULT_FILTER_MODEL
#define SEARCH_RESULT_FILTER_MODEL

#include <QSortFilterProxyModel>

class SearchResultFilter;

class SearchResultFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    SearchResultFilterModel(const SearchResultFilter* filter = 0, QObject* parent = 0);

    void setSearchResultFilter(const SearchResultFilter* filter);

    // override from QSortFilterProxyModel
    bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const;

public:
    enum SearchColumn
    {
        SEARCH_NAME = 0,
        SEARCH_SIZE,
        SEARCH_NBHOST,
        SEARCH_TYPE,
        SEARCH_HASH,
        SEARCH_HOST,
        SEARCH_HOSTHASH,
        NUMBER_OF_SEARCH_COLUMN
    };

    static const char* searchColumnName[NUMBER_OF_SEARCH_COLUMN];

private:
    const SearchResultFilter* filter_;
};
#endif
