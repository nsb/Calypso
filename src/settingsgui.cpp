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

#include "settingsgui.h"
#include "settings.h"
#include "settingshelper.h"
#include "listdialog.h"

#include "KommuteApplication.h"
#include "kommute.h"


#include <QMessageBox>
#include <QtGui>

#ifdef DEBUG
#include <QtDebug>
#endif

/* Images for context menu icons */
#define IMAGE_CANCEL        ":/delete.png"

/**
@author Andy Gebauer - feivelda@googlemail.com
*/

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

SettingsGui::SettingsGui(QWidget *parent, Qt::WFlags flags)
: QDialog(parent, flags)
{
    setupUi(this);
    // parent object is the settingsgui, but we're not at app start
    settings = new Settings(this, false);

    QRegExp regExp("Q(.*)Style");
    QString defaultStyle = QApplication::style()->metaObject()->className();

    if (defaultStyle == QLatin1String("QMacStyle"))
        defaultStyle = QLatin1String("Macintosh (Aqua)");
    else if (regExp.exactMatch(defaultStyle))
        defaultStyle = regExp.cap(1);

    styleCombo->addItems(QStyleFactory::keys());
    styleCombo->setCurrentIndex(styleCombo->findText(defaultStyle));
    styleSheetCombo->setCurrentIndex(styleSheetCombo->findText("Default"));
    otherShareDirListView->setModel(&otherShareModel);

    setSettings();
    // set to basic tab. Because of the signal emitted when the row is changed, the stackedWidget is also set
    itemListWidget->setCurrentRow( 0 );

    initializeLanguageCombo ();
    connect( otherShareDirListView, SIGNAL( customContextMenuRequested( QPoint ) ), this, SLOT( otherShareDirListViewCostumPopupMenu( QPoint ) ) );
}

void SettingsGui::initializeLanguageCombo ()
{
    int index = 0;

    foreach (QString localeStr, KommuteApplication::availableLanguages ())
    {
        QLocale locale (localeStr);
        languageComboBox->addItem (QLocale::countryToString (locale.country ()), localeStr);

        if (KommuteApplication::getLanguage () == localeStr)
            languageComboBox->setCurrentIndex (index);

        ++index;
    }
    connect (languageComboBox, SIGNAL (activated (int)), this, SLOT (setLanguage (int)));
}

SettingsGui::~SettingsGui()
{
}

void SettingsGui::otherShareDirListViewCostumPopupMenu( QPoint point )
{
      QMenu contextMnu( this );
      QMouseEvent *mevent = new QMouseEvent( QEvent::MouseButtonPress, point, Qt::RightButton, Qt::RightButton, Qt::NoModifier );

      removeAct = new QAction(QIcon(IMAGE_CANCEL), tr( "Remove" ), this );
      connect( removeAct , SIGNAL( triggered() ), this, SLOT( on_suppressSharePushButton_clicked() ) );

      contextMnu.clear();
      contextMnu.addAction( removeAct );
      contextMnu.exec( mevent->globalPos() );
}

void SettingsGui::on_wizardAgain_PushButton_clicked()
{
    int code = QMessageBox::question(this,tr("Set First Start again"),
                tr("Do you really want to show the SetupWizard again on next startup?") + "\n"
                + tr("You must know, this will also delete your key and making the key will use some time."),
                QMessageBox::Yes, QMessageBox::No, QMessageBox::NoButton);
    if ( QMessageBox::Yes == code )
    {
        // set the first start
        settings->setFirstStartSetting(true);
    }
}

