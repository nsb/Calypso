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

#include "listdialog.h"

#include <QStringListModel>
#include <QMessageBox>
#include <QUrl>

#ifdef DEBUG
#include <QtDebug>
#endif

/**
@author Andy Gebauer - feivelda@googlemail.com
*/

/*  QListView listView
    QLineEdit itemLineEdit
    QPushButton addButton
    QPushButton deleteButton
    QPushButton acceptButton
    QPushButton cancelButton*/

ListDialog::ListDialog(QWidget *parent, Qt::WFlags flags)
: QDialog(parent, flags)
{
    setupUi(this);
}

ListDialog::~ListDialog()
{
}

void ListDialog::setData( QStringList data )
{
    // create a new QStringListModel with the QStringList given by data
    stringListModel = new QStringListModel( data );
    // set the stringListModel as model for listView
    listView->setModel( stringListModel );
}

QStringList ListDialog::getData()
{
    // return the stringList
    return stringListModel->stringList();
}

void ListDialog::on_addButton_clicked()
{
    // testing if data is valid
    // create new url object from the string
    QUrl *url = new QUrl( itemLineEdit->text() );
    // clear the lineEdit
    itemLineEdit->clear();
    // test if the url is valid.
    // this means, QUrl tests, if the url consists of something like http://www.foo.bar
    // not all fields must be given, but it should be an url, so minimum should be foo.bar
    if ( !url->isValid() )
    {
        QMessageBox::information( this, tr("Invalid URL"),
                tr("The given URL is invalid") + "\n" + url->toString(),
                QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
        return;
    }
    // adding the Url to the beginning of the list
    QModelIndex modelIndex = stringListModel->index( 0 );
    stringListModel->insertRow( 0 );
    stringListModel->setData( modelIndex, url->toString() );
}

void ListDialog::on_deleteButton_clicked()
{
    int row = listView->currentIndex().row();
    stringListModel->removeRows( row, 1 );
}

void ListDialog::changeEvent (QEvent* event)
{
    if (event->type () == QEvent::LanguageChange)
    {
        // apply the new language
        // all designed forms have retranslateUi() method
        retranslateUi (this);
    }

    QWidget::changeEvent (event);
}

