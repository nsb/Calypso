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


#include <control/bandwidthevent.h>
#include "downloadthread.h"
#include "bwgraph.h"
#include <QMutex>
#include <QTime>

#define BWGRAPH_LINE_SEND       (1u<<0)
#define BWGRAPH_LINE_RECV       (1u<<1)
#define SETTING_FILTER          "LineFilter"
#define SETTING_OPACITY         "Opacity"
#define SETTING_ALWAYS_ON_TOP   "AlwaysOnTop"
#define SETTING_STYLE           "GraphStyle"
#define DEFAULT_FILTER          (BWGRAPH_LINE_SEND|BWGRAPH_LINE_RECV)
#define DEFAULT_ALWAYS_ON_TOP   false
#define DEFAULT_OPACITY         100
#define DEFAULT_STYLE           GraphFrame::AreaGraph

#define ADD_TO_FILTER(f,v,b)  (f = ((b) ? ((f) | (v)) : ((f) & ~(v))))

/* Define the format used for displaying the date and time */
#define DATETIME_FMT  "MMM dd hh:mm:ss"

/* Images used in the graph style drop-down */
#define IMG_AREA_GRAPH    ":/resources/graph-area.png"
#define IMG_LINE_GRAPH    ":/resources/graph-line.png"

/*qulonglong TotBytesDownloaded=0L;
qulonglong TotBytesUploaded=0L;
qulonglong LastTotBytesDownloaded=0L;
qulonglong LastTotBytesUploaded=0L;*/

/** Default constructor */
BandwidthGraph::BandwidthGraph(QWidget *parent, Qt::WFlags flags)
  : KWindow("BandwidthGraph", parent, flags)
{
  /* Invoke Qt Designer generated QObject setup routine */
  ui.setupUi(this);
#if defined(Q_WS_WIN)
  //setShortcut("Esc", SLOT(close()));
#else
  //setShortcut("Ctrl+W", SLOT(close()));
#endif

  /* Bind events to actions */
  createActions();

  /* Ask Mute core to notify us about bandwidth updates */


  /* Initialize Sent/Receive data counters */
  reset();
  /* Hide Bandwidth Graph Settings frame */
  showSettingsFrame(false);
  /* Load the previously saved settings */
  loadSettings();

  /* Turn off opacity group on unsupported platforms */
#if defined(Q_WS_WIN)
  if(!(QSysInfo::WV_2000 <= QSysInfo::WindowsVersion <= QSysInfo::WV_2003)) {
    ui.frmOpacity->setVisible(false);
  }
#endif
  
#if defined(Q_WS_X11)
  ui.frmOpacity->setVisible(false);
#endif
}

/** Custom event handler. Checks if the event is a bandwidth update event. If it
 * is, it will add the data point to the history and updates the graph. */
void
BandwidthGraph::customEvent(QEvent *event)
{
  if (event->type() == CustomEventType::BandwidthEvent) {
    BandwidthEvent *bw = (BandwidthEvent *)event;
    updateGraph(bw->bytesRead(), bw->bytesWritten());
  }
}

void BandwidthGraph::BandwidthGraphtimerEvent( QTimerEvent * )
{

  //
  /*updateGraph(TotBytesDownloaded-LastTotBytesDownloaded,TotBytesUploaded-LastTotBytesUploaded);
  LastTotBytesDownloaded=TotBytesDownloaded;
  LastTotBytesUploaded=TotBytesUploaded;*/
}

/** Binds events to actions. */
void
BandwidthGraph::createActions()
{
  connect(ui.btnToggleSettings, SIGNAL(toggled(bool)),
      this, SLOT(showSettingsFrame(bool)));

  connect(ui.btnReset, SIGNAL(clicked()),
      this, SLOT(reset()));

  connect(ui.btnSaveSettings, SIGNAL(clicked()),
      this, SLOT(saveChanges()));

  connect(ui.btnCancelSettings, SIGNAL(clicked()),
      this, SLOT(cancelChanges()));
  
  connect(ui.sldrOpacity, SIGNAL(valueChanged(int)),
      this, SLOT(setOpacity(int)));
}

/** Adds new data to the graph. */
void
BandwidthGraph::updateGraph(quint64 bytesRead, quint64 bytesWritten)
{
  /* Graph only cares about kilobytes */
  ui.frmGraph->addPoints(bytesRead/1024.0, bytesWritten/1024.0);
}

