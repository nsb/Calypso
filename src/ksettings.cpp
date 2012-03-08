/****************************************************************
 * This file is distributed under the following license:
 *
 * Copyright (c) 2006-2007, defnax
 * Copyright (c) 2006, Matt Edman, Justin Hipple
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


#include <QDir>
#include <QCoreApplication>

#include "ksettings.h"

#include <QWidget>
#include <QMainWindow>

#define SETTING_DATA_DIRECTORY      "DataDirectory"

#define SETTING_BWGRAPH_FILTER        "StatisticDialog/BWLineFilter"
#define SETTING_BWGRAPH_OPACITY       "StatisticDialog/Opacity"
#define SETTING_BWGRAPH_ALWAYS_ON_TOP "StatisticDialog/AlwaysOnTop"

#define DEFAULT_OPACITY             100


#if defined(Q_OS_WIN32)


#else

#endif

/* Default bandwidth graph settings */
#define DEFAULT_BWGRAPH_FILTER          (BWGRAPH_SEND|BWGRAPH_REC)
#define DEFAULT_BWGRAPH_ALWAYS_ON_TOP   false

/** The location of RetroShare's settings and configuration file. */
//#define SETTINGS_FILE   ( QDir::homePath() + "/kommute.conf")
#define SETTINGS_FILE   ( QDir::currentPath() + "/kommute.conf")

/** Default Constructor */
KSettings::KSettings()
: QSettings(SETTINGS_FILE, QSettings::IniFormat)
{ 
  //setDefault(SETTING_STYLE, DEFAULT_STYLE); 
}

/** Sets the default value of <b>key</b> to be <b>val</b>. */
void KSettings::setDefault(QString key, QVariant val)
{
  _defaults.insert(key, val);
}

/** Returns the default value for <b>key</b>. */
QVariant KSettings::defaultValue(QString key)
{
  if (_defaults.contains(key)) {
    return _defaults.value(key);
  }
  return QVariant();
}



/** Save <b>val</b> to the configuration file for the setting <b>key</b>, if
 * <b>val</b> is different than <b>key</b>'s current value. */
void KSettings::setValue(QString key, QVariant val)
{
  if (value(key) != val) {
    QSettings::setValue(key, val);
  }
}

/** Returns the value for <b>key</b>. If no value is currently saved, then the
 * default value for <b>key</b> will be returned. */
QVariant KSettings::value(QString key)
{
  return value(key, defaultValue(key));
}

/** Returns the value for <b>key</b>. If no value is currently saved, then
 * <b>defaultValue</b> will be returned. */
QVariant KSettings::value(QString key, QVariant defaultValue)
{
  return QSettings::value(key, defaultValue);
}


/** Resets all of RetroShare's settings. */
void KSettings::reset()
{
  QSettings settings(SETTINGS_FILE, QSettings::IniFormat);
  settings.clear();
}

/** Returns the bandwidth line filter. */
uint KSettings::getBWGraphFilter()
{
  return value(SETTING_BWGRAPH_FILTER, DEFAULT_BWGRAPH_FILTER).toUInt(); 
}

/** Saves the setting for whether or not the given line will be graphed */
void KSettings::setBWGraphFilter(uint line, bool status)
{
  uint filter = getBWGraphFilter();
  filter = (status ? (filter | line) : (filter & ~(line)));
  setValue(SETTING_BWGRAPH_FILTER, filter);
}

/** Get the level of opacity for the BandwidthGraph window. */
int KSettings::getBWGraphOpacity()
{
  return value(SETTING_BWGRAPH_OPACITY, DEFAULT_OPACITY).toInt();
}

/** Set the level of opacity for the BandwidthGraph window. */
void KSettings::setBWGraphOpacity(int value)
{
  setValue(SETTING_BWGRAPH_OPACITY, value);
}

/** Gets whether the bandwidth graph is always on top when displayed. */
bool KSettings::getBWGraphAlwaysOnTop()
{
  return value(SETTING_BWGRAPH_ALWAYS_ON_TOP,
               DEFAULT_BWGRAPH_ALWAYS_ON_TOP).toBool();
}

/** Sets whether the bandwidth graph is always on top when displayed. */
void KSettings::setBWGraphAlwaysOnTop(bool alwaysOnTop)
{
  setValue(SETTING_BWGRAPH_ALWAYS_ON_TOP, alwaysOnTop);
}




