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

#include "searchresultsview.h"
#include "searchresultevent.h"
#include "SearchListDelegate.h"
#include "closesearchthreadqueue.h"
#include "kommuteutils.h"
#include "kommutedefs.h"
#include "mutelayer.h"
#include "download.h"
#include "searchresultfilter.h"
#include "searchresultfiltermodel.h"
#include "applog.h"

#include <QSettings>
#include <QStandardItemModel>
#include <QModelIndex>
#include <QMenu>
#include <QAction>
#include <QContextMenuEvent>
#include <QHeaderView>
#include <QIntValidator>
#include <QMessageBox>

#ifdef DEBUG
#include <QDebug>
#endif

void SearchResultsView::initSearchList ()
{
    for (int i = 0; i < SearchResultFilterModel::NUMBER_OF_SEARCH_COLUMN; ++i)
    {
        SearchListModel->setHeaderData(i, Qt::Horizontal, SearchResultFilterModel::tr(SearchResultFilterModel::searchColumnName[i]));
    }
}

void SearchResultsView::retranslateSearchList ()
{
    for (int i = 0; i < SearchListModel->rowCount (); ++i)
    {
        QString ext = QFileInfo(SearchListModel->data (SearchListModel->index (i, SearchResultFilterModel::SEARCH_NAME)).toString ()).suffix();
        SearchListModel->setData(SearchListModel->index(i, SearchResultFilterModel::SEARCH_TYPE), KommuteDefs::tr(KommuteDefs::FileTypeName[misc::fileType (ext)]));
    }
}

SearchResultsView::SearchResultsView(QWidget *parent, QString address, QString searchTerms, CloseSearchThreadQueue *searchThreadQueue)
: QWidget(parent)
, active( true )
, searchThreadQueue( searchThreadQueue )
, numSearchResults( 0 )
, address( address )
, searchTerms( searchTerms )
{
    setupUi(this);

    SearchListModel = new QStandardItemModel(0, SearchResultFilterModel::NUMBER_OF_SEARCH_COLUMN);

    initSearchList ();

    searchFilter = new SearchResultFilter();
    proxyModel = new SearchResultFilterModel(searchFilter);
    proxyModel->setDynamicSortFilter(true);
    proxyModel->setSourceModel(SearchListModel);
    searchtreeView->setModel(proxyModel);

    SearchDelegate = new SearchListDelegate();
    searchtreeView->setItemDelegate(SearchDelegate);

    searchtreeView->setSortingEnabled(true);
    searchtreeView->setDragDropOverwriteMode(false);
    searchtreeView->setDropIndicatorShown(false);
    searchtreeView->setTabKeyNavigation(false);
    /* hide the Tree +/- */
    searchtreeView->setRootIsDecorated(false);

    //searchtreeView->header()->setClickable(true);
    //searchtreeView->header()->setSortIndicatorShown(true);
    // Load last columns width for search results list

    if (!loadColWidthSearchList())
    {
        searchtreeView->header()->resizeSection(SearchResultFilterModel::SEARCH_NAME, 275);
    }


    /* Set header resize modes and initial section sizes */

    QHeaderView * _header =  searchtreeView->header () ;
    _header->setResizeMode (SearchResultFilterModel::SEARCH_SIZE, QHeaderView::Interactive);
    _header->setResizeMode (SearchResultFilterModel::SEARCH_HASH, QHeaderView::Interactive);
    _header->setResizeMode (SearchResultFilterModel::SEARCH_NBHOST, QHeaderView::Interactive);
    _header->setResizeMode (SearchResultFilterModel::SEARCH_HOST, QHeaderView::Interactive);
    _header->setResizeMode (SearchResultFilterModel::SEARCH_HOSTHASH, QHeaderView::Interactive);
    _header->resizeSection (SearchResultFilterModel::SEARCH_SIZE, 70);
    _header->resizeSection (SearchResultFilterModel::SEARCH_HASH, 260);
    _header->resizeSection (SearchResultFilterModel::SEARCH_NBHOST, 60);
    _header->resizeSection (SearchResultFilterModel::SEARCH_HOST, 80);
    _header->resizeSection (SearchResultFilterModel::SEARCH_TYPE, 70);
    _header->resizeSection (SearchResultFilterModel::SEARCH_HOSTHASH, 280);



    minSizeLine->setValidator(new QIntValidator(0, INT_MAX, minSizeLine));
    maxSizeLine->setValidator(new QIntValidator(0, INT_MAX, maxSizeLine));
    sourceNumLine->setValidator(new QIntValidator(0, INT_MAX, sourceNumLine));

    // initialize file type combo with file types.
    for (int i = 0; i < KommuteDefs::NUMBER_OF_FILETYPES; ++i)
    {
        fileTypeCombo->addItem(QIcon(KommuteDefs::FileTypeResourcePath[i]), KommuteDefs::tr(KommuteDefs::FileTypeName[i]));
    }


    connect(searchtreeView, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(downloadSelectedItem(const QModelIndex&)));

    connect(searchtreeView, SIGNAL(downloadItemsRequested()), this, SLOT(downloadSelectedItems()));
    connect(searchtreeView, SIGNAL(grabFriendRequested(QString, unsigned long, QString, QString )),
        this, SLOT(addFriend( QString, unsigned long, QString, QString )));
    connect(searchLine, SIGNAL(textChanged(const QString &)),

        this, SLOT(filterRegExpChanged()));
    connect(minSizeLine, SIGNAL(textChanged(const QString&)), this, SLOT(filterMinSizeChanged()));
    connect(maxSizeLine, SIGNAL(textChanged(const QString&)), this, SLOT(filterMaxSizeChanged()));
    connect(sourceNumLine, SIGNAL(textChanged(const QString&)), this, SLOT(filterSourceNumChanged()));
    connect(fileTypeCombo, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(filterFileTypeChanged()));

    connect(stopSearch, SIGNAL(clicked(bool )), this, SLOT(stopSearchButtonClicked()));
    connect(startSearch, SIGNAL(clicked(bool )), this, SLOT(startSearchButtonClicked()));
}

