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

#ifndef SETTINGSGUI_H
#define SETTINGSGUI_H
#define DEBUG

#include "ui_settingsgui.h"
#include <QStringListModel>

/**
@author Andy Gebauer - feivelda@googlemail.com
*/

class SettingsGui : public QDialog, private Ui::SettingsGui
{
    Q_OBJECT

/*
    cancel_Button
    help_Button
    ok_Button
    itemListWidget
    stackedWidget
    loadListStartup_CheckBox
    downloadLocations_PushButton
    editWebHostCachesList_PushButton
    wizardAgain_PushButton
    inboundValueSpinBox
    outboundValueSpinBox
    maintainAtLeastSpinBox
    allowAtMostSpinBox
    firewall_no_radioButton
    firewall_yes_radioButton
    portValueSpinBox
    timeoutValueSpinBox
    maxDownloadsValueSpinBox
    incomingDirPushButton
    tempDirPushButton
    languageComboBox
*/

public:
    SettingsGui(QWidget *parent = 0, Qt::WFlags flags = 0);
    ~SettingsGui();


protected:
  void closeEvent (QCloseEvent * event);
  void changeEvent (QEvent* event);

private slots:
    void on_wizardAgain_PushButton_clicked();
    void on_ok_Button_clicked();
    void on_cancel_Button_clicked();
    void on_editWebHostCachesList_PushButton_clicked();
    void on_downloadLocations_PushButton_clicked();
    void on_incomingDirPushButton_clicked();
    void on_tempDirPushButton_clicked();
    void on_otherShareDirPushButton_clicked();
    void on_suppressSharePushButton_clicked();
    void on_styleCombo_activated(const QString &styleName);
    void on_styleSheetCombo_activated(const QString &styleSheetName);
    void otherShareDirListViewCostumPopupMenu( QPoint point );

    void setLanguage (int index);

private:
    void setSettings();
    void parseSettings();
    void loadStyleSheet(const QString &sheetName);
    void retranslate ();
    void initializeLanguageCombo ();

private:
    class Settings *settings;
    QStringListModel otherShareModel;
    /** Define the popup menus for the Context menu */
    QMenu* contextMnu;
    /** Defines the actions for the context menu */
    QAction* removeAct;
};

#endif

