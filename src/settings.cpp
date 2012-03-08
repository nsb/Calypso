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

#include "settings.h"
#include "setupwizard.h"
#include "keygenthread.h"

#include "defaultsettings.h"
#include "settingnames.h"
#include "applog.h"

#include "MUTE/layers/messageRouting/messageRouter.h"
//#include "MUTE/common/CryptoUtils.h"

#include <QDir>
#include <QProgressDialog>

#ifdef DEBUG
#include <QtDebug>
#endif

/**
@author Andy Gebauer - feivelda@googlemail.com
*/

/*!
* The constructor shall include all code needed at initialization of the class
* In our case this is only to load the settings
*/
extern char *settingsDir;

Settings::Settings(QObject *parent, bool applicationStart)
{
    appStart = applicationStart;
    // load settings when class is initialized
    loadSettings();
}

Settings::~Settings()
{
    delete settings;
}

/*!
* loadSettings loads the settings given in a configuration file and make them available for everyone.
* If one or more of the settings isn't available in the settings file, it loads the setupwizard to get
* all settings-> If only some settings are missing there seems to be an error in the settings file so
* it was a decision for safety to use the setupwizard again.
*/

int Settings::loadSettings()
{
    // we suggest all values are present
    bool foundValues = true;
    // QString for temporary values
    QString temp;

    // we open a qsettings object at the settings file
    settings = new QSettings( getSettingsDir() + DEFAULT_SETTINGS_SETTINGSFILE, QSettings::IniFormat);

    // test firewall settings
    settings->beginGroup(SETTING_NAMES_GROUPFIREWALLSETTINGS);
    foundValues &= settings->contains(SETTING_NAMES_HAS_FIREWALL);
    settings->endGroup();

    // test directory settings
    settings->beginGroup(SETTING_NAMES_GROUPSHARESETTINGS);
    foundValues &= settings->contains(SETTING_NAMES_OTHER_SHARE_PATH);
    settings->endGroup();

    // test key size settings
    settings->beginGroup(SETTING_NAMES_GROUPKEYSETTINGS);
    foundValues &= settings->contains(SETTING_NAMES_KEY_SIZE);
    // test seed settings ( is part of key settings )
    foundValues &= settings->contains(SETTING_NAMES_SEED);
    settings->endGroup();

    // test if first start
    setFirstStartSetting(settings->value(SETTING_NAMES_FIRST_START, DEFAULT_SETTINGS_STANDARDFIRSTSTART).toBool());
    // and if to download the WebHostCachesList everytime
    setReloadWebHostCachesSetting(settings->value(SETTING_NAMES_RELOAD_WEB_HOST_CACHES, DEFAULT_SETTINGS_STANDARDRELOADWEBHOSTCACHES).toBool());
    // get my IP
    setMyIP(settings->value(SETTING_NAMES_MYIP, DEFAULT_SETTINGS_STANDARDMYIP).toString());
    // get Dictionary
    temp = settings->value(SETTING_NAMES_DICTIONARY,DEFAULT_SETTINGS_STANDARDDICTIONARY).toString();
    setDictionary(temp.split(LIST_SEPARATOR));

    // load the download locations of WebHostCaches

    // load the old WebHostCachesSettings
    settings->beginGroup(SETTING_NAMES_GROUPWEBSETTINGS);
    temp = settings->value(SETTING_NAMES_WEB_HOST_CACHE, DEFAULT_SETTINGS_STANDARDWEBHOSTCACHE).toString();
    setWebHostCachesSetting(temp.split(LIST_SEPARATOR));
    temp = settings->value(SETTING_NAMES_DOWNLOAD_WEB_HOST_CACHE, WEBHOSTCACHESLISTDOWNLOAD).toString();
    setDownloadWebHostCachesSetting(temp.split(LIST_SEPARATOR));
    setMaxHostList(settings->value(SETTING_NAMES_MAXHOSTLIST, DEFAULT_SETTINGS_STANDARDMAXHOSTLIST).toInt());
    settings->endGroup();

    // if not all settings are available
    if ( appStart )
    {
        if ( !foundValues || getFirstStartSetting() )
        {
            // create setupwizard
            SetupWizard *setupWizard = new SetupWizard(0, Qt::Dialog);
            // reset the SetupWizard and give the pointer to our QSettings object
            setupWizard->reset(settings);
            // Show the dialog modal and test if pressed cancel or finish ( accepted )
            setFirstStartSetting(true);
            if ( QDialog::Accepted == setupWizard->exec() )
            {
                setFirstStartSetting(false);
            }
        }
    }

    /**
    @todo checking if loaded values are valid
    */

    /*
    * I hate to say this, but there is no chance to avoid the .to[Type]() because QSettings->value return a QVariant
    * Maybe someone knows a better way to implement this
    */
    // firewall.
    settings->beginGroup(SETTING_NAMES_GROUPFIREWALLSETTINGS);
    setFirewallSetting(settings->value(SETTING_NAMES_HAS_FIREWALL, DEFAULT_SETTINGS_STANDARDFIREWALL).toBool());
    setPortNumberSetting(settings->value(SETTING_NAMES_PORT, DEFAULT_SETTINGS_STANDARDPORT).toInt());
    settings->endGroup();

    // directories.
    settings->beginGroup(SETTING_NAMES_GROUPSHARESETTINGS);
    setIncomingDirectory(settings->value(SETTING_NAMES_INCOMING_PATH, QApplication::applicationDirPath().append("/").append("incoming")).toString());
    setTempDirectory(settings->value(SETTING_NAMES_TEMP_PATH, QApplication::applicationDirPath().append("/").append("temp")).toString());
    setSendQueueSizePerConnectionSetting(settings->value(SETTING_NAMES_SEND_QUEUE_SIZE_PER_CONNECTION, DEFAULT_SETTINGS_STANDARDSENDQUEUESIZEPERCONNECTION).toInt());
    setTargetConnectionCountSetting(settings->value(SETTING_NAMES_TARGET_CONNECTION_COUNT, DEFAULT_SETTINGS_STANDARDTARGETCONNECTIONCOUNT).toInt());
    temp = settings->value(SETTING_NAMES_OTHER_SHARE_PATH, DEFAULT_SETTINGS_STANDARDSHAREPATH).toString();
    if (temp != "")
    {
        setOtherSharedPathSetting(temp.split(LIST_SEPARATOR));
    }
    settings->endGroup();

    // key size, seed.
    settings->beginGroup(SETTING_NAMES_GROUPKEYSETTINGS);
    setKeySizeSetting(settings->value(SETTING_NAMES_KEY_SIZE, DEFAULT_SETTINGS_STANDARDKEYSIZE).toInt());
    setNodePublicKey(settings->value(SETTING_NAMES_NODE_PUBLIC_KEY, "").toString());
    setNodePrivateKey(settings->value(SETTING_NAMES_NODE_PRIVATE_KEY, "").toString());
    setSeedSetting(settings->value(SETTING_NAMES_SEED, DEFAULT_SETTINGS_STANDARDSEED).toString());
    setRandomSeed(settings->value(SETTING_NAMES_RANDOMSEED, "").toString());
    // generate randomSeed if needed
    if ( randomSeed.isEmpty() || getFirstStartSetting())
    {
        muteSeedRandomGenerator( (char*) getSeedSetting().toLatin1().constData() );
        randomSeed = QString::fromLatin1( muteGetRandomGeneratorState() );
    }
    settings->endGroup();

    settings->beginGroup(SETTING_NAMES_GROUPMIMETYPESSETTINGS);
    temp = settings->value(SETTING_NAMES_MIME_TYPES, DEFAULT_SETTINGS_STANDARDMIMETYPES).toString();
    setMimeTypesSetting(temp.split(LIST_SEPARATOR));
    settings->endGroup();

    settings->beginGroup(SETTING_NAMES_GROUPDOWNLOADSETTINGS);
    setDownloadChunkRetriesSetting(settings->value(SETTING_NAMES_DOWNLOAD_CHUNK_RETRIES, DEFAULT_SETTINGS_STANDARDDOWNLOADCHUNKRETRIES).toInt());
    setDownloadFileInfoRetriesSetting(settings->value(SETTING_NAMES_DOWNLOAD_FILE_INFO_RETRIES, DEFAULT_SETTINGS_STANDARDDOWNLOADFILEINFORETRIES).toInt());
    setDownloadFreshRouteProbabilitySetting(settings->value(SETTING_NAMES_DOWNLOAD_FRESH_ROUTE_PROBABILITY, DEFAULT_SETTINGS_STANDARDDOWNLOADFRESHROUTEPROBABILITY).toDouble());
    setPrintSearchSyncTraceSetting(settings->value(SETTING_NAMES_PRINT_SEARCH_SYNC_TRACE, DEFAULT_SETTINGS_STANDARDPRINTSEARCHSYNCTRACE).toInt());
    settings->beginGroup(SETTING_NAMES_GROUPDOWNLOAD_TIMEOUTSETTINGS);
    setDownloadTimeoutCurrentTimeoutWeightSetting(settings->value(SETTING_NAMES_DOWNLOAD_TIMEOUT_CURRENT_TIMEOUT_WEIGHT, DEFAULT_SETTINGS_STANDARDDOWNLOADTIMEOUTCURRENTTIMEOUTWEIGHT).toInt());
    setDownloadTimeoutMilliSecondsSetting(settings->value(SETTING_NAMES_DOWNLOAD_TIMEOUT_MILLI_SECONDS, DEFAULT_SETTINGS_STANDARDDOWNLOADTIMEOUTMILLISECONDS).toInt());
    setDownloadTimeoutRecentChunkWeightSetting(settings->value(SETTING_NAMES_DOWNLOAD_TIMEOUT_RECENT_CHUNK_WEIGHT, DEFAULT_SETTINGS_STANDARDDOWNLOADTIMEOUTRECENTCHUNKWEIGHT).toInt());
    settings->endGroup();

    settings->endGroup();

    settings->beginGroup(SETTING_NAMES_GROUPMESSAGELIMITSETTINGS);
    setInboundKBLimitSetting(settings->value(SETTING_NAMES_INBOUND_KB_LIMIT, DEFAULT_SETTINGS_STANDARDINBOUNDKBLIMIT).toInt());
    setOutboundKBLimitSetting(settings->value(SETTING_NAMES_OUTBOUND_KB_LIMIT, DEFAULT_SETTINGS_STANDARDOUTBOUNDKBLIMIT).toInt());
    settings->endGroup();

    settings->beginGroup(SETTING_NAMES_GROUPMAXSETTINGS);
    setMaxConnectionCountSetting(settings->value(SETTING_NAMES_MAX_CONNECTION_COUNT, DEFAULT_SETTINGS_STANDARDMAXCONNECTIONCOUNT).toInt());
    setMaxDroppedMessageFractionSetting(settings->value(SETTING_NAMES_MAX_DROPPED_MESSAGE_FRACTION, DEFAULT_SETTINGS_STANDARDMAXDROPPEDMESSAGEFRACTION).toDouble());
    setMaxMessageUtilitySetting(settings->value(SETTING_NAMES_MAX_MESSAGE_UTILITY, DEFAULT_SETTINGS_STANDARDMAXMESSAGEUTILITY).toInt());
    setMaxSimultaneousDownloadsSetting(settings->value(SETTING_NAMES_MAX_SIMULTANEOUS_DOWNLOADS, DEFAULT_SETTINGS_STANDARDMAXSIMULTANEOUSDOWNLOADS).toInt());
    settings->endGroup();

    settings->beginGroup(SETTING_NAMES_GROUPUTILITYSETTINGS);
    setUtilityAlphaSetting(settings->value(SETTING_NAMES_UTILITY_ALPHA, DEFAULT_SETTINGS_STANDARDUTILITYALPHA).toInt());
    setUtilityBetaSetting(settings->value(SETTING_NAMES_UTILITY_BETA, DEFAULT_SETTINGS_STANDARDUTILITYBETA).toInt());
    setUtilityGammaSetting(settings->value(SETTING_NAMES_UTILITY_GAMMA, DEFAULT_SETTINGS_STANDARDUTILITYGAMMA).toInt());
    settings->endGroup();

    settings->beginGroup(SETTING_NAMES_GROUPPROBABILITYSETTINGS);
    setBroadcastProbabilitySetting(settings->value(SETTING_NAMES_BROADCAST_PROBABILITY, DEFAULT_SETTINGS_STANDARDBROADCASTPROBABILITY).toDouble());
    setContinueForwardProbabilitySetting(settings->value(SETTING_NAMES_CONTINUE_FORWARD_PROBABILITY, DEFAULT_SETTINGS_STANDARDCONTINUEFORWARDPROBABILITY).toDouble());
    settings->endGroup();

    settings->beginGroup(SETTING_NAMES_GROUPMISCELLANEOUS);
    setShowNiceQuitSetting(settings->value(SETTING_NAMES_SHOW_NICE_QUIT, DEFAULT_SETTINGS_STANDARDSHOWNICEQUIT).toInt());
    setMuteLanguageSetting(settings->value(SETTING_NAMES_MUTE_LANGUAGE, DEFAULT_SETTINGS_STANDARDMUTELANGUAGE).toString());
    //setMuteVersionSetting(settings->value(SETTING_NAMES_MUTE_VERSION, DEFAULT_SETTINGS_STANDARDMUTEVERSION).toString());
    setChkAutorunWithWin ( settings->value(SETTING_NAMES_CHKAUTORUNWITHWIN, DEFAULT_SETTINGS_CHKAUTORUNWITHWIN).toBool());
    setMuteVersionSetting( DEFAULT_SETTINGS_STANDARDMUTEVERSION);
    settings->endGroup();

    settings->beginGroup(SETTING_NAMES_GROUPLOG);
    setLogConnectionContentsSetting(settings->value(SETTING_NAMES_LOG_CONNECTION_CONTENTS, DEFAULT_SETTINGS_STANDARDLOGCONNECTIONCONTENTS).toInt());
    setLogConnectionsSetting(settings->value(SETTING_NAMES_LOG_CONNECTIONS, DEFAULT_SETTINGS_STANDARDLOGCONNECTIONS).toInt());
    setLogDownloadTimeoutChangesSetting(settings->value(SETTING_NAMES_LOG_DOWNLOAD_TIMEOUT_CHANGES, DEFAULT_SETTINGS_STANDARDLOGDOWNLOADTIMEOUTCHANGES).toInt());
    setLogLevelSetting(settings->value(SETTING_NAMES_LOG_LEVEL, DEFAULT_SETTINGS_STANDARDLOGLEVEL).toInt());
    AppLog::setLoggingLevel(getLogLevelSetting());
    setLogMessageHistorySetting(settings->value(SETTING_NAMES_LOG_MESSAGE_HISTORY, DEFAULT_SETTINGS_STANDARDLOGMESSAGEHISTORY).toInt());
    setLogRollOverInSecondsSetting(settings->value(SETTING_NAMES_LOG_ROLL_OVER_IN_SECONDS, DEFAULT_SETTINGS_STANDARDLOGROLLOVERINSECONDS).toInt());
    setLogRoutingHistorySetting(settings->value(SETTING_NAMES_LOG_ROUTING_HISTORY, DEFAULT_SETTINGS_STANDARDLOGROUTINGHISTORY).toInt());
    settings->endGroup();

    settings->beginGroup(SETTING_NAMES_GROUPWEBSETTINGS);
    setWebHostCachePostIntervalSetting(settings->value(SETTING_NAMES_WEB_HOST_CACHE_POST_INTERVAL, DEFAULT_SETTINGS_STANDARDWEBHOSTCACHEPOSTINTERVAL).toInt());
    setUseMajorityRoutingSetting(settings->value(SETTING_NAMES_USE_MAJORITY_ROUTING, DEFAULT_SETTINGS_STANDARDUSEMAJORITYROUTING).toInt());
    setEnableWebHostCache(settings->value(SETTING_NAMES_ENABLE_WEB_HOST_CACHE, DEFAULT_SETTINGS_ENABLEWEBHOSTCACHE).toBool());
    setSendSeedNodes(settings->value(SETTING_NAMES_SEND_SEED_NODES, DEFAULT_SETTINGS_SENDSEEDNODES).toBool());
    setAcceptSeedNodes(settings->value(SETTING_NAMES_ACCEPT_SEED_NODES, DEFAULT_SETTINGS_ACCEPTSEEDNODES).toBool());
    settings->endGroup();

    settings->beginGroup(SETTING_NAMES_GROUPSTYLE);
    setStyleName(settings->value(SETTING_NAMES_STYLENAME, "Default").toString());
    qApp->setStyle(getStyleName());
    setSheetName(settings->value(SETTING_NAMES_SHEETNAME, "Default").toString());
    if(getSheetName().toLower() == "custom")
    {
       QFile file( getSettingsDir() + "custom.qss");
       file.open(QFile::ReadOnly);
       QString styleSheet = QLatin1String(file.readAll());
       qApp->setStyleSheet(styleSheet);
    }
    else
    {
       QFile file(":/qss/" + getSheetName().toLower() + ".qss");
       file.open(QFile::ReadOnly);
       QString styleSheet = QLatin1String(file.readAll());
       qApp->setStyleSheet(styleSheet);
    }
    setSearchWithDownloads(settings->value(SETTING_NAMES_DOWNLOADSWITHSEARCH, DEFAULT_SETTINGS_DOWNLOADSWITHSEARCH).toBool());
    settings->endGroup();

    settings->beginGroup(SETTING_NAMES_GROUPPROXY);
    setProxyHost(settings->value(SETTING_NAMES_PROXYHOST,DEFAULT_SETTINGS_STANDARDPROXYHOST).toString());
    setProxyPort(settings->value(SETTING_NAMES_PROXYPORT,DEFAULT_SETTINGS_STANDARDPROXYPORT).toInt());
    setProxyUser(settings->value(SETTING_NAMES_PROXYUSER,DEFAULT_SETTINGS_STANDARDPROXYUSER).toString());
    setProxyPassword(settings->value(SETTING_NAMES_PROXYPASSWORD,DEFAULT_SETTINGS_STANDARDPROXYPASSWORD).toString());
    settings->endGroup();

    if ( appStart )
    {
        #ifdef DEBUG
        qDebug() << "Firewall:"     << firewallSetting;
        qDebug() << "Port:"         << portSetting;
        qDebug() << "SharePathList:"    << otherSharedPathSetting;
        qDebug() << "Key Size:"     << keySizeSetting;
        qDebug() << "Seed:"         << seedSetting;
        qDebug() << "IncomingPath:" << incomingDirectory;
        qDebug() << "TempPath:"     << tempDirectory;
        #endif
        if(nodePublicKey.isEmpty() || nodePrivateKey.isEmpty())
        {
          // generate RSA Key
          #ifdef DEBUG
          qDebug() << "generate RSA Key";
          #endif
          KeyGenThread *thread = new KeyGenThread( getKeySizeSetting() );
          // show a progress dialog
          QProgressDialog progress(0, Qt::Popup);
          // set text in progress dialog
          progress.setLabelText(QProgressDialog::tr("Generating RSA key"));
          // set minimum and maximum range to 0. Through that we only have a moving indicator
          progress.setRange(0,0);
          // no cancel button
          progress.setCancelButton(0);
          progress.setAutoClose(false);
          progress.setAutoReset(false);
  
          // show the progress dialog
          progress.show();
          while ( thread->isRunning() )
          {
              // let the progress dialog perform the moving of the progress bar
              qApp->processEvents();
              /* Comment this because it stops all other apps
              // Test if progressdialog is visible
              if ( ! progress.isVisible() )
              {
                  // show progress dialog if it's not visible
                  progress.show();
              }*/
          }
          nodePublicKey = thread->getNodePublicKey();
          nodePrivateKey = thread->getNodePrivateKey();
          // delete KeyGenThread, it isn't needed anymore
          delete thread;
          #ifdef DEBUG
          qDebug() << "RSA Key done";
          #endif
        }

        // write the settings back in the settings object. This writes options there that aren't set by setupwizard
        // on the first startup, like port setting and similar
        saveSettings();

        // write the mute ini files
        writeMuteIniFiles( INIFILE_OVERWRITE );
    }

    // the return value should be a status value for this function
    /**
    @todo implement some real code to set the return value
    */
    return 0;
}