SearchResultsView::~SearchResultsView()
{
    sThread->eventsAllowed( false );
    active = false;
    searchThreadQueue->insertSearchThread( sThread );

    saveColWidthSearchList();
    delete proxyModel;
    delete SearchListModel;
    delete SearchDelegate;
    delete searchFilter;
}

void SearchResultsView::searchStopped()
{
    startSearch->setEnabled(true);
    startSearch->setDown (false);
}

void SearchResultsView::searchStarted()
{
    startSearch->setEnabled(false);
    startSearch->setDown (true);
}


void SearchResultsView::stopSearchButtonClicked ( )
{
    if(sThread->isRunning())
    sThread->stop();
}

void SearchResultsView::startSearchButtonClicked ( )
{
    if(!sThread->isRunning())
        sThread->start();
}

void SearchResultsView::on_clearfilter_Button_clicked()
{
    searchLine->clear();
    minSizeLine->clear();
    maxSizeLine->clear();
    sourceNumLine->clear();
    fileTypeCombo->setCurrentIndex(0);
}

void SearchResultsView::filterRegExpChanged()
{
    searchFilter->setValueFromName(SearchResultFilter::FilterNames[SearchResultFilter::REG_EXP], searchLine->text());
    proxyModel->setSearchResultFilter(searchFilter);
    emit numberOfSearchResultsChanged( this, numSearchResults, proxyModel->rowCount());
}

void SearchResultsView::filterMinSizeChanged()
{

    searchFilter->setValueFromName(SearchResultFilter::FilterNames[SearchResultFilter::MIN_FILE_SIZE], minSizeLine->text());
    proxyModel->setSearchResultFilter(searchFilter);
    emit numberOfSearchResultsChanged( this, numSearchResults, proxyModel->rowCount());

}

void SearchResultsView::filterMaxSizeChanged()
{
    searchFilter->setValueFromName(SearchResultFilter::FilterNames[SearchResultFilter::MAX_FILE_SIZE], maxSizeLine->text());
    proxyModel->setSearchResultFilter(searchFilter);
    emit numberOfSearchResultsChanged( this, numSearchResults, proxyModel->rowCount());
}

void SearchResultsView::filterSourceNumChanged()
{
    searchFilter->setValueFromName(SearchResultFilter::FilterNames[SearchResultFilter::MIN_SOURCE_NUM], sourceNumLine->text());
    proxyModel->setSearchResultFilter(searchFilter);
    emit numberOfSearchResultsChanged( this, numSearchResults, proxyModel->rowCount());
}

void SearchResultsView::filterFileTypeChanged()
{
    searchFilter->setValueFromName(SearchResultFilter::FilterNames[SearchResultFilter::FILE_TYPE], fileTypeCombo->currentText());
    proxyModel->setSearchResultFilter(searchFilter);
    emit numberOfSearchResultsChanged( this, numSearchResults, proxyModel->rowCount());
}


void SearchResultsView::attachSearchThread( SearchThread *sThread )
{
    this->sThread = sThread;
}

