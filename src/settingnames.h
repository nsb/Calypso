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

 /**
@author Andy Gebauer - feivelda@googlemail.com
*/

#ifndef SETTINGNAMES_H
#define SETTINGNAMES_H

#define SETTING_NAMES_DOWNLOAD_WEB_HOST_CACHE                   "DownloadWebHostCachesSettings"

#define SETTING_NAMES_GROUPFIREWALLSETTINGS                     "FirewallSettings"
#define SETTING_NAMES_GROUPSHARESETTINGS                        "ShareSettings"
#define SETTING_NAMES_GROUPKEYSETTINGS                          "KeySettings"
#define SETTING_NAMES_GROUPMIMETYPESSETTINGS                    "MimeTypesSettings"
#define SETTING_NAMES_GROUPWEBSETTINGS                          "WebSettings"
#define SETTING_NAMES_GROUPDOWNLOADSETTINGS                     "DownloadSettings"
#define SETTING_NAMES_GROUPDOWNLOAD_TIMEOUTSETTINGS             "DownloadTimeoutSettings"
#define SETTING_NAMES_GROUPMESSAGELIMITSETTINGS                 "MessageLimitSettings"
#define SETTING_NAMES_GROUPMAXSETTINGS                          "MaxValueSettings"
#define SETTING_NAMES_GROUPUTILITYSETTINGS                      "UtilitySettings"
#define SETTING_NAMES_GROUPPROBABILITYSETTINGS                  "ProbabilitySettings"
#define SETTING_NAMES_GROUPMISCELLANEOUS                        "Miscellaneous"
#define SETTING_NAMES_GROUPLOG                                  "Logging"
#define SETTING_NAMES_GROUPSTYLE                                "Style"
#define SETTING_NAMES_GROUPPROXY                                "Proxy"

#define SETTING_NAMES_FIRST_START                               "First_Start"
#define SETTING_NAMES_RELOAD_WEB_HOST_CACHES                    "Reload_Web_Host_Caches"
#define SETTING_NAMES_MYIP                    			"My_IP"
#define SETTING_NAMES_DICTIONARY               			"Dictionary"

#define SETTING_NAMES_HAS_FIREWALL                              "Has_Firewall"
#define SETTING_NAMES_PORT                                      "Port_Number"

//#define SETTING_NAMES_SHARE_PATH                                "Share_Path"
#define SETTING_NAMES_INCOMING_PATH                             "Incoming_Path"
#define SETTING_NAMES_TEMP_PATH                                 "Temp_Path"
#define SETTING_NAMES_SEND_QUEUE_SIZE_PER_CONNECTION            "Send_Queue_Size_Per_Connection"
#define SETTING_NAMES_TARGET_CONNECTION_COUNT                   "Target_Connection_Count"
#define SETTING_NAMES_OTHER_SHARE_PATH                          "Other_Share_Path"

#define SETTING_NAMES_KEY_SIZE                                  "Key_Size"
#define SETTING_NAMES_NODE_PUBLIC_KEY                           "NodePublicKey"
#define SETTING_NAMES_NODE_PRIVATE_KEY                          "NodePrivateKey"
#define SETTING_NAMES_SEED                                      "Seed_String"
#define SETTING_NAMES_RANDOMSEED                                "Random_Seed"

#define SETTING_NAMES_MIME_TYPES                                "Mime_Types"

#define SETTING_NAMES_WEB_HOST_CACHE                            "Web_Host_Cache"
#define SETTING_NAMES_WEB_HOST_CACHE_POST_INTERVAL              "Web_Host_Cache_Post_Interval"
#define SETTING_NAMES_USE_MAJORITY_ROUTING                      "Use_Majority_Routing"
#define SETTING_NAMES_ENABLE_WEB_HOST_CACHE                     "Enable_Web_Host_Cache"
#define SETTING_NAMES_SEND_SEED_NODES                           "Send_Seed_Nodes"
#define SETTING_NAMES_ACCEPT_SEED_NODES                         "Accept_Seed_Nodes"
#define SETTING_NAMES_MAXHOSTLIST                               "Max_Host_List"

