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

#ifndef LISTDIALOG_H
#define LISTDIALOG_H
#define DEBUG

class QStringListModel;

#include "ui_listdialog.h"

/**
@author Andy Gebauer - feivelda@googlemail.com
*/

/*  QListView listView
    QLineEdit itemLineEdit
    QPushButton addButton
    QPushButton deleteButton
    QPushButton acceptButton
    QPushButton cancelButton*/

class ListDialog : public QDialog, private Ui::ListDialog
{
    Q_OBJECT

public:
    ListDialog(QWidget *parent = 0, Qt::WFlags flags = 0);
    ~ListDialog();
    void setData( QStringList data );
    QStringList getData();

private slots:
    void on_addButton_clicked();
    void on_deleteButton_clicked();
    void changeEvent (QEvent* event);

private:
    QStringListModel *stringListModel;
};

#endif

