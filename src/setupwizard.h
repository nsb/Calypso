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

#ifndef SETUPWIZARD_H
#define SETUPWIZARD_H
#define DEBUG

#include <QDialog>
#include <QString>
#include <QSettings>
#include <QStringListModel>

#include "ui_setupwizard.h"

/**
@author Andy Gebauer - feivelda@googlemail.com
*/

class SetupWizard : public QDialog, private Ui::SetupWizard
{
    Q_OBJECT

public:
    SetupWizard(QWidget *parent = 0, Qt::WFlags flags = 0);
    ~SetupWizard();
    void reset(QSettings *settingsPointer);

private:
    QSettings *settings;
    bool lastStep;
    bool firewall_true_false;
    QStringList otherSharedPathSetting;
    int keySize;
    QString seed_String;
    void writeSettings();
    QStringListModel otherShareModel;

/*
* - labelstackedWidget              stackedWidget   - text displayed above in the upper left corner.
* - textstackedWidget               stackedWidget   - Objects displayed in the middle of the wizard. Connected with labelstackedWidget. If you move textstackedWidget forwards or backwards, labelstackedWidget will also move
* - backButton                      QPushButton     - to push the textstackedWidget backward
* - cancelButton                    QPushButton     - to cancel the wizard
* - helpButton                      QPushButton     - to display a help window
* - nextButton                      QPushButton     - to push the textstackedWidget forward
* - fw_no_radioButton               QRadioButton    - if you select, that you aren't behind a firewall, this button shall be checked
* - fw_yes_radioButton              QRadioButton    - if you select, that you are behind a firewall, this button shall be checked
* - otherShareDirPushButton_3       QPushButton     - to select a directory for the share_lineEdit through a QFileDialog
* - suppressSharePushButton_3       QPushButton     - to remove a directory for the share_lineEdit through a QFileDialog
* - key512_radioButton              QRadioButton    - to select that you wish a 512 bit key
* - key1024_radioButton             QRadioButton    - to select that you wish a 1024 bit key
* - key2048_radioButton             QRadioButton    - to select that you wish a 2048 bit key
* - key4096_radioButton             QRadioButton    - to select that you wish a 4096 bit key
* - seed_lineEdit                   QLineEdit       - to give a starting point for the seed generation
*/

private slots:
    void on_nextButton_clicked();
    void on_backButton_clicked();
    void on_cancelButton_clicked();
    void on_helpButton_clicked();
    void on_otherShareDirPushButton_3_clicked();
    void on_suppressSharePushButton_3_clicked();
    void otherShareDirListViewCostumPopupMenu( QPoint point );
    void changeEvent (QEvent* event);
};

#endif