void SearchResultsView::customEvent( QEvent *e )
{
    int c;
    QModelIndex index;

    switch( e->type() ) {
        case KommuteDefs::EVENTTYPE_SEARCHRESULT:

            // search this hash in list :
            for (int i =0 ; i < SearchListModel->rowCount() ; i++)
            {
                if(SearchListModel->data(SearchListModel->index(i, SearchResultFilterModel::SEARCH_HASH)).toString()==static_cast<SearchResultEvent*>(e)->inFileHash)
                {
                    if(SearchListModel->data(SearchListModel->index(i, SearchResultFilterModel::SEARCH_HOSTHASH)).toString().contains(static_cast<SearchResultEvent*>(e)->inFileHostVirtualAddress))
                    {
                        // don't display two times same file with same host
                        return;
                    }
                    else
                    {
                        //TODO : add this host as a child ?
                        index = SearchListModel->index(i, SearchResultFilterModel::SEARCH_NBHOST);
                        //int nbSources= SearchListModel->data(SearchListModel->index(i,SEARCH_NBHOST)).toInt();
                        int nbSources= SearchListModel->data(index).toInt();
                        SearchListModel->setData(index, nbSources+1);
                        //sources = index.model()->index(index.row(),SEARCH_NBHOST).data().toDouble();
                        
                        /*if (nbSources <2.0)
                            setRowColor2(i, QString::fromUtf8("black"));
                        if (nbSources <3.0)
                            setRowColor2(i, QString::fromUtf8("darkBlue"));
                        else
                            setRowColor2(i, QString::fromUtf8("blue"));*/
                            
                        if ( nbSources < 1)
                        {
                          setRowColor2(i, QColor(0, 0, 19));
                        }
                        else if ( nbSources < 2)
                        {
                          setRowColor2(i, QColor(0, 0, 38));
                        }
                        else if ( nbSources < 3)
                        {
                          setRowColor2(i, QColor(0, 0, 57));
                        }	
                        else if ( nbSources < 4)
                        {
                          setRowColor2(i, QColor(0, 0, 76));
                        }
                        else if ( nbSources < 5)
                        {
                          setRowColor2(i, QColor(0, 0, 96));
                        }
                        else if ( nbSources < 6)
                        {
                          setRowColor2(i, QColor(0, 0, 114));
                        }
                        else if ( nbSources < 7)
                        {
                          setRowColor2(i, QColor(0, 0, 133));
                        }
                        else if ( nbSources < 8)
                        {
                          setRowColor2(i, QColor(0, 0, 152));	
                        }
                        else if ( nbSources < 9)
                        {
                          setRowColor2(i, QColor(0, 0, 171));
                        }
                        else if ( nbSources < 10)
                        {
                          setRowColor2(i, QColor(0, 0, 190));
                        }
                        else if ( nbSources < 11)
                        {
                          setRowColor2(i, QColor(0, 0, 209));
                        }
                        else if ( nbSources < 12)
                        {
                          setRowColor2(i, QColor(0, 0, 228));
                        }
                        else
                        {
                          setRowColor2(i, QColor(0, 0, 228));
                        }


                        index = SearchListModel->index(i, SearchResultFilterModel::SEARCH_HOST);
                        QString tmpData= SearchListModel->data(index).toString();
                        tmpData += " "+MuteLayer::getMnemonic(static_cast<SearchResultEvent*>(e)->inFileHostVirtualAddress);
                        SearchListModel->setData(index, tmpData);
                        index = SearchListModel->index(i, SearchResultFilterModel::SEARCH_HOSTHASH);
                        tmpData= SearchListModel->data(index).toString();
                        tmpData += " "+static_cast<SearchResultEvent*>(e)->inFileHostVirtualAddress;
                        SearchListModel->setData(index, tmpData);
                        return;
                    }
                }
            }
            c = SearchListModel->rowCount();
            SearchListModel->insertRow(c);
            index = SearchListModel->index(c, SearchResultFilterModel::SEARCH_NAME);
            SearchListModel->setData(index, static_cast<SearchResultEvent*>(e)->inFilePath);
            index = SearchListModel->index(c, SearchResultFilterModel::SEARCH_SIZE);
            SearchListModel->setData(index, static_cast<unsigned long long int>(static_cast<SearchResultEvent*>(e)->inFileLength));
            index = SearchListModel->index(c, SearchResultFilterModel::SEARCH_HASH);
            SearchListModel->setData(index, static_cast<SearchResultEvent*>(e)->inFileHash);

            index = SearchListModel->index(c, SearchResultFilterModel::SEARCH_NBHOST);
            SearchListModel->setData(index, 1);
            index = SearchListModel->index(c, SearchResultFilterModel::SEARCH_HOST);

            SearchListModel->setData(index, MuteLayer::getMnemonic(static_cast<SearchResultEvent*>(e)->inFileHostVirtualAddress));
            index = SearchListModel->index(c, SearchResultFilterModel::SEARCH_HOSTHASH);
            SearchListModel->setData(index, (static_cast<SearchResultEvent*>(e)->inFileHostVirtualAddress));

            {
                QString ext = QFileInfo(static_cast<SearchResultEvent*>(e)->inFilePath).suffix();
                KommuteDefs::FileType fileType = misc::fileType (ext);

                SearchListModel->setData(SearchListModel->index(c, SearchResultFilterModel::SEARCH_NAME), QIcon(KommuteDefs::FileTypeResourcePath[fileType]), Qt::DecorationRole);
                SearchListModel->setData(SearchListModel->index(c, SearchResultFilterModel::SEARCH_TYPE), KommuteDefs::tr(KommuteDefs::FileTypeName[fileType]));
            }
            numSearchResults++;
            emit numberOfSearchResultsChanged( this, numSearchResults, SearchListModel->rowCount() );
            break;
        default:
            break;
    }
}

