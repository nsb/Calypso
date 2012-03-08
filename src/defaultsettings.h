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

#ifndef DEFAULTSETTINGS_H
#define DEFAULTSETTINGS_H

#define LIST_SEPARATOR                                                  ","

#define WEBHOSTCACHESLISTDOWNLOAD                                       "http://kommute.sourceforge.net/mcaches.txt"

#define DEFAULT_SETTINGS_SETTINGSFILE                                   "kommuterc"
#define DEFAULT_SETTINGS_SETTINGSDIR                                    ".kommute/"
#define DEFAULT_SETTINGS_SETTINGSFOMRMAT                                QSettings::IniFormat

#define DEFAULT_SETTINGS_STANDARDFIRSTSTART                             true
#define DEFAULT_SETTINGS_STANDARDRELOADWEBHOSTCACHES                    false
#define DEFAULT_SETTINGS_STANDARDMYIP                    		""
#define DEFAULT_SETTINGS_STANDARDDICTIONARY                    		"train,fox,blue,green,bird,eagle,onto,race,ants,mute,down,cloud"

#define DEFAULT_SETTINGS_STANDARDFIREWALL                               true
#define DEFAULT_SETTINGS_STANDARDSHAREPATH                              ""
#define DEFAULT_SETTINGS_STANDARDSENDQUEUESIZEPERCONNECTION             300
#define DEFAULT_SETTINGS_STANDARDTARGETCONNECTIONCOUNT                  4
#define DEFAULT_SETTINGS_STANDARDKEYSIZE                                512
#define DEFAULT_SETTINGS_STANDARDSEED                                   ""
#define DEFAULT_SETTINGS_STANDARDSEEDLENGTH                             20
#define DEFAULT_SETTINGS_STANDARDPORT                                   4900
#define DEFAULT_SETTINGS_STANDARDMIMETYPES                              ".html|text/html,.htm|text/html,.css|text/css,.txt|text/plain,.asc|text/plain,.rtx|text/richtext,.rtf|text/rtf,.xml|text/xml,.jpg|image/jpeg,.jpeg|image/jpeg,.jpe|image/jpeg,.gif|image/gif,.png|image/png,.tif|image/tif,.tiff|image/tif,.mp3|audio/mpeg,.mp2|audio/mpeg,.ram|audio/x-pn-realaudio,.rm|audio/x-pn-realaudio,.ra|audio/x-realaudio,.wav|audio/x-wav,.aif|audio/x-aiff,.aiff|audio/x-aiff,.aifc|audio/x-aiff,.mpeg|video/mpeg,.mpg|video/mpeg,.mpe|video/mpeg,.qt|video/quicktime,.mov|video/quicktime,.avi|video/x-msvideo,.hqx|application/mac-binhex40,.cpt|application/mac-compactpro,.doc|application/msword,.pdf|application/pdf,.bin|application/octet-stream,.dsm|application/octet-stream,.lha|application/octet-stream,.lzh|application/octet-stream,.exe|application/octet-stream,.class|application/octet-stream,.ps|application/postscript,.eps|application/postscript,.ppt|application/vnd.ms-powerpoint,.bz2|application/x-bzip2,.dvi|application/x-dvi,.gz|application/x-gzip,.tgz|application/x-gzip,.rpm|application/x-rpm,.swf|application/x-shockwave-flash,.sit|application/x-stuffit,.tar|application/x-tar,.tex|application/x-tex,.zip|application/zip"
#define DEFAULT_SETTINGS_STANDARDWEBHOSTCACHE                           "http://grantgalitz.com/Beacon/gwc.php,http://gwc.mitigated.net/gwc.php,http://mcache.boisard.com/mcache.php,http://mcache.guenever.net/mcache.php,http://mcache.mc.funpic.de/mcache.php,http://mcache.northcountrynotes.org/mcache.php,http://mute.filesharinghelp.com/mcache.php,http://mutecache.ath.cx/mcache.php,http://mutenext.net23.net/mcache.php,http://reezer.freeshell.org/mcache/mcache.php,http://www.mutenext.co.cc/mwc/mcache.php"
#define DEFAULT_SETTINGS_STANDARDMAXHOSTLIST                            500

