/****************************************************************
 *  Kommute is distributed under the following license:
 *
 *  Copyright (C) 2006-2008, defnax
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, 
 *  Boston, MA  02110-1301, USA.
 ****************************************************************/

#ifndef _COMMENTSDIALOG_H
#define _COMMENTSDIALOG_H

#include <QtGui>
#include <QObject>
#include <QModelIndex>
#include <QVariant>

#include "ui_CommentsDialog.h"


class CommentsDialog : public QDialog
{
  Q_OBJECT

public:
  /** Default constructor */
 CommentsDialog(QWidget *parent = 0, Qt::WFlags flags = 0);
  /** Default destructor */
  void setComm(QString str) {ui.lineEdit->setText(str);};
  QString getComm() { return ui.lineEdit->text();};


public slots:
  /** Overloaded QWidget.show */
  void show();
  
  void setLink(const QString & link);


protected:
  void closeEvent (QCloseEvent * event);

  
private slots:
  void on_ok_pushButton_clicked();
  void on_cancel_pushButton_clicked();
  
private:

	class QStandardItemModel *CommentModel;

    /** Loads the saved connectdialog settings */
	//  void loadSettings();
 

  
  /** Qt Designer generated object */
  Ui::CommentsDialog ui;
};

#endif