#include "minorGems/network/p2pParts/HostCatcher.h"
extern HostCatcher *muteHostCatcher;
void Settings::saveSettings()
{
    // QString for temporary values
    QString temp;
if(muteHostCatcher) muteHostCatcher->saveHosts();

    // kommute settings
    settings->setValue(SETTING_NAMES_FIRST_START, getFirstStartSetting());
    settings->setValue(SETTING_NAMES_RELOAD_WEB_HOST_CACHES, getReloadWebHostCachesSetting());
    settings->setValue(SETTING_NAMES_MYIP, getMyIP());
    settings->setValue(SETTING_NAMES_DICTIONARY, getDictionary().join(LIST_SEPARATOR));

    // mute core settings
    // firewall settings
    settings->beginGroup(SETTING_NAMES_GROUPFIREWALLSETTINGS);
    settings->setValue(SETTING_NAMES_HAS_FIREWALL, getFirewallSetting());
    settings->setValue(SETTING_NAMES_PORT, getPortNumberSetting());
    settings->endGroup();

    // share directory settings
    settings->beginGroup(SETTING_NAMES_GROUPSHARESETTINGS);
    settings->setValue(SETTING_NAMES_INCOMING_PATH, getIncomingDirectory());
    settings->setValue(SETTING_NAMES_TEMP_PATH, getTempDirectory());
    settings->setValue(SETTING_NAMES_SEND_QUEUE_SIZE_PER_CONNECTION, getSendQueueSizePerConnectionSetting());
    settings->setValue(SETTING_NAMES_TARGET_CONNECTION_COUNT, getTargetConnectionCountSetting());
    temp = getOtherSharedPathSetting().join(LIST_SEPARATOR);
    settings->setValue(SETTING_NAMES_OTHER_SHARE_PATH, temp);
    settings->endGroup();

    // key size settings
    settings->beginGroup(SETTING_NAMES_GROUPKEYSETTINGS);
    settings->setValue(SETTING_NAMES_KEY_SIZE, getKeySizeSetting());
    settings->setValue(SETTING_NAMES_NODE_PUBLIC_KEY, getNodePublicKey());
    settings->setValue(SETTING_NAMES_NODE_PRIVATE_KEY, getNodePrivateKey());
    settings->setValue(SETTING_NAMES_RANDOMSEED, getRandomSeed());
    settings->setValue(SETTING_NAMES_SEED, getSeedSetting());
    settings->endGroup();

    // mime type settings
    settings->beginGroup(SETTING_NAMES_GROUPMIMETYPESSETTINGS);
    temp = getMimeTypesSetting().join(LIST_SEPARATOR);
    settings->setValue(SETTING_NAMES_MIME_TYPES, temp);
    settings->endGroup();

    // webhost caches settings
    settings->beginGroup(SETTING_NAMES_GROUPWEBSETTINGS);
    temp = getWebHostCachesSetting().join(LIST_SEPARATOR);
    settings->setValue(SETTING_NAMES_WEB_HOST_CACHE, temp);
    temp = getDownloadWebHostCachesSetting().join(LIST_SEPARATOR);
    settings->setValue(SETTING_NAMES_DOWNLOAD_WEB_HOST_CACHE, temp);
    settings->setValue(SETTING_NAMES_WEB_HOST_CACHE_POST_INTERVAL, getWebHostCachePostIntervalSetting());
    settings->setValue(SETTING_NAMES_USE_MAJORITY_ROUTING, getUseMajorityRoutingSetting());
    settings->setValue(SETTING_NAMES_ENABLE_WEB_HOST_CACHE , getEnableWebHostCache());
    settings->setValue(SETTING_NAMES_SEND_SEED_NODES, getSendSeedNodes());
    settings->setValue(SETTING_NAMES_ACCEPT_SEED_NODES , getAcceptSeedNodes());
    settings->setValue(SETTING_NAMES_MAXHOSTLIST , getMaxHostList());
    settings->endGroup();

    // download settings
    settings->beginGroup(SETTING_NAMES_GROUPDOWNLOADSETTINGS);
    settings->setValue(SETTING_NAMES_DOWNLOAD_CHUNK_RETRIES, getDownloadChunkRetriesSetting());
    settings->setValue(SETTING_NAMES_DOWNLOAD_FILE_INFO_RETRIES, getDownloadFileInfoRetriesSetting());
    settings->setValue(SETTING_NAMES_DOWNLOAD_FRESH_ROUTE_PROBABILITY, getDownloadFreshRouteProbabilitySetting());
    settings->setValue(SETTING_NAMES_PRINT_SEARCH_SYNC_TRACE, getPrintSearchSyncTraceSetting());
    settings->beginGroup(SETTING_NAMES_GROUPDOWNLOAD_TIMEOUTSETTINGS);
    settings->setValue(SETTING_NAMES_DOWNLOAD_TIMEOUT_CURRENT_TIMEOUT_WEIGHT, getDownloadTimeoutCurrentTimeoutWeightSetting());
    settings->setValue(SETTING_NAMES_DOWNLOAD_TIMEOUT_MILLI_SECONDS, getDownloadTimeoutMilliSecondsSetting());
    settings->setValue(SETTING_NAMES_DOWNLOAD_TIMEOUT_RECENT_CHUNK_WEIGHT, getDownloadTimeoutRecentChunkWeightSetting());
    settings->endGroup();
    settings->endGroup();

    settings->beginGroup(SETTING_NAMES_GROUPMESSAGELIMITSETTINGS);
    settings->setValue(SETTING_NAMES_INBOUND_KB_LIMIT, getInboundKBLimitSetting());
    settings->setValue(SETTING_NAMES_OUTBOUND_KB_LIMIT, getOutboundKBLimitSetting());
    settings->endGroup();

    settings->beginGroup(SETTING_NAMES_GROUPMAXSETTINGS);
    settings->setValue(SETTING_NAMES_MAX_CONNECTION_COUNT, getMaxConnectionCountSetting());
    settings->setValue(SETTING_NAMES_MAX_DROPPED_MESSAGE_FRACTION, getMaxDroppedMessageFractionSetting());
    settings->setValue(SETTING_NAMES_MAX_MESSAGE_UTILITY, getMaxMessageUtilitySetting());
    settings->setValue(SETTING_NAMES_MAX_SIMULTANEOUS_DOWNLOADS, getMaxSimultaneousDownloadsSetting());
    settings->endGroup();

    settings->beginGroup(SETTING_NAMES_GROUPUTILITYSETTINGS);
    settings->setValue(SETTING_NAMES_UTILITY_ALPHA, getUtilityAlphaSetting());
    settings->setValue(SETTING_NAMES_UTILITY_BETA, getUtilityBetaSetting());
    settings->setValue(SETTING_NAMES_UTILITY_GAMMA, getUtilityGammaSetting());
    settings->endGroup();

    settings->beginGroup(SETTING_NAMES_GROUPPROBABILITYSETTINGS);
    settings->setValue(SETTING_NAMES_BROADCAST_PROBABILITY, getBroadcastProbabilitySetting());
    settings->setValue(SETTING_NAMES_CONTINUE_FORWARD_PROBABILITY, getContinueForwardProbabilitySetting());
    settings->endGroup();

    settings->beginGroup(SETTING_NAMES_GROUPMISCELLANEOUS);
    settings->setValue(SETTING_NAMES_SHOW_NICE_QUIT, getShowNiceQuitSetting());
    settings->setValue(SETTING_NAMES_MUTE_LANGUAGE, getMuteLanguageSetting());
    settings->setValue(SETTING_NAMES_MUTE_VERSION, getMuteVersionSetting());
    settings->setValue(SETTING_NAMES_CHKAUTORUNWITHWIN, getChkAutorunWithWin());
    settings->endGroup();

    settings->beginGroup(SETTING_NAMES_GROUPLOG);
    settings->setValue(SETTING_NAMES_LOG_CONNECTION_CONTENTS, getLogConnectionContentsSetting());
    settings->setValue(SETTING_NAMES_LOG_CONNECTIONS, getLogConnectionsSetting());
    settings->setValue(SETTING_NAMES_LOG_DOWNLOAD_TIMEOUT_CHANGES, getLogDownloadTimeoutChangesSetting());
    settings->setValue(SETTING_NAMES_LOG_LEVEL, getLogLevelSetting());
    settings->setValue(SETTING_NAMES_LOG_MESSAGE_HISTORY, getLogMessageHistorySetting());
    settings->setValue(SETTING_NAMES_LOG_ROLL_OVER_IN_SECONDS, getLogRollOverInSecondsSetting());
    settings->setValue(SETTING_NAMES_LOG_ROUTING_HISTORY, getLogRoutingHistorySetting());
    settings->endGroup();
    // kommute settings : style
    settings->beginGroup(SETTING_NAMES_GROUPSTYLE);
    settings->setValue(SETTING_NAMES_STYLENAME, getStyleName());
    settings->setValue(SETTING_NAMES_SHEETNAME, getSheetName());
    settings->setValue(SETTING_NAMES_DOWNLOADSWITHSEARCH, getSearchWithDownloads());
    settings->endGroup();
    settings->beginGroup(SETTING_NAMES_GROUPPROXY);
    settings->setValue(SETTING_NAMES_PROXYHOST, getProxyHost());
    settings->setValue(SETTING_NAMES_PROXYPORT, getProxyPort());
    settings->setValue(SETTING_NAMES_PROXYUSER, getProxyUser());
    settings->setValue(SETTING_NAMES_PROXYPASSWORD, getProxyPassword());
    settings->endGroup();
}