QString SearchResultsView::getSearchTerms()
{
    return searchTerms;
}

void SearchResultsView::addFriend(QString fname,
    unsigned long fsize,
    QString fhash,
    QString fhost)
{
    emit grabFriendRequested(fname, fsize, fhash, fhost);
}

// Download the given item from search results list

void SearchResultsView::downloadSelectedItem(const QModelIndex& index)
{
    QString fname = index.sibling (index.row (), SearchResultFilterModel::SEARCH_NAME).data ().toString ();
    unsigned long fsize =	static_cast <unsigned long> ( index.sibling( index.row(), SearchResultFilterModel::SEARCH_SIZE).data().toULongLong() );
    QString fhash = index.sibling (index.row (), SearchResultFilterModel::SEARCH_HASH).data ().toString ();
    QString fhost = index.sibling (index.row (), SearchResultFilterModel::SEARCH_HOSTHASH).data ().toString ();

    fhost.remove(QRegExp(" .*"));

    emit downloadFileRequested(fname, fsize, fhash, fhost, -1, -1, -1, -1);
    setRowColor(index.row(), QString::fromUtf8("red"));
}

void SearchResultsView::downloadSelectedItems()
{
  QModelIndexList lst = searchtreeView->selectionModel()->selectedIndexes();
  for (int i = 0; i < lst.count (); i++)
        {
                if (lst[i].column () == 0)
                {
                        downloadSelectedItem(lst[i]);
                }
        }
}

// Set the color of a row in proxy model
void SearchResultsView::setRowColor(int row, QString color)
{
  for(int i=0; i<SearchListModel->columnCount(); ++i){
    proxyModel->setData(proxyModel->index(row, i), QVariant(QColor(color)), Qt::ForegroundRole);
  }
}

// Set the color of a row in data model
void SearchResultsView::setRowColor2(int row, QColor color)
{
  for(int i=0; i<SearchListModel->columnCount(); ++i){
    SearchListModel->setData(SearchListModel->index(row, i), QVariant(QColor(color)), Qt::ForegroundRole);
  }
}

// Sort the Search List
void SearchResultsView::sortSearchList(int index)
{
    static Qt::SortOrder sortOrder = Qt::AscendingOrder;

    if(searchtreeView->header()->sortIndicatorSection() == index)
    {
        if(sortOrder == Qt::AscendingOrder)
        {
            sortOrder = Qt::DescendingOrder;
        }
        else
        {
            sortOrder = Qt::AscendingOrder;
        }
    }
    searchtreeView->header()->setSortIndicator(index, sortOrder);

    switch(index)
    {
        case SearchResultFilterModel::SEARCH_SIZE:
            sortSearchListInt(index, sortOrder);
            break;
        default:
            sortSearchListString(index, sortOrder);
    }
}

void SearchResultsView::sortSearchListInt(int index, Qt::SortOrder sortOrder)
{
    QList<QPair<int, qlonglong> > lines;

    // Insertion sorting
    for(int i=0; i<SearchListModel->rowCount(); ++i)
    {
        misc::insertSort(lines, QPair<int,qlonglong>(i, SearchListModel->data(SearchListModel->index(i, index)).toLongLong()), sortOrder);
    }

    // Insert items in new model, in correct order
    int nbRows_old = lines.size();

    for(int row=0; row<lines.size(); ++row)
    {
        SearchListModel->insertRow(SearchListModel->rowCount());
        int sourceRow = lines[row].first;
        for(int col = 0; col < SearchResultFilterModel::NUMBER_OF_SEARCH_COLUMN; ++col)
        {
            SearchListModel->setData(SearchListModel->index(nbRows_old+row, col), SearchListModel->data(SearchListModel->index(sourceRow, col)));
            SearchListModel->setData(SearchListModel->index(nbRows_old+row, col), SearchListModel->data(SearchListModel->index(sourceRow, col), Qt::ForegroundRole), Qt::ForegroundRole);
        }
    }
    // Remove old rows
    SearchListModel->removeRows(0, nbRows_old);
}

