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

#include <QDir>
#include <QtGui>

#include <stdio.h>
#include <stdlib.h>
// as seed for the random number generator
#include <time.h>

#include "setupwizard.h"
#include "settingshelper.h"

#include "defaultsettings.h"
#include "settingnames.h"

#ifdef DEBUG
#include <QtDebug>
#endif

/**
@author Andy Gebauer - feivelda@googlemail.com
*/

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

/* Images for context menu icons */
#define IMAGE_CANCEL        ":/delete.png"

/*!
* The constructor for SetupWizard calls the reset function to set all values to the default ones at start
*/
SetupWizard::SetupWizard(QWidget *parent, Qt::WFlags flags)
: QDialog(parent, flags)
{
    setupUi(this);

    otherShareDirListView_3->setModel(&otherShareModel);
    connect( otherShareDirListView_3, SIGNAL( customContextMenuRequested( QPoint ) ), this, SLOT( otherShareDirListViewCostumPopupMenu( QPoint ) ) );
}

/*!
* The destructor for SetupWizard
*/
SetupWizard::~SetupWizard()
{
}

/*!
* The reset function sets all values to the defaults defined in the defaultSettings. For this it
* initializes the defaultSettings class, which holds the default values, and the settingsNames class,
* which holds the names for the values in the configuration files.
* It also sets the labelstackedWidget ( the small text in the upper left ) and the textstackedWidget
* ( the large Text and input boxes in the middle ) both to the first ones. Through we are at the
* first position for both widgets, we can disable the backButton. It isn't needed at the first widget
* because there's no widget -1.
* Laststep is set the false here because we aren't at the last widget, we the next button becomes the
* finish button.
*/
void SetupWizard::reset(QSettings *settingsPointer)
{
    // set the labelstackedWidget and textstackedWidget to the first position
    labelstackedWidget->setCurrentIndex(0);
    textstackedWidget->setCurrentIndex(0);
    // disable the backButton: We don't need it if we are on the first position
    backButton->setEnabled(false);
    // and we aren't at the last step, were the next button becomes the finish button
    lastStep = false;

    settings = settingsPointer;

    // initialize the variables with the default values
    firewall_true_false     = DEFAULT_SETTINGS_STANDARDFIREWALL;
    keySize                 = DEFAULT_SETTINGS_STANDARDKEYSIZE;
    seed_String             = DEFAULT_SETTINGS_STANDARDSEED;
    otherSharedPathSetting.clear ();
}

void SetupWizard::writeSettings()
{
    // first the firewall settings
    settings->beginGroup(SETTING_NAMES_GROUPFIREWALLSETTINGS);
    settings->setValue(SETTING_NAMES_HAS_FIREWALL,firewall_true_false);
    settings->setValue(SETTING_NAMES_PORT, kommutePortSpinBox->value());
    settings->endGroup();

    // share directory settings
    settings->beginGroup(SETTING_NAMES_GROUPSHARESETTINGS);
    QString temp = otherSharedPathSetting.join(LIST_SEPARATOR);
    settings->setValue(SETTING_NAMES_OTHER_SHARE_PATH, temp);
    settings->endGroup();

    // key size settings
    settings->beginGroup(SETTING_NAMES_GROUPKEYSETTINGS);
    settings->setValue(SETTING_NAMES_KEY_SIZE,keySize);
    settings->endGroup();

    // proxy setting
    settings->beginGroup(SETTING_NAMES_GROUPPROXY);
    settings->setValue(SETTING_NAMES_PROXYHOST, proxyHostLineEdit->text());
    settings->setValue(SETTING_NAMES_PROXYPORT, proxyPortSpinBox->value());
    settings->setValue(SETTING_NAMES_PROXYUSER, proxyUserLineEdit->text());
    settings->setValue(SETTING_NAMES_PROXYPASSWORD, proxyPasswordLineEdit->text());
    settings->endGroup();
    // seed settings ( is part of key settings )
    // generate a seed if there's none, because we need one under all circumstances
    /*!
    * String generation routine is part of kapplication source
    * so the copyright is at kde.org
    * Thank guys there
    */
    if ( seed_String.isEmpty() )
    {
        int length = DEFAULT_SETTINGS_STANDARDSEEDLENGTH;
        int i = 0;
        seed_String.resize( length );
        srand( time( NULL ));
        while (length--)
        {
            //int r=34;
            int r = rand() % 62;
            r += 48;
            if (r>57) r += 7;
            if (r>90) r += 6;
            seed_String[i++] = char(r);
        }
        // string generation end
    }

    settings->setValue(SETTING_NAMES_SEED,seed_String);
    settings->endGroup();
}