// get Settings functions start
bool Settings::getFirstStartSetting()                           { return firstStartSetting; }
bool Settings::getReloadWebHostCachesSetting()                  { return reloadWebHostCachesSetting; }
QString Settings::getMyIP()                  			{ return myIP; }
bool Settings::getFirewallSetting()                             { return firewallSetting; }
QString Settings::getIncomingDirectory()                        { return incomingDirectory;}
QString Settings::getTempDirectory()                            { return tempDirectory;}
int Settings::getSendQueueSizePerConnectionSetting()            { return sendQueueSizePerConnectionSetting; }
int Settings::getTargetConnectionCountSetting()                 { return targetConnectionCountSetting; }
int Settings::getKeySizeSetting()                               { return keySizeSetting; }
QString Settings::getSeedSetting()                              { return seedSetting; }
QString Settings::getSettingsDir()                              
{
    if(settingsDir[0]=='/' || (settingsDir[0]=='.'&&settingsDir[1]=='/'))
      return QString( QString(settingsDir) + QDir::separator());
    else
      return QString(QDir::homePath() + QDir::separator() + settingsDir + QDir::separator());
}
int Settings::getPortNumberSetting()                            { return portSetting; }
QStringList Settings::getMimeTypesSetting()                     { return mimeTypesSetting; }
QStringList Settings::getWebHostCachesSetting()                 { return webHostCachesSetting; }
QStringList Settings::getOtherSharedPathSetting()               { return otherSharedPathSetting; }
QStringList Settings::getDownloadWebHostCachesSetting()         { return downloadWebHostCachesSetting; }
int Settings::getWebHostCachePostIntervalSetting()              { return webHostCachePostIntervalSetting; }
int Settings::getUseMajorityRoutingSetting()                    { return useMajorityRoutingSetting; }
int Settings::getDownloadChunkRetriesSetting()                  { return downloadChunkRetriesSetting; }
int Settings::getDownloadFileInfoRetriesSetting()               { return downloadFileInfoRetriesSetting; }
float Settings::getDownloadFreshRouteProbabilitySetting()       { return downloadFreshRouteProbabilitySetting; }
int Settings::getPrintSearchSyncTraceSetting()                  { return printSearchSyncTraceSetting; }
int Settings::getDownloadTimeoutCurrentTimeoutWeightSetting()   { return downloadTimeoutCurrentTimeoutWeightSetting; }
int Settings::getDownloadTimeoutMilliSecondsSetting()           { return downloadTimeoutMilliSecondsSetting; }
int Settings::getDownloadTimeoutRecentChunkWeightSetting()      { return downloadTimeoutRecentChunkWeightSetting; }
int Settings::getInboundKBLimitSetting()                        { return inboundKBLimitSetting; }
int Settings::getOutboundKBLimitSetting()                       { return outboundKBLimitSetting; }
int Settings::getMaxConnectionCountSetting()                    { return maxConnectionCountSetting; }
float Settings::getMaxDroppedMessageFractionSetting()           { return maxDroppedMessageFractionSetting; }
int Settings::getMaxMessageUtilitySetting()                     { return maxMessageUtilitySetting; }
int Settings::getMaxSimultaneousDownloadsSetting()              { return maxSimultaneousDownloadsSetting; }
int Settings::getUtilityAlphaSetting()                          { return utilityAlphaSetting; }
int Settings::getUtilityBetaSetting()                           { return utilityBetaSetting; }
int Settings::getUtilityGammaSetting()                          { return utilityGammaSetting; }
float Settings::getBroadcastProbabilitySetting()                { return broadcastProbabilitySetting; }
float Settings::getContinueForwardProbabilitySetting()          { return continueForwardProbabilitySetting; }
int Settings::getShowNiceQuitSetting()                          { return showNiceQuitSetting; }
QString Settings::getMuteLanguageSetting()                      { return muteLanguageSetting; }
QString Settings::getMuteVersionSetting()                       { return muteVersionSetting; }
int Settings::getLogConnectionContentsSetting()                 { return logConnectionContentsSetting; }
int Settings::getLogConnectionsSetting()                        { return logConnectionsSetting; }
int Settings::getLogDownloadTimeoutChangesSetting()             { return logDownloadTimeoutChangesSetting; }
int Settings::getLogLevelSetting()                          	{ return logLevelSetting; }
int Settings::getLogMessageHistorySetting()                     { return logMessageHistorySetting; }
int Settings::getLogRollOverInSecondsSetting()                  { return logRollOverInSecondsSetting; }
int Settings::getLogRoutingHistorySetting()                     { return logRoutingHistorySetting; }
QString Settings::getStyleName()                                { return styleName; }
QString Settings::getSheetName()                                { return sheetName; }
bool Settings::getSearchWithDownloads()                         { return searchWithDownloads; }
bool Settings::getSendSeedNodes()                               { return sendSeedNodes; }
bool Settings::getAcceptSeedNodes()                             { return acceptSeedNodes; }
bool Settings::getEnableWebHostCache()                          { return enableWebHostCache; }
bool Settings::getChkAutorunWithWin()				{ return chkAutorunWithWin; }
QString Settings::getNodePublicKey()                  		{ return nodePublicKey; }
QString Settings::getNodePrivateKey()                  		{ return nodePrivateKey; }
QString Settings::getProxyHost()                  		{ return proxyHost; }
int Settings::getProxyPort()                  			{ return proxyPort; }
QString Settings::getProxyUser()                  		{ return proxyUser; }
QString Settings::getProxyPassword()                  		{ return proxyPassword; }
QStringList Settings::getDictionary()                  		{ return dictionary; }
QString Settings::getRandomSeed()                  		{ return randomSeed; }
int Settings::getMaxHostList()                  		{ return maxHostList; }
// get Settings functions end

