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

#include "searchresultfiltermodel.h"
#include "searchresultfilter.h"

#ifdef DEBUG
#include <qDebug>
#endif

const char* SearchResultFilterModel::searchColumnName[SearchResultFilterModel::NUMBER_OF_SEARCH_COLUMN] =
{
    QT_TR_NOOP("File Name"),
    QT_TR_NOOP("Size"),
    QT_TR_NOOP("Sources"),
    QT_TR_NOOP("Type"),
    QT_TR_NOOP("Hash"),
    QT_TR_NOOP("Host"),
    QT_TR_NOOP("Host (hash)")
};

SearchResultFilterModel::SearchResultFilterModel(const SearchResultFilter* filter, QObject* parent)
: QSortFilterProxyModel(parent)
, filter_(filter)
{
    invalidateFilter();
}

void SearchResultFilterModel::setSearchResultFilter(const SearchResultFilter* filter)
{
    filter_ = filter;
    invalidateFilter();
}

bool SearchResultFilterModel::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
{
    QModelIndex fileNameIndex = sourceModel()->index(sourceRow, SearchResultFilterModel::SEARCH_NAME, sourceParent);
    QModelIndex fileSizeIndex = sourceModel()->index(sourceRow, SearchResultFilterModel::SEARCH_SIZE, sourceParent);
    QModelIndex sourcesIndex = sourceModel()->index(sourceRow, SearchResultFilterModel::SEARCH_NBHOST, sourceParent);
    QModelIndex fileTypeIndex = sourceModel()->index(sourceRow, SearchResultFilterModel::SEARCH_TYPE, sourceParent);

    return filter_->isSearchRegExpMatch(sourceModel()->data(fileNameIndex).toString())
            && filter_->isFileSizeMatch(sourceModel()->data(fileSizeIndex).toString())
            && filter_->isSourceNumMatch(sourceModel()->data(sourcesIndex).toString())
            && filter_->isFileTypeMatch(sourceModel()->data(fileTypeIndex).toString());
}
