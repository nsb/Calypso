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

#include <qmap.h>
#include <QListWidgetItem>
#include "ui_searchview.h"

typedef QMap<QString, QString> FriendsMap;

class QHideEvent;

class SearchView : public QWidget, private Ui::SearchView
{
    Q_OBJECT

public:
    SearchView(QWidget *parent = 0);
    void hideEvent( QHideEvent * );
    void contextMenuEvent( QContextMenuEvent* );

public slots:
    void addFriend( QString );
    void removeFriend( );
signals:
    void startSearch( QString address, QString s );
    void visible(bool);
private:
    FriendsMap map;

    /** Contains the mapping of filetype combobox to filetype extensions */
    static const int FILETYPE_IDX_ANY;
    static const int FILETYPE_IDX_AUDIO;
    static const int FILETYPE_IDX_VIDEO;
    static const int FILETYPE_IDX_PICTURE;
    static const int FILETYPE_IDX_PROGRAM;
    static const int FILETYPE_IDX_ARCHIVE;
    static const int FILETYPE_IDX_DOCUMENT;

    static QMap<int, QString> * FileTypeExtensionMap;
    static bool initialised;
    void initialiseFileTypeMappings();

private slots:
    void startSearch();
    void changeEvent (QEvent* event);
	void on_clearButton_clicked();
};