// set Settings functions start
void Settings::setFirstStartSetting(bool setting)                           { firstStartSetting = setting; }
void Settings::setReloadWebHostCachesSetting(bool setting)                  { reloadWebHostCachesSetting = setting; }
void Settings::setMyIP(QString setting)                  		    { myIP = setting; }
void Settings::setFirewallSetting(bool setting)                             { firewallSetting = setting; }
void Settings::setIncomingDirectory(QString setting)                        { incomingDirectory = setting;}
void Settings::setTempDirectory(QString setting)                            { tempDirectory = setting;}
void Settings::setSendQueueSizePerConnectionSetting(int setting)            { sendQueueSizePerConnectionSetting = setting; }
void Settings::setTargetConnectionCountSetting(int setting)                 { targetConnectionCountSetting = setting; }
void Settings::setKeySizeSetting(int setting)                               { keySizeSetting = setting; }
void Settings::setSeedSetting(QString setting)                              { seedSetting = setting; }
void Settings::setPortNumberSetting(int setting)                            { portSetting = setting; }
void Settings::setMimeTypesSetting(QStringList setting)                     { mimeTypesSetting = setting; }
void Settings::setWebHostCachesSetting(QStringList setting)                 { webHostCachesSetting = setting; }
void Settings::setOtherSharedPathSetting(QStringList setting)               { otherSharedPathSetting = setting; }
void Settings::setDownloadWebHostCachesSetting(QStringList setting)         { downloadWebHostCachesSetting = setting; }
void Settings::setWebHostCachePostIntervalSetting(int setting)              { webHostCachePostIntervalSetting = setting; }
void Settings::setUseMajorityRoutingSetting(int setting)                    { useMajorityRoutingSetting = setting; }
void Settings::setDownloadChunkRetriesSetting(int setting)                  { downloadChunkRetriesSetting = setting; }
void Settings::setDownloadFileInfoRetriesSetting(int setting)               { downloadFileInfoRetriesSetting = setting; }
void Settings::setDownloadFreshRouteProbabilitySetting(float setting)       { downloadFreshRouteProbabilitySetting = setting; }
void Settings::setPrintSearchSyncTraceSetting(int setting)                  { printSearchSyncTraceSetting = setting; }
void Settings::setDownloadTimeoutCurrentTimeoutWeightSetting(int setting)   { downloadTimeoutCurrentTimeoutWeightSetting = setting; }
void Settings::setDownloadTimeoutMilliSecondsSetting(int setting)           { downloadTimeoutMilliSecondsSetting = setting; }
void Settings::setDownloadTimeoutRecentChunkWeightSetting(int setting)      { downloadTimeoutRecentChunkWeightSetting = setting; }
void Settings::setInboundKBLimitSetting(int setting)                        { inboundKBLimitSetting = setting; }
void Settings::setOutboundKBLimitSetting(int setting)                       { outboundKBLimitSetting = setting; }
void Settings::setMaxConnectionCountSetting(int setting)                    { maxConnectionCountSetting = setting; }
void Settings::setMaxDroppedMessageFractionSetting(float setting)           { maxDroppedMessageFractionSetting = setting; }
void Settings::setMaxMessageUtilitySetting(int setting)                     { maxMessageUtilitySetting = setting; }
void Settings::setMaxSimultaneousDownloadsSetting(int setting)              { maxSimultaneousDownloadsSetting = setting; }
void Settings::setUtilityAlphaSetting(int setting)                          { utilityAlphaSetting = setting; }
void Settings::setUtilityBetaSetting(int setting)                           { utilityBetaSetting = setting; }
void Settings::setUtilityGammaSetting(int setting)                          { utilityGammaSetting = setting; }
void Settings::setBroadcastProbabilitySetting(float setting)                { broadcastProbabilitySetting = setting; }
void Settings::setContinueForwardProbabilitySetting(float setting)          { continueForwardProbabilitySetting = setting; }
void Settings::setShowNiceQuitSetting(int setting)                          { showNiceQuitSetting = setting; }
void Settings::setMuteLanguageSetting(QString setting)                      { muteLanguageSetting = setting; }
void Settings::setMuteVersionSetting(QString setting)                       { muteVersionSetting = setting; }
void Settings::setLogConnectionContentsSetting(int setting)                 { logConnectionContentsSetting = setting; }
void Settings::setLogConnectionsSetting(int setting)                        { logConnectionsSetting = setting; }
void Settings::setLogDownloadTimeoutChangesSetting(int setting)             { logDownloadTimeoutChangesSetting = setting; }
void Settings::setLogLevelSetting(int setting)                              { logLevelSetting = setting; }
void Settings::setLogMessageHistorySetting(int setting)                     { logMessageHistorySetting = setting; }
void Settings::setLogRollOverInSecondsSetting(int setting)                  { logRollOverInSecondsSetting = setting; }
void Settings::setLogRoutingHistorySetting(int setting)                     { logRoutingHistorySetting = setting; }
void Settings::setStyleName(QString style)                                  { styleName = style; }
void Settings::setSheetName(QString sheet)                                  { sheetName = sheet; }
void Settings::setSearchWithDownloads(bool x)                               { searchWithDownloads = x; }
void Settings::setAcceptSeedNodes(bool setting)                             { acceptSeedNodes = setting; }
void Settings::setSendSeedNodes(bool setting)                               { sendSeedNodes = setting; }
void Settings::setEnableWebHostCache(bool setting)                          { enableWebHostCache = setting; }
void Settings::setChkAutorunWithWin(bool setting)                           { chkAutorunWithWin = setting; }
void Settings::setNodePublicKey(QString setting)                  	    { nodePublicKey = setting; }
void Settings::setNodePrivateKey(QString setting)                  	    { nodePrivateKey = setting; }
void Settings::setProxyHost(QString setting)                  		    { proxyHost = setting; }
void Settings::setProxyPort(int setting)                  		    { proxyPort = setting; }
void Settings::setProxyUser(QString setting)                  		    { proxyUser = setting; }
void Settings::setProxyPassword(QString setting)			    { proxyPassword = setting; }
void Settings::setDictionary(QStringList setting)			    { dictionary = setting; }
void Settings::setRandomSeed(QString setting)                  		    { randomSeed = setting; }
void Settings::setMaxHostList(int setting)                  		    { maxHostList = setting; }
// set Settings functions end