void SettingsGui::setSettings()
{
    // network settings
    inboundValueSpinBox->setValue( settings->getInboundKBLimitSetting() );
    outboundValueSpinBox->setValue( settings->getOutboundKBLimitSetting() );
    maintainAtLeastSpinBox->setValue( settings->getTargetConnectionCountSetting() );
    allowAtMostSpinBox->setValue( settings->getMaxConnectionCountSetting() );
    timeoutValueSpinBox->setValue( settings->getDownloadTimeoutMilliSecondsSetting() / 1000 );
    portValueSpinBox->setValue( settings->getPortNumberSetting() );

    if ( settings->getFirewallSetting() )
    {
        firewall_yes_radioButton->setChecked( true );
    }
    else
    {
        firewall_no_radioButton->setChecked( true );
    }

    // share settings
    incomingDirLineEdit->setText( settings->getIncomingDirectory() );
    tempDirLineEdit->setText( settings->getTempDirectory() );
    maxDownloadsValueSpinBox->setValue( settings->getMaxSimultaneousDownloadsSetting() );
    otherShareModel.setStringList(settings->getOtherSharedPathSetting());

    // basic settings
    loadListStartup_CheckBox->setChecked( settings->getReloadWebHostCachesSetting() );
    downloadLocations_PushButton->setEnabled( loadListStartup_CheckBox->isChecked() );
    enableWebHostCache_CheckBox->setChecked( settings->getEnableWebHostCache() );
    sendSeedNodes_CheckBox->setChecked( settings->getSendSeedNodes() );
    acceptSeedNodes_CheckBox->setChecked( settings->getAcceptSeedNodes() );
    checkWinAutostart_CheckBox->setChecked( settings->getChkAutorunWithWin() );
    logComboBox->setCurrentIndex(settings->getLogLevelSetting());

#ifndef Q_WS_WIN
    setupGroup->setVisible(false);
#endif

    // style settings
    styleCombo->setCurrentIndex(styleCombo->findText(settings->getStyleName()));
    styleSheetCombo->setCurrentIndex(styleSheetCombo->findText(settings->getSheetName()));
    searchWithDownloadsCheckBox->setCheckState(settings->getSearchWithDownloads()? (Qt::Checked) : (Qt::Unchecked));

   // proxy settings
   proxyHostLineEdit->setText(settings->getProxyHost());
   proxyPortSpinBox->setValue(settings->getProxyPort());
   proxyUserLineEdit->setText(settings->getProxyUser());
   proxyPasswordLineEdit->setText(settings->getProxyPassword());
}

void SettingsGui::parseSettings()
{
    // read all the spin boxes and use the set methods in the settingsclass
    // network settings
    settings->setInboundKBLimitSetting( inboundValueSpinBox->value() );
    settings->setOutboundKBLimitSetting( outboundValueSpinBox->value() );
    settings->setTargetConnectionCountSetting( maintainAtLeastSpinBox->value() );
    settings->setMaxConnectionCountSetting( allowAtMostSpinBox->value() );
    settings->setDownloadTimeoutMilliSecondsSetting( timeoutValueSpinBox->value() * 1000 ); // *1000 because we have ms
    settings->setPortNumberSetting( portValueSpinBox->value() );
    settings->setFirewallSetting( firewall_yes_radioButton->isChecked() );

    // share settings
    settings->setOtherSharedPathSetting (otherShareModel.stringList ());
    settings->setIncomingDirectory( incomingDirLineEdit->text() );
    settings->setTempDirectory( tempDirLineEdit->text() );
    settings->setMaxSimultaneousDownloadsSetting( maxDownloadsValueSpinBox->value() );

    // basic settings
    settings->setReloadWebHostCachesSetting( loadListStartup_CheckBox->isChecked() );
    settings->setEnableWebHostCache( enableWebHostCache_CheckBox->isChecked() );
    settings->setSendSeedNodes( sendSeedNodes_CheckBox->isChecked() );
    settings->setAcceptSeedNodes( acceptSeedNodes_CheckBox->isChecked() );
    settings->setChkAutorunWithWin( checkWinAutostart_CheckBox->isChecked());
    settings->setLogLevelSetting( logComboBox->currentIndex());

    // style settings
    settings->setStyleName( styleCombo->currentText());
    settings->setSheetName( styleSheetCombo->currentText());
    settings->setSearchWithDownloads(searchWithDownloadsCheckBox->checkState()==Qt::Checked);

    // proxy settings
    settings->setProxyHost( proxyHostLineEdit->text());
    settings->setProxyPort( proxyPortSpinBox->value());
    settings->setProxyUser( proxyUserLineEdit->text());
    settings->setProxyPassword( proxyPasswordLineEdit->text());
}

void SettingsGui::on_ok_Button_clicked()

{
    parseSettings();
    settings->saveSettings();
#ifdef Q_WS_WIN
    Kommute ::updateAutorunRegistrySetting ( checkWinAutostart_CheckBox->isChecked());
#endif
    QDialog::close();

    //Kommute :: updateAutorunRegistrySetting (  Kommute :: checkIfAutorunWithWindows ( ) );



    //accept();
}

void SettingsGui::on_cancel_Button_clicked()
{
    //reject();
    QDialog::close();
}

