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
#ifndef SETTINGS_H
#define SETTINGS_H
#define DEBUG

#include <QStringList>
#include <QSettings>

/**
@author Andy Gebauer - feivelda@googlemail.com
*/

class Settings
{

public:
    Settings(QObject *parent = 0, bool applicationStart = true);
    ~Settings();
    int loadSettings();
    void saveSettings();

    // get settings functions
    bool getFirstStartSetting();
    bool getReloadWebHostCachesSetting();
    QString getSeedSetting();
    QString getIncomingDirectory();
    QString getTempDirectory();
    int getSendQueueSizePerConnectionSetting();
    int getTargetConnectionCountSetting();
    int getKeySizeSetting();
    bool getFirewallSetting();
    QString getSettingsDir();
    int getPortNumberSetting();
    QStringList getMimeTypesSetting();
    QStringList getWebHostCachesSetting();
    QStringList getDownloadWebHostCachesSetting();
    QStringList getOtherSharedPathSetting();
    int getWebHostCachePostIntervalSetting();
    int getUseMajorityRoutingSetting();
    int getDownloadChunkRetriesSetting();
    int getDownloadFileInfoRetriesSetting();
    float getDownloadFreshRouteProbabilitySetting();
    int getPrintSearchSyncTraceSetting();
    int getDownloadTimeoutCurrentTimeoutWeightSetting();
    int getDownloadTimeoutMilliSecondsSetting();
    int getDownloadTimeoutRecentChunkWeightSetting();
    int getInboundKBLimitSetting();
    int getOutboundKBLimitSetting();
    int getMaxConnectionCountSetting();
    float getMaxDroppedMessageFractionSetting();
    int getMaxMessageUtilitySetting();
    int getMaxSimultaneousDownloadsSetting();
    int getUtilityAlphaSetting();
    int getUtilityBetaSetting();
    int getUtilityGammaSetting();
    float getBroadcastProbabilitySetting();
    float getContinueForwardProbabilitySetting();
    int getShowNiceQuitSetting();
    QString getMuteLanguageSetting();
    QString getMuteVersionSetting();
    int getLogConnectionContentsSetting();
    int getLogConnectionsSetting();
    int getLogDownloadTimeoutChangesSetting();
    int getLogLevelSetting();
    int getLogMessageHistorySetting();
    int getLogRollOverInSecondsSetting();
    int getLogRoutingHistorySetting();
    QString getStyleName();
    QString getSheetName();
    bool getSearchWithDownloads();
    bool getEnableWebHostCache();
    bool getSendSeedNodes();
    bool getAcceptSeedNodes();
    bool getChkAutorunWithWin();
    QString getMyIP();
    QString getNodePublicKey();
    QString getNodePrivateKey();
    QString getProxyHost();
    int getProxyPort();
    QString getProxyUser();
    QString getProxyPassword();
    QStringList getDictionary();
    QString getRandomSeed();
    int getMaxHostList();