/*!
* MuteIniFile is a function to handle the settings for the mute core.
* The settings for the mute core are located in ini file in the ~/.mute dir
* in this function we write the values to that files or read them from there
* this is controlled through the read parameter
* value is given by reference. This gives us the chance to call the function
* with normal vars, but we can get them back, if we are in read mode.
* So the function can be nearly equal for read and write
*/

bool Settings::muteIniFile( QString fileName, QString &value, bool read )
{
    // confDirPath shows the path to the mute settings dir
    QString confDirPath = getSettingsDir();
    // file is the settings file inside confDirPath
    QFile file( confDirPath + fileName );
    // we need this cascaded, else the file.open is executed with the AND operator and we try to open the same file twice
    if ( read )
    {
        // if we're in read  mode and we can't open the file in read  mode or it's no text file, then exit without success
        if ( ! file.open(QIODevice::ReadOnly  | QIODevice::Text)) return false;
    }
    else
    {
        // if we're in write mode and we can't open the file in write mode or it's no text file, then exit without success
        if ( ! file.open(QIODevice::WriteOnly | QIODevice::Text)) return false;
    }
    // create a textstream at the file
    QTextStream stream(&file);
    // if we are in read mode, we put the stream from the file into value
    if ( read )
    {
        stream >> value;
    }
    // if we are in write mode, we put value into the textstream
    else
    {
        stream << value;
    }
    // close the file
    file.close();
    // exit and show success
    return true;
}

