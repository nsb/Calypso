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
#ifndef SEARCHRESULTSVIEW_H
#define SEARCHRESULTSVIEW_H

#include <QString>
#include <QWidget>
#include <QEvent>
#include <QModelIndex>
#include <QListWidgetItem>

#include "ui_searchresultsview.h"

class SearchThread;
class CloseSearchThreadQueue;
class QContextMenuEvent;
class QStandardItemModel;
class SearchListDelegate;
class Download;
class SearchResultFilterModel;
class SearchResultFilter;

class SearchResultsView : public QWidget, private Ui::SearchResultsView
{
    Q_OBJECT

public:
    SearchResultsView(QWidget *parent = 0, QString address = "ALL", QString searchTerms = "", CloseSearchThreadQueue *searchThreadQueue = 0);

    ~SearchResultsView();

    void attachSearchThread( SearchThread * );

    void customEvent( QEvent *e );

    QString getSearchTerms();

    bool loadColWidthSearchList();
    void saveColWidthSearchList() const;
    //void setRowColor(int row, QString color);
    void sortSearchList(int index);
    void sortSearchListInt(int index, Qt::SortOrder sortOrder);
    void sortSearchListString(int index, Qt::SortOrder sortOrder);
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;


protected slots:
    void setRowColor(int row, QString color);
    void setRowColor2(int row, QColor color);
    void downloadSelectedItem(const QModelIndex& index);
    void downloadSelectedItems();



signals:
    void downloadItemRequested(const QModelIndex& index);
    void downloadFileRequested(QString fname,
                               unsigned long fsize,
                               QString fhash,
                               QString fhost,int,int,int,int);
    void grabFriendRequested(QString fname,
                             unsigned long fsize,
                             QString fhash,
                             QString fhost);

    void numberOfSearchResultsChanged(QWidget*, int, int);

public slots:
    void addFriend(QString fname,
                  unsigned long fsize,
                  QString fhash,
                  QString fhost);
    void filterRegExpChanged();
    void filterMinSizeChanged();
    void filterMaxSizeChanged();
    void filterSourceNumChanged();
    void filterFileTypeChanged();
    void stopSearchButtonClicked();
    void startSearchButtonClicked();

private slots:
    void on_clearfilter_Button_clicked();

    void changeEvent (QEvent* event);
    void searchStopped();
    void searchStarted();


private:
    void retranslate ();
    void initSearchList ();
    void retranslateSearchList ();

    SearchThread *sThread;
    bool active;
    CloseSearchThreadQueue *searchThreadQueue;
    uint numSearchResults;
    QString address, searchTerms;

    QAbstractItemModel *SearchListModel;
    SearchListDelegate *SearchDelegate;

    QModelIndex lastIndex;
    SearchResultFilterModel *proxyModel;
    SearchResultFilter *searchFilter;
};

#endif