/** Loads the saved Bandwidth Graph settings. */
void
BandwidthGraph::loadSettings()
{
  /* Set window opacity slider widget */
  //ui.sldrOpacity->setValue(getSetting(SETTING_OPACITY, DEFAULT_OPACITY).toInt());
  setOpacity(ui.sldrOpacity->value());

  /* Set whether the window appears on top. */
  /*ui.chkAlwaysOnTop->setChecked(getSetting(SETTING_ALWAYS_ON_TOP,
                                           DEFAULT_ALWAYS_ON_TOP).toBool());*/
  if (ui.chkAlwaysOnTop->isChecked()) {
    setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
  } else {
    setWindowFlags(windowFlags() & ~Qt::WindowStaysOnTopHint);
  }

  /* Set the line filter checkboxes accordingly */
  /*uint filter = getSetting(SETTING_FILTER, DEFAULT_FILTER).toUInt();
  ui.chkReceiveRate->setChecked(filter & BWGRAPH_LINE_RECV);
  ui.chkSendRate->setChecked(filter & BWGRAPH_LINE_SEND);*/

  /* Set whether we are plotting bandwidth as area graphs or not */
  /*int graphStyle = getSetting(SETTING_STYLE, DEFAULT_STYLE).toInt();
  if (graphStyle < 0 || graphStyle >= ui.cmbGraphStyle->count()) {*/
  int  graphStyle = DEFAULT_STYLE;
  //}*/
  ui.cmbGraphStyle->setCurrentIndex(graphStyle);
  ui.frmGraph->setGraphStyle((GraphFrame::GraphStyle)graphStyle);

  /* Set graph frame settings */
  ui.frmGraph->setShowCounters(ui.chkReceiveRate->isChecked(),
                               ui.chkSendRate->isChecked());
}

/** Resets the log start time. */
void
BandwidthGraph::reset()
{
  /* Set to current time */
  ui.statusbar->showMessage(tr("Since:") + " " + 
			    QDateTime::currentDateTime()
			    .toString(DATETIME_FMT));
  /* Reset the graph */
  ui.frmGraph->resetGraph();
}

/** Saves the Bandwidth Graph settings and adjusts the graph if necessary. */
void
BandwidthGraph::saveChanges()
{
  /* Hide the settings frame and reset toggle button */
  showSettingsFrame(false);
  
  /* Save the opacity and graph style */
  //saveSetting(SETTING_OPACITY, ui.sldrOpacity->value());
  //saveSetting(SETTING_STYLE, ui.cmbGraphStyle->currentIndex());

  /* Save the Always On Top setting */
  //saveSetting(SETTING_ALWAYS_ON_TOP, ui.chkAlwaysOnTop->isChecked());
  if (ui.chkAlwaysOnTop->isChecked()) {
    setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
  } else {
    setWindowFlags(windowFlags() & ~Qt::WindowStaysOnTopHint);
  }
  setOpacity(ui.sldrOpacity->value());

  /* Save the line filter values */
  uint filter = 0;
  ADD_TO_FILTER(filter, BWGRAPH_LINE_RECV, ui.chkReceiveRate->isChecked());
  ADD_TO_FILTER(filter, BWGRAPH_LINE_SEND, ui.chkSendRate->isChecked());
  //saveSetting(SETTING_FILTER, filter);


  /* Update the graph frame settings */
  ui.frmGraph->setShowCounters(ui.chkReceiveRate->isChecked(),
                               ui.chkSendRate->isChecked());
  ui.frmGraph->setGraphStyle((GraphFrame::GraphStyle)ui.cmbGraphStyle->currentIndex());
  
  /* A change in window flags causes the window to disappear, so make sure
   * it's still visible. */
  showNormal();
}

/** Simply restores the previously saved settings. */
void 
BandwidthGraph::cancelChanges()
{
  /* Hide the settings frame and reset toggle button */
  showSettingsFrame(false);

  /* Reload the settings */
  loadSettings();
}

/** Toggles the Settings pane on and off, changes toggle button text. */
void
BandwidthGraph::showSettingsFrame(bool show)
{
  static QSize minSize = minimumSize();
  
  QSize newSize = size();
  if (show) {
    /* Extend the bottom of the bandwidth graph and show the settings */
    ui.frmSettings->setVisible(true);
    ui.btnToggleSettings->setChecked(true);
    ui.btnToggleSettings->setText(tr("Hide Settings"));

    /* 6 = vertical spacing between the settings frame and graph frame */
    newSize.setHeight(newSize.height() + ui.frmSettings->height() + 6);
  } else {
    /* Shrink the height of the bandwidth graph and hide the settings */
    ui.frmSettings->setVisible(false);
    ui.btnToggleSettings->setChecked(false);
    ui.btnToggleSettings->setText(tr("Show Settings"));
    
    /* 6 = vertical spacing between the settings frame and graph frame */
    newSize.setHeight(newSize.height() - ui.frmSettings->height() - 6);
    setMinimumSize(minSize);
  }
  resize(newSize);
}

/** Sets the opacity of the Bandwidth Graph window. */
void
BandwidthGraph::setOpacity(int value)
{
  qreal newValue = value / 100.0;
  
  /* Opacity only supported by Mac and Win32 */
#if defined(Q_WS_MAC)
  this->setWindowOpacity(newValue);
  ui.lblPercentOpacity->setText(QString::number(value));
#elif defined(Q_WS_WIN)
  if(QSysInfo::WV_2000 <= QSysInfo::WindowsVersion <= QSysInfo::WV_2003) {
    this->setWindowOpacity(newValue);
    ui.lblPercentOpacity->setText(QString::number(value));
  }
#else
  Q_UNUSED(newValue);
#endif
}

/** Overloads the default show() slot so we can set opacity. */
void
BandwidthGraph::showWindow()
{
  /* Load saved settings */
  loadSettings();
  /* Show the window */
  KWindow::showWindow();
}

