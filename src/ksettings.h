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



#ifndef _KSETTINGS_H
#define _KSETTINGS_H

#include <QHash>
#include <QSettings>

#include "linetypes.h"


//Forward declaration.
class QWidget;
class QMainWindow;

/** Handles saving and restoring RShares's settings, such as the
 * location of Tor, the control port, etc.
 *
 * NOTE: Qt 4.1 documentation states that constructing a QSettings object is
 * "very fast", so we shouldn't need to create a global instance of this
 * class.
 */
class KSettings : public QSettings
{
  
public:
  /** Default constructor. */
  KSettings();

  /** Resets all of Rshare's settings. */
  static void reset();
  
  /** Sets the default value of <b>key</b> to be <b>val</b>. */
  void setDefault(QString key, QVariant val);
  /** Returns the default value for <b>key</b>. */
  QVariant defaultValue(QString key);
  /** Save <b>val</b> to the configuration file for the setting <b>key</b>, if
   * <b>val</b> is different than <b>key</b>'s current value. */
  void setValue(QString key, QVariant val);
  /** Returns the value for <b>key</b>. If no value is currently saved, then
   * the default value for <b>key</b> will be returned. */
  QVariant value(QString key);
  /** Returns the value for <b>key</b>. If no value is currently saved, then
   * <b>defaultValue</b> will be returned. */
  QVariant value(QString key, QVariant defaultValue);


  /* Get the bandwidth graph line filter. */
  uint getBWGraphFilter();
  /** Set the bandwidth graph line filter. */
  void setBWGraphFilter(uint line, bool status);

  /** Set the bandwidth graph opacity setting. */
  int getBWGraphOpacity();
  /** Set the bandwidth graph opacity settings. */
  void setBWGraphOpacity(int value);

  /** Gets whether the bandwidth graph is always on top. */
  bool getBWGraphAlwaysOnTop();
  /** Sets whether the bandwidth graph is always on top. */
  void setBWGraphAlwaysOnTop(bool alwaysOnTop);
  


private:
  QHash<QString,QVariant> _defaults;



};

#endif