    // set settings functions
    void setFirstStartSetting(bool setting);
    void setReloadWebHostCachesSetting(bool setting);
    void setFirewallSetting(bool setting);
    void setIncomingDirectory(QString setting);
    void setTempDirectory(QString setting);
    void setSendQueueSizePerConnectionSetting(int setting);
    void setTargetConnectionCountSetting(int setting);
    void setKeySizeSetting(int setting);
    void setSeedSetting(QString setting);
    void setPortNumberSetting(int setting);
    void setMimeTypesSetting(QStringList setting);
    void setWebHostCachesSetting(QStringList setting);
    void setOtherSharedPathSetting(QStringList setting);
    void setDownloadWebHostCachesSetting(QStringList setting);
    void setWebHostCachePostIntervalSetting(int setting);
    void setUseMajorityRoutingSetting(int setting);
    void setDownloadChunkRetriesSetting(int setting);
    void setDownloadFileInfoRetriesSetting(int setting);
    void setDownloadFreshRouteProbabilitySetting(float setting);
    void setPrintSearchSyncTraceSetting(int setting);
    void setDownloadTimeoutCurrentTimeoutWeightSetting(int setting);
    void setDownloadTimeoutMilliSecondsSetting(int setting);
    void setDownloadTimeoutRecentChunkWeightSetting(int setting);
    void setInboundKBLimitSetting(int setting);
    void setOutboundKBLimitSetting(int setting);
    void setMaxConnectionCountSetting(int setting);
    void setMaxDroppedMessageFractionSetting(float setting);
    void setMaxMessageUtilitySetting(int setting);
    void setMaxSimultaneousDownloadsSetting(int setting);
    void setUtilityAlphaSetting(int setting);
    void setUtilityBetaSetting(int setting);
    void setUtilityGammaSetting(int setting);
    void setBroadcastProbabilitySetting(float setting);
    void setContinueForwardProbabilitySetting(float setting);
    void setShowNiceQuitSetting(int setting);
    void setMuteLanguageSetting(QString setting);
    void setMuteVersionSetting(QString setting);
    void setLogConnectionContentsSetting(int setting);
    void setLogConnectionsSetting(int setting);
    void setLogDownloadTimeoutChangesSetting(int setting);
    void setLogLevelSetting(int setting);
    void setLogMessageHistorySetting(int setting);
    void setLogRollOverInSecondsSetting(int setting);
    void setLogRoutingHistorySetting(int setting);
    void setStyleName(QString style);
    void setSheetName(QString sheet);
    void setSearchWithDownloads(bool dws);
    void setEnableWebHostCache(bool setting);
    void setSendSeedNodes(bool setting);
    void setAcceptSeedNodes(bool setting);
    void setChkAutorunWithWin(bool setting);
    void setMyIP(QString setting);
    void setNodePublicKey(QString setting);
    void setNodePrivateKey(QString setting);
    void setProxyHost(QString setting);
    void setProxyPort(int setting);
    void setProxyUser(QString setting);
    void setProxyPassword(QString setting);
    void setDictionary(QStringList setting);
    void setRandomSeed(QString setting);
    void setMaxHostList(int setting);

private:
    QSettings *settings;

    bool appStart;

    bool firstStartSetting;
    QString myIP; // internet address of this client
    bool reloadWebHostCachesSetting;
    bool firewallSetting;
    QString seedSetting;
    int keySizeSetting;
    QString incomingDirectory;
    QString tempDirectory;
    int sendQueueSizePerConnectionSetting;
    int targetConnectionCountSetting;
    int portSetting;
    QStringList mimeTypesSetting;
    QStringList webHostCachesSetting;
    QStringList downloadWebHostCachesSetting;
    int webHostCachePostIntervalSetting;
    int useMajorityRoutingSetting;
    int downloadChunkRetriesSetting;
    int downloadFileInfoRetriesSetting;
    float downloadFreshRouteProbabilitySetting;
    int printSearchSyncTraceSetting;
    int downloadTimeoutCurrentTimeoutWeightSetting;
    int downloadTimeoutMilliSecondsSetting;
    int downloadTimeoutRecentChunkWeightSetting;
    int inboundKBLimitSetting;
    int outboundKBLimitSetting;
    int maxConnectionCountSetting;
    float maxDroppedMessageFractionSetting;
    int maxMessageUtilitySetting;
    int maxSimultaneousDownloadsSetting;
    QString seedHostsSetting;
    int utilityAlphaSetting;
    int utilityBetaSetting;
    int utilityGammaSetting;
    float broadcastProbabilitySetting;
    float continueForwardProbabilitySetting;
    int showNiceQuitSetting;
    QString muteLanguageSetting;
    QString muteVersionSetting;
    int logConnectionContentsSetting;
    int logConnectionsSetting;
    int logDownloadTimeoutChangesSetting;
    int logLevelSetting;
    int logMessageHistorySetting;
    int logRollOverInSecondsSetting;
    int logRoutingHistorySetting;
    QString settingsDirSetting;
    QString styleName;
    QString sheetName;
    bool searchWithDownloads;
    QStringList otherSharedPathSetting;
    bool sendSeedNodes;
    bool acceptSeedNodes;
    bool enableWebHostCache;
    bool chkAutorunWithWin;
    QString nodePublicKey;
    QString nodePrivateKey;
    QString proxyHost;
    int proxyPort;
    QString proxyUser;
    QString proxyPassword;
    QStringList dictionary;
    QString randomSeed;
    int maxHostList;

    bool muteIniFile(QString fileName, QString &value, bool read);
    void writeMuteIniFiles(bool overwrite);
    bool muteIniFileExists(QString fileName);
    int testAndWriteIniFile(QString fileName, QString setting, bool overwrite);
    void debugMissingIniFile(QString fileName);
};

#endif

