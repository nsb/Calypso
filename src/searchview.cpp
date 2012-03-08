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

#include "searchview.h"
#include "mutelayer.h"

#include <QtDebug>
#include <QMenu>
#include <QContextMenuEvent>

/* These indices MUST be identical to their equivalent indices in the combobox */
const int SearchView::FILETYPE_IDX_ANY = 0;
const int SearchView::FILETYPE_IDX_AUDIO = 1;
const int SearchView::FILETYPE_IDX_VIDEO = 2;
const int SearchView::FILETYPE_IDX_PICTURE = 3;
const int SearchView::FILETYPE_IDX_PROGRAM = 4;
const int SearchView::FILETYPE_IDX_ARCHIVE = 5;
const int SearchView::FILETYPE_IDX_DOCUMENT = 6;
QMap<int, QString> * SearchView::FileTypeExtensionMap = new QMap<int, QString>();
bool SearchView::initialised = false;


SearchView::SearchView(QWidget *parent)
  : QWidget(parent)
{
    setupUi(this);

    /* initialise the filetypes mapping */
    if (!SearchView::initialised)
    {
	initialiseFileTypeMappings();
    }

    connect(pushButton, SIGNAL(clicked()), this, SLOT(startSearch()));
//    connect(friendsList, SIGNAL(rightButtonPressed( QListBoxItem*, const QPoint &)), this, SLOT(removeFriend( QListBoxItem*, const QPoint &)));
    friendsList->setCurrentRow(0);
}

void SearchView::startSearch()
{
    QString address;

    if( friendsList->currentRow() == 0 )
    {
        address = "ALL";
    }
    else
    {
        address = map[friendsList->currentItem()->text()];
    }

    emit startSearch( address, lineEdit->text());
    lineEdit->clear();
}

void SearchView::hideEvent( QHideEvent *e )
{

    emit visible(false);
}

void SearchView::addFriend( QString name )
{
    QString mnemonic = MuteLayer::getMnemonic( name );

    if( !map.contains( mnemonic ))
    {
        if( friendsList->count() > 10 )
        {
            map.remove(friendsList->item(1)->text());
            delete friendsList->item(1);
        }

        map[mnemonic] = name;
	friendsList->addItem(mnemonic);
    }
}

void SearchView::contextMenuEvent( QContextMenuEvent* e)
{
   QPoint  p(e->x()-friendsList->x(),e->y()-friendsList->y());
   QListWidgetItem* item = friendsList->itemAt( p );
    if( item != NULL && friendsList->row( item ) != 0 )
    {
        QMenu popMenu(this);
	QAction *actionRemove = popMenu.addAction(tr("Remove"));
	actionRemove->setIcon (QIcon (":/resources/delete.png"));
	connect(actionRemove, SIGNAL(triggered()), this, SLOT(removeFriend()));
        friendsList->setCurrentItem(item);
        popMenu.setMouseTracking(true);
        popMenu.exec(QCursor::pos());
    }
}


void SearchView::removeFriend()
{
    QListWidgetItem *item = friendsList->currentItem();
    if( item != NULL && friendsList->row( item ) != 0 )
    {
      delete item;
    }
}

void SearchView::initialiseFileTypeMappings()
{
    /* edit these strings to change the range of extensions recognised by the search */
    SearchView::FileTypeExtensionMap->insert(FILETYPE_IDX_ANY, "");
    SearchView::FileTypeExtensionMap->insert(FILETYPE_IDX_AUDIO,
        "aac aif iff m3u mid midi mp3 mpa ogg ra ram wav wma");
    SearchView::FileTypeExtensionMap->insert(FILETYPE_IDX_VIDEO,
        "3gp asf asx avi mov mp4 mpeg mpg qt rm swf vob wmv");
    SearchView::FileTypeExtensionMap->insert(FILETYPE_IDX_PICTURE,
        "3dm 3dmf ai bmp drw dxf eps gif ico indd jpe jpeg jpg mng pcx pcc pct pgm "
        "pix png psd psp qxd qxprgb sgi svg tga tif tiff xbm xcf");
    SearchView::FileTypeExtensionMap->insert(FILETYPE_IDX_PROGRAM,
        "app bat cgi com bin exe js pif py pl sh vb ws ");
    SearchView::FileTypeExtensionMap->insert(FILETYPE_IDX_ARCHIVE,
        "7z bz2 gz pkg rar sea sit sitx tar zip");
    SearchView::FileTypeExtensionMap->insert(FILETYPE_IDX_DOCUMENT,
        "doc odt ott rtf pdf ps txt log msg wpd wps" );
    SearchView::initialised = true;
}

void SearchView::changeEvent (QEvent* event)
{
    if (event->type () == QEvent::LanguageChange)
    {
        // apply the new language
        // all designed forms have retranslateUi() method
        retranslateUi (this);
    }

    QWidget::changeEvent (event);
}
void SearchView::on_clearButton_clicked()
{
	lineEdit->clear();
}
