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


#ifndef _PLUGINS_PAGE_H_
#define _PLUGINS_PAGE_H_

#include <QGroupBox>
#include <QString>
#include <QDir>

class QVBoxLayout;
class QTabWidget;
class QFrame;
class QLabel;
class QTextEdit;
class QSpacerItem;

class QScriptEngine;

class PluginManager;


//! A demo widget for showing plugin engine in action :)

//!     In current version this is just a container for PluginManagerWidget and
//! loaded plugin widgets. All specific actions moved to
//! PluginManagerWidget class. It contains a PluginManager instance, but it's
//! supposed that in future a pluginManager will become a global variable
class PluginsPage : public QGroupBox 
{
    Q_OBJECT

public:
  /** Default Constructor */
    PluginsPage(QWidget *parent = 0);
  /** Default Destructor */
    virtual ~PluginsPage() ;

public slots:
    //! A slot for processing new plugin registration events.

    //!     Every page, which supports plugins, has to process
    //! the PluginManager::newPluginRegistered signal. Suppose, the page knows,
    //! that there is a possible plugin "PuzzleGame"; Then, the page  should
    //! compare received pluginName with "PuzzleGame", and request the plugin
    //! widget with PluginManager::pluginWidget(..) method
    void pluginRegistered(QString pluginName);

protected:
    QVBoxLayout* pluginPageLayout;
    QGroupBox* pluginPanel;
    QVBoxLayout* pluginPanelLayout;
  
    //! Plugin widgets will be loaded into this tabs
    QTabWidget* pluginTabs ;
    
    QVBoxLayout* pmLay;
    QFrame* pmFrame;
    QSpacerItem* pmSpacer;

    //! This should be global, every page should have access to it
    PluginManager* pluginManager;
};

#endif