/*!
* The on_nextButton_clicked function holds most of the event code and sets most
* of the variables. It's implemented that way, because an implementation with
* signals/slots for each button would be much more complecated. This is caused
* by the standard values so we always have a save value.
* The implementation of one if after another might look strange or even
* unpractically, but it has a necessary function. Let us took a short look at
* the firewall radio buttons: At start, no one of them is checked, so what if the
* user presses next? The first check fails, the second check fails, but the
* default value at the end of the case catchs this. If the firewall was set to
* true, then the switch is exited after the firwall_true_false is set.
*/
void SetupWizard::on_nextButton_clicked()
{
    // The current index position
    int index = textstackedWidget->currentIndex();
    // Test the different widgets
    switch ( textstackedWidget->currentIndex()) {
        // Firewall buttons
        case 1 :
        {
            if ( fw_no_radioButton->isChecked() )
            {
                // we use no firewall
                firewall_true_false = false;
                break;
            }
            if ( fw_yes_radioButton->isChecked() )
            {
                // we use a firewall
                firewall_true_false = true;
                break;
            }
            firewall_true_false = DEFAULT_SETTINGS_STANDARDFIREWALL;
            break;
        }
        // Share Directory choosing
        case 2 :
        {
            break;
        }
        // Key size buttons
        case 3 :
        {
            if ( key512_radioButton->isChecked() )
            {
                // use 512 bit key
                keySize = 512;
                break;
            }
            if ( key1024_radioButton->isChecked() )
            {
                // use 1024 bit key
                keySize = 1024;
                break;
            }
            if ( key2048_radioButton->isChecked() )
            {
                // use 2048 bit key
                keySize = 2048;
                break;
            }
            if ( key4096_radioButton->isChecked() )
            {
                // use 4096 bit key
                keySize = 4096;
                break;
            }
            keySize = DEFAULT_SETTINGS_STANDARDKEYSIZE;
            break;
        }
        // Seed string
        case 4 :
        {
            if ( !seed_lineEdit->text().isEmpty() )
            {
                seed_String = seed_lineEdit->text();
            }
            break;
        }
        default :
            break;
    }

    /*
    * move to the next widget, with the signal/slot also labelstackedWidget,
    * it's also moved
    */
    index++;
    // activate the backButton, because we are at the second widget or higher
    backButton->setEnabled(true);

    // if lastStep is true, then we clicked the finish button
    if ( lastStep )
    {
        // set Settings in the object
        writeSettings();
        // leave the dialog
        accept();
    }

    /*
    * if we are at the last widget or higher
    * the >= is here because the button "Next" becomes the button "Finish" there
    */
    if ( index >= textstackedWidget->count() - 1 )
    {
        // the widget position is set to the last widget
        index = textstackedWidget->count() - 1;
        // the button "Next" gets a new text: "Finish"
        nextButton->setText(tr("Finish"));
        // we are at the last step, so we can finish here
        lastStep = true;
    }
    else
    {
        lastStep = false;
    }

    textstackedWidget->setCurrentIndex(index);
}

/*!
* The on_backButton_clicked function is called when the back button is clicked.
* First it set the variable index to the currentIndex of textstackedWidget minus 1.
* So we have the previous position. Then it tests if the index goes lower then 0.
* Because of there is no value prior 0, it sets the index back to 0.
* If we was at the last widget, the button was named "Finish". If we go back, it's
* named "Next" again and the lastStep is set to false.
*/
void SetupWizard::on_backButton_clicked()
{
    int index = textstackedWidget->currentIndex() - 1;
    if ( index <= 0 )
    {
        index = 0;
        backButton->setEnabled(false);
    }
    nextButton->setText(tr("Next"));
    lastStep = false;
    textstackedWidget->setCurrentIndex(index);
}

/*!
* This function closes the dialog without saving the values
*/
void SetupWizard::on_cancelButton_clicked()
{
    // if cancel is pressed, use the standard settings
    writeSettings();
    // leave but show that cancel was pressed
    reject();
}

/*!
* This function shows the dialog help
*/
void SetupWizard::on_helpButton_clicked()
{
}

/*!
* This functions helps us to select a directory for sharing
*/
void SetupWizard::on_otherShareDirPushButton_3_clicked()
{
    SettingsHelper *settingsHelper = new SettingsHelper();
    QString pathName = "";
    // pathName is given by reference
    settingsHelper->sharePathDialog(this, pathName);

    // set for gui change.
    otherShareModel.insertRow(0);
    QModelIndex modelIndex = otherShareModel.index(0);
    otherShareModel.setData(modelIndex, pathName);

    // set to variable for settings.
    otherSharedPathSetting = otherShareModel.stringList ();
}

/*!
* This functions helps us to remove a directory for sharing
*/
void SetupWizard::on_suppressSharePushButton_3_clicked()
{
    QModelIndexList lst = otherShareDirListView_3->selectionModel()->selectedIndexes();
    QModelIndex index;

    // set for gui change.
    foreach (index, lst)
    {
        otherShareModel.removeRow (index.row ());
    }

    // set to variable for settings.
    otherSharedPathSetting = otherShareModel.stringList ();
}

void SetupWizard::otherShareDirListViewCostumPopupMenu( QPoint point )
{
      QMenu contextMnu( this );
      QMouseEvent *mevent = new QMouseEvent( QEvent::MouseButtonPress, point, Qt::RightButton, Qt::RightButton, Qt::NoModifier );

      QAction* removeAct = new QAction(QIcon(IMAGE_CANCEL), tr( "Remove" ), this );
      connect( removeAct , SIGNAL( triggered() ), this, SLOT( on_suppressSharePushButton_3_clicked() ) );

      contextMnu.clear();
      contextMnu.addAction( removeAct );
      contextMnu.exec( mevent->globalPos() );
}

void SetupWizard::changeEvent (QEvent* event)
{
    if (event->type () == QEvent::LanguageChange)
    {
        // apply the new language
        // all designed forms have retranslateUi() method
        retranslateUi (this);
    }

    QDialog::changeEvent (event);
}

