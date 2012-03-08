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

#ifndef _CONNECTDIALOG_H
#define _CONNECTDIALOG_H

#include "ui_ConnectDialog.h"


class ConnectDialog : public QMainWindow
{
  Q_OBJECT

public:
  /** Default constructor */
  ConnectDialog(QWidget *parent = 0, Qt::WFlags flags = 0);
  /** Default destructor */


public slots:
  /** Overloaded QWidget.show */
  void show();

protected:
  void closeEvent (QCloseEvent * event);
  
private slots:
  void addhost();

  
private:

  /** Loads the saved connectdialog settings */
//  void loadSettings();
 

  
  /** Qt Designer generated object */
  Ui::ConnectDialog ui;
};

#endif

