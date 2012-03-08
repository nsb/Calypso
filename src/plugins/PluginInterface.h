/***************************************************************************
 *   Copyright (C) 2009 by Oleksiy Bilyanskyy                              *
 *   			                                                           *
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


#ifndef _PLUGIN_INTERFACE_H_
#define _PLUGIN_INTERFACE_H_

#include <QtPlugin>

//QT_BEGIN_NAMESPACE
class QString;
class QWidget;
//QT_END_NAMESPACE

//! a base class for plugins

//! All plugin classes must inherite this class and QObject.
class PluginInterface
{
public:
    virtual ~PluginInterface() {}

public slots:
    //! A description of the plugin

    //! A description of the plugin. Is not used in current version.
    virtual QString pluginDescription() const = 0;

    //! The plugin's name

    //! A name serves like an unique ID. The name is used in all operations
    //! such as installing, removing, receiving a widget
    virtual QString pluginName() const = 0;

    //! plugin's widget.

    //! Returns the widget, which is actually a plugin. Main application must
    //! delete the widget; usually, a parent widget does it. If you want to use
    //! the widget  as top-level (i.e. parent==0), please, set
    //! Qt::WA_DeleteOnClose  flag .
    virtual QWidget* pluginWidget(QWidget * parent = 0) = 0;
};

//QT_BEGIN_NAMESPACE

Q_DECLARE_INTERFACE(PluginInterface,
                    "com.beardog.retroshare.PluginInterface/1.0")

//QT_END_NAMESPACE

#endif

