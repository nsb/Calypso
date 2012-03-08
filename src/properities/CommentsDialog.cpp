/****************************************************************
 *  Kommute is distributed under the following license:
 *
 *  Copyright (C) 2006-2008,  defnax
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

#include "CommentsDialog.h"
//#include "config/kconfig.h"

#include <QAction>
#include <QTreeView>
#include <QList>
#include <QtDebug>
#include <QContextMenuEvent>
#include <QMenu>
#include <QCursor>
#include <QPoint>
#include <QMouseEvent>
#include <QPixmap>
#include <QHeaderView>
#include <QModelIndex>
#include <QStandardItemModel>


/** Default constructor */
CommentsDialog::CommentsDialog(QWidget *parent, Qt::WFlags flags)
  : QDialog(parent, flags)
{
	/* Invoke Qt Designer generated QObject setup routine */
	ui.setupUi(this);

	//KConfig config;
	//config.loadWidgetInformation(this);
 
    CommentModel = new QStandardItemModel(0, 3);
    CommentModel->setHeaderData(0, Qt::Horizontal, tr("Rating"));
    CommentModel->setHeaderData(1, Qt::Horizontal, tr("Comments"));
    CommentModel->setHeaderData(2, Qt::Horizontal, tr("File Name"));
    
    ui.commentTreeView->setModel(CommentModel);
    ui.commentTreeView->setSortingEnabled(true);
    ui.commentTreeView->setRootIsDecorated(false);
    
    /* Set header resize modes and initial section sizes */
	QHeaderView * _coheader = ui.commentTreeView->header();
	_coheader->setResizeMode ( 0, QHeaderView::Custom);
    _coheader->resizeSection ( 0, 100 );
	_coheader->resizeSection ( 1, 240 );
	_coheader->resizeSection ( 2, 100 );

 
 
}


void
CommentsDialog::show()
{
    if (!this->isVisible()) {
    QDialog::show();
  } else {
    QDialog::activateWindow();
    setWindowState(windowState() & ~Qt::WindowMinimized | Qt::WindowActive);
    QDialog::raise();
  }
}

void CommentsDialog::on_ok_pushButton_clicked()
{
    QDialog::close();
}

void CommentsDialog::on_cancel_pushButton_clicked()
{
    //reject();
    QDialog::close();
}

void CommentsDialog::closeEvent (QCloseEvent * event)
{
 //KConfig config;
 //config.saveWidgetInformation(this);

 QWidget::closeEvent(event);
}

void CommentsDialog::setLink(const QString & link) 
{
	ui.linkstextEdit->setText(link);
}