void SettingsGui::on_editWebHostCachesList_PushButton_clicked()
{
    ListDialog *listDialog = new ListDialog();
    listDialog->setData( settings->getWebHostCachesSetting() );
    listDialog->setWindowTitle( tr("Edit list of WebHostCaches") );
    // end if the dialog was closed with reject
    if ( QDialog::Accepted != listDialog->exec() )
    {
        return;
    }
    // write the data of the list dialog to the settings
    settings->setWebHostCachesSetting( listDialog->getData() );
}

void SettingsGui::on_downloadLocations_PushButton_clicked()
{
    ListDialog *listDialog = new ListDialog();
    listDialog->setWindowTitle( tr("Edit download locations of WebHostCaches") );
    listDialog->setData( settings->getDownloadWebHostCachesSetting() );
    // end if the dialog was closed with reject
    if ( QDialog::Accepted != listDialog->exec() )
    {
        return;
    }
    // write the data of the list to the settings
    settings->setDownloadWebHostCachesSetting( listDialog->getData() );
}

void SettingsGui::on_incomingDirPushButton_clicked()
{
    SettingsHelper *settingsHelper = new SettingsHelper();
    QString pathName = "";
    // pathName is given by reference
    if (settingsHelper->sharePathDialog(this, pathName))
    {
        // set the String in the incomingDirLineEdit
        incomingDirLineEdit->setText(pathName);
    }
}

void SettingsGui::on_tempDirPushButton_clicked()
{
    SettingsHelper *settingsHelper = new SettingsHelper();
    QString pathName = "";
    // pathName is given by reference
    if (settingsHelper->sharePathDialog(this, pathName))
    {
        // set the String in the tempDirLineEdit
        tempDirLineEdit->setText(pathName);
    }
}

void SettingsGui::on_otherShareDirPushButton_clicked()
{
    SettingsHelper *settingsHelper = new SettingsHelper();
    QString pathName = "";
    // pathName is given by reference
    if (settingsHelper->sharePathDialog(this, pathName))
    {
        // set the String in the shareLineEdit
        otherShareModel.insertRow(0);
        QModelIndex modelIndex = otherShareModel.index( 0 );
        otherShareModel.setData( modelIndex,pathName);
        settings->setOtherSharedPathSetting( otherShareModel.stringList());
    }
}

void SettingsGui::on_suppressSharePushButton_clicked()
{
    QModelIndexList lst = otherShareDirListView->selectionModel()->selectedIndexes();
    QModelIndex index;
    foreach (index, lst)
    {
        otherShareModel.removeRow (index.row ());
    }
    settings->setOtherSharedPathSetting( otherShareModel.stringList());
}

void SettingsGui::on_styleCombo_activated(const QString &styleName)
{
    qApp->setStyle(styleName);

}

void SettingsGui::on_styleSheetCombo_activated(const QString &sheetName)
{
    loadStyleSheet(sheetName);
}


void SettingsGui::loadStyleSheet(const QString &sheetName)
{
    if(sheetName.toLower() == "custom")
    {
       QFile file( settings->getSettingsDir() + "custom.qss");
       file.open(QFile::ReadOnly);
       QString styleSheet = QLatin1String(file.readAll());
       qApp->setStyleSheet(styleSheet);
    }
    else
    {
       QFile file(":/qss/" + sheetName.toLower() + ".qss");
       file.open(QFile::ReadOnly);
       QString styleSheet = QLatin1String(file.readAll());
       qApp->setStyleSheet(styleSheet);
    }
}

void SettingsGui::closeEvent (QCloseEvent * event)
{
    hide();
    event->ignore();
}

void SettingsGui::changeEvent (QEvent* event)
{
    if (event->type () == QEvent::LanguageChange)
    {
        // apply the new language
        // all designed forms have retranslateUi() method
        retranslateUi (this);
        // retranslate other widgets which weren't added in the designer
        retranslate ();
    }

    QDialog::changeEvent (event);
}

void SettingsGui::retranslate ()
{
    // everything related to translation not included in ui.
    //for (int index = 0; index < languageComboBox->count (); ++index)
    //{
    //    languageComboBox->setItemText (index, tr (languageComboBox->itemText (index).toAscii ()));
    //}
}

void SettingsGui::setLanguage (int index)
{
    QString locale = languageComboBox->itemData (index).toString ();

    KommuteApplication::setLanguage (locale);
}