#define DEFAULT_SETTINGS_STANDARDWEBHOSTCACHEPOSTINTERVAL               3660
#define DEFAULT_SETTINGS_STANDARDUSEMAJORITYROUTING                     0
#define DEFAULT_SETTINGS_STANDARDDOWNLOADCHUNKRETRIES                   10
#define DEFAULT_SETTINGS_STANDARDDOWNLOADFILEINFORETRIES                5
#define DEFAULT_SETTINGS_STANDARDDOWNLOADFRESHROUTEPROBABILITY          0.25
#define DEFAULT_SETTINGS_STANDARDPRINTSEARCHSYNCTRACE                   0
#define DEFAULT_SETTINGS_STANDARDDOWNLOADTIMEOUTCURRENTTIMEOUTWEIGHT    2
#define DEFAULT_SETTINGS_STANDARDDOWNLOADTIMEOUTMILLISECONDS            60000
#define DEFAULT_SETTINGS_STANDARDDOWNLOADTIMEOUTRECENTCHUNKWEIGHT       1
#define DEFAULT_SETTINGS_STANDARDINBOUNDKBLIMIT                    -1
#define DEFAULT_SETTINGS_STANDARDOUTBOUNDKBLIMIT                   -1
#define DEFAULT_SETTINGS_STANDARDMAXCONNECTIONCOUNT                     16
#define DEFAULT_SETTINGS_STANDARDMAXDROPPEDMESSAGEFRACTION              0.1
#define DEFAULT_SETTINGS_STANDARDMAXMESSAGEUTILITY                      35
#define DEFAULT_SETTINGS_STANDARDMAXSIMULTANEOUSDOWNLOADS               10
#define DEFAULT_SETTINGS_STANDARDMAXSUBFOLDERDEPTH                      10
#define DEFAULT_SETTINGS_STANDARDUTILITYALPHA                           0
#define DEFAULT_SETTINGS_STANDARDUTILITYBETA                            0
#define DEFAULT_SETTINGS_STANDARDUTILITYGAMMA                           5
#define DEFAULT_SETTINGS_STANDARDBROADCASTPROBABILITY                   0
#define DEFAULT_SETTINGS_STANDARDCONTINUEFORWARDPROBABILITY             0.66
#define DEFAULT_SETTINGS_STANDARDSHOWNICEQUIT                           0
#define DEFAULT_SETTINGS_STANDARDMUTELANGUAGE                           "English"
#define DEFAULT_SETTINGS_STANDARDMUTEVERSION                            "0.24"
#define DEFAULT_SETTINGS_STANDARDLOGCONNECTIONCONTENTS                  0
#define DEFAULT_SETTINGS_STANDARDLOGCONNECTIONS                         0
#define DEFAULT_SETTINGS_STANDARDLOGDOWNLOADTIMEOUTCHANGES              0
#define DEFAULT_SETTINGS_STANDARDLOGLEVEL                               0
#define DEFAULT_SETTINGS_STANDARDLOGMESSAGEHISTORY                      0
#define DEFAULT_SETTINGS_STANDARDLOGROLLOVERINSECONDS                   3600
#define DEFAULT_SETTINGS_STANDARDLOGROUTINGHISTORY                      0
#define DEFAULT_SETTINGS_DOWNLOADSWITHSEARCH                            false
#define DEFAULT_SETTINGS_ENABLEWEBHOSTCACHE															true
#define DEFAULT_SETTINGS_SENDSEEDNODES																	true
#define DEFAULT_SETTINGS_ACCEPTSEEDNODES																true
#define DEFAULT_SETTINGS_CHKAUTORUNWITHWIN								true
#define DEFAULT_SETTINGS_STANDARDPROXYHOST                                     ""
#define DEFAULT_SETTINGS_STANDARDPROXYPORT                                     0
#define DEFAULT_SETTINGS_STANDARDPROXYUSER                                     ""
#define DEFAULT_SETTINGS_STANDARDPROXYPASSWORD                                 ""

#endif