void SearchResultsView::sortSearchListString(int index, Qt::SortOrder sortOrder)
{
    QList<QPair<int, QString> > lines;

    // Insetion sorting
    for(int i=0; i<SearchListModel->rowCount(); ++i)
    {
        misc::insertSortString(lines, QPair<int, QString>(i, SearchListModel->data(SearchListModel->index(i, index)).toString()), sortOrder);
    }

    // Insert items in new model, in correct order
    int nbRows_old = lines.size();
    for(int row=0; row<nbRows_old; ++row)
    {
        SearchListModel->insertRow(SearchListModel->rowCount());
        int sourceRow = lines[row].first;
        for(int col = 0; col < SearchResultFilterModel::NUMBER_OF_SEARCH_COLUMN; ++col)
        {
            SearchListModel->setData(SearchListModel->index(nbRows_old+row, col), SearchListModel->data(SearchListModel->index(sourceRow, col)));
            SearchListModel->setData(SearchListModel->index(nbRows_old+row, col), SearchListModel->data(SearchListModel->index(sourceRow, col), Qt::ForegroundRole), Qt::ForegroundRole);
        }
    }
    // Remove old rows
    SearchListModel->removeRows(0, nbRows_old);
}

// Save columns width in a file to remember them
// (download list)
void SearchResultsView::saveColWidthSearchList() const
{
    #ifdef DEBUG
    qDebug("Saving columns width in search list");
    #endif

    QSettings settings("Kommute", "Kommute");
    QStringList width_list;

    for(int i=0; i<SearchListModel->columnCount(); ++i)
    {
        width_list << misc::toQString(searchtreeView->columnWidth(i));
    }
    settings.setValue("SearchListColsWidth", width_list.join(" "));

    #ifdef DEBUG
    qDebug("Search list columns width saved");
    #endif
}


// Load columns width in a file that were saved previously
// (search list)
bool SearchResultsView::loadColWidthSearchList()
{
    #ifdef DEBUG
    qDebug("Loading columns width for search list");
    #endif

    QSettings settings("Kommute", "Kommute");
    QString line = settings.value("SearchListColsWidth", QString()).toString();

    if(line.isEmpty())
        return false;

    QStringList width_list = line.split(' ');

    if(width_list.size() != SearchListModel->columnCount())
        return false;

    for(int i=0; i<width_list.size(); ++i)
    {
        searchtreeView->header()->resizeSection(i, width_list.at(i).toInt());
    }

    #ifdef DEBUG
    qDebug("Search list columns width loaded");
    #endif

    return true;
}

QVariant SearchResultsView::data(const QModelIndex& index, int role) const {

    int col = index.column();

    switch (role) {
        case Qt::DisplayRole:
        case Qt::EditRole:
        case Qt::TextColorRole:

        case Qt::TextAlignmentRole:
            if (col == SearchResultFilterModel::SEARCH_NAME)
                return int(Qt::AlignLeft | Qt::AlignVCenter);
            else if (col == SearchResultFilterModel::SEARCH_SIZE)
                return int(Qt::AlignRight | Qt::AlignVCenter);
            else if (col == SearchResultFilterModel::SEARCH_NBHOST)
                return int(Qt::AlignRight | Qt::AlignVCenter);
            else
                return QVariant();

        case Qt::FontRole:
        {
            QFont font = QFont("Helvetica", 9);
            return qVariantFromValue(QFont("Helvetica", 9));
        }
        case Qt::BackgroundColorRole:
        case Qt::CheckStateRole:
        case Qt::SizeHintRole:
        case Qt::DecorationRole:
            return QVariant();
    }
    return QVariant();
}

void SearchResultsView::changeEvent (QEvent* event)
{
    if (event->type () == QEvent::LanguageChange)
    {
        // apply the new language
        // all designed forms have retranslateUi() method
        retranslateUi (this);
        // retranslate other widgets which weren't added in the designer
        retranslate ();
    }

    QWidget::changeEvent (event);
}

void SearchResultsView::retranslate ()
{
    retranslateSearchList ();
}