bool Settings::muteIniFileExists( QString fileName )
{
    // confDirPath shows the path to the mute settings dir
    QString confDirPath = getSettingsDir();
    // file is the settings file inside confDirPath
    QFile file( confDirPath + fileName );
    // return if the file exists
    return file.exists();
}

void Settings::writeMuteIniFiles(bool overwrite)
{
    // the temp is needed because we can't give a reference to muteIniFile
    QString temp;
    // the fileName is used because of easier fileName editing
    QString fileName, fileNameExtra;

/*
    // generate randomSeed
    fileName = "randomSeed.ini";
    if ( !muteIniFileExists( fileName ) || getFirstStartSetting())
    {
        debugMissingIniFile( fileName );
        muteSeedRandomGenerator( (char*) getSeedSetting().toLatin1().constData() );
        temp = QString::fromLatin1( muteGetRandomGeneratorState() );
        muteIniFile( fileName, temp, INIFILE_WRITE);
    }
*/
}

int Settings::testAndWriteIniFile( QString fileName, QString setting, bool overwrite )
{
    if ( fileName.isEmpty() ) // also returns true if fileName is NULL
        return -1; // error: fileName empty or null
    if (( !muteIniFileExists( fileName ) ) || ( overwrite == INIFILE_OVERWRITE ))
    {
        debugMissingIniFile( fileName );
        muteIniFile( fileName, setting, INIFILE_WRITE );
        return 1; // new file written
    }
    return 0; // nothing done
}

void Settings::debugMissingIniFile( QString fileName )
{
    #ifdef DEBUG
    qDebug() << "Kommute creates" << fileName;
    #endif
}

