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

#include <QMessageBox>

#include "ConnectDialog.h"
#include "kommute.h"
#include "MUTE/layers/messageRouting/messageRouter.h"
//#include "config/kconfig.h"

/* Define the format used for displaying the date and time */
#define DATETIME_FMT "MMM dd hh:mm:ss"

#define IMAGE_INFO ":/images/informations_16x16.png"

/** Default constructor */
ConnectDialog::ConnectDialog(QWidget *parent, Qt::WFlags flags)
  : QMainWindow(parent, flags)
{
    /* Invoke Qt Designer generated QObject setup routine */
    ui.setupUi(this);

    connect (ui.pushButton,SIGNAL(clicked()),this,SLOT(addhost()));

    //KConfig config;
    //config.loadWidgetInformation(this);

    // Create the status bar
    statusBar()->showMessage("Please enter a ip adress or a host name");

    setFixedSize(QSize(317, 88));
}

/**
 Overloads the default show() slot so we can set opacity
 */
void ConnectDialog::show()
{
    if (!this->isVisible()) {
        QMainWindow::show();
    }
    else
    {
        QMainWindow::activateWindow();
        setWindowState(windowState() & ~Qt::WindowMinimized | Qt::WindowActive);
        QMainWindow::raise();
    }
}

void ConnectDialog::closeEvent (QCloseEvent * event)
{
    //KConfig config;
    //config.saveWidgetInformation(this);

    QWidget::closeEvent(event);
}

void ConnectDialog::addhost ()
{
    QString text = ui.lineEdit->text();

    if(text.isEmpty())
    {
        /* error message */
        QMessageBox::warning(this, tr("Kommute"),
                   tr("Please Add a IP or Address"),
                   QMessageBox::Ok, QMessageBox::Ok);

        return; //Don't Add a empty Host!!
    }

    printf("add host %s to host list\n",ui.lineEdit->text().toLatin1().data());
    ((Kommute *)parent())->setLogInfo("Add Host "+ui.lineEdit->text()+":"+ QString::number( ui.spinBox->value() ) +" to host list.",5000);
    muteAddHost(ui.lineEdit->text().toLatin1().data() ,ui.spinBox->value());

    QMainWindow::close();
    ui.lineEdit->clear();
}