#define SETTING_NAMES_DOWNLOAD_CHUNK_RETRIES                    "Download_Chunk_Retries"
#define SETTING_NAMES_DOWNLOAD_FILE_INFO_RETRIES                "Download_File_Info_Retries"
#define SETTING_NAMES_DOWNLOAD_FRESH_ROUTE_PROBABILITY          "Download_Fresh_Route_Probability"
#define SETTING_NAMES_PRINT_SEARCH_SYNC_TRACE                   "Print_Search_Sync_Trace"

#define SETTING_NAMES_DOWNLOAD_TIMEOUT_CURRENT_TIMEOUT_WEIGHT   "Download_Timeout_Current_Timeout_Weight"
#define SETTING_NAMES_DOWNLOAD_TIMEOUT_MILLI_SECONDS            "Download_Timeout_Milli_Seconds"
#define SETTING_NAMES_DOWNLOAD_TIMEOUT_RECENT_CHUNK_WEIGHT      "Download_Timeout_Recent_Chunk_Weight"

#define SETTING_NAMES_INBOUND_KB_LIMIT                          "Inbound_KB_Limit"
#define SETTING_NAMES_OUTBOUND_KB_LIMIT                         "Outbound_KB_Limit"

#define SETTING_NAMES_MAX_CONNECTION_COUNT                      "Max_Connection_Count"
#define SETTING_NAMES_MAX_DROPPED_MESSAGE_FRACTION              "Max_Dropped_Message_Fraction"
#define SETTING_NAMES_MAX_MESSAGE_UTILITY                       "Max_Message_Utility"
#define SETTING_NAMES_MAX_SIMULTANEOUS_DOWNLOADS                "Max_Simultaneous_Downloads"
#define SETTING_NAMES_MAX_SUBFOLDER_DEPTH                       "Max_Subfolder_Depth"

#define SETTING_NAMES_UTILITY_ALPHA                             "Utility_Alpha"
#define SETTING_NAMES_UTILITY_BETA                              "Utility_Beta"
#define SETTING_NAMES_UTILITY_GAMMA                             "Utility_Gamma"

#define SETTING_NAMES_BROADCAST_PROBABILITY                     "Broadcast_Probability"
#define SETTING_NAMES_CONTINUE_FORWARD_PROBABILITY              "ContinueForwardProbability"

#define SETTING_NAMES_SHOW_NICE_QUIT                            "Show_Nice_Quit"
#define SETTING_NAMES_MUTE_LANGUAGE                             "Mute_Language"
#define SETTING_NAMES_MUTE_VERSION                              "Mute_Version"

#define SETTING_NAMES_LOG_CONNECTION_CONTENTS                   "Log_Connection_Contents"
#define SETTING_NAMES_LOG_CONNECTIONS                           "Log_Connections"
#define SETTING_NAMES_LOG_DOWNLOAD_TIMEOUT_CHANGES              "Log_Download_Timeout_Changes"
#define SETTING_NAMES_LOG_LEVEL                                 "Log_Level"
#define SETTING_NAMES_LOG_MESSAGE_HISTORY                       "Log_Message_History"
#define SETTING_NAMES_LOG_ROLL_OVER_IN_SECONDS                  "Log_Roll_Over_In_Seconds"
#define SETTING_NAMES_LOG_ROUTING_HISTORY                       "Log_Routing_History"

#define SETTING_NAMES_STYLENAME                                 "StyleName"
#define SETTING_NAMES_SHEETNAME                                 "SheetName"
#define SETTING_NAMES_DOWNLOADSWITHSEARCH                       "DownloadsWithSearch"
#define SETTING_NAMES_CHKAUTORUNWITHWIN                         "CheckAutorunWithWin"

#define SETTING_NAMES_PROXYHOST					"ProxyHost"
#define SETTING_NAMES_PROXYPORT					"ProxyPort"
#define SETTING_NAMES_PROXYUSER					"ProxyUser"
#define SETTING_NAMES_PROXYPASSWORD				"ProxyPassword"

#define INIFILE_WRITE                                           false
#define INIFILE_READ                                            true

#define INIFILE_OVERWRITE                                       true
#define INIFILE_KEEP                                            false

#endif

