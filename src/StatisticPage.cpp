/****************************************************************
 *  Kommute is distributed under the following license:
 *
 *  Copyright (C) 2008, defnax
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
#include "kommute.h"
#include "kommuteutils.h"
#include "linetypes.h"
#include <QHeaderView>
#include <QMutex>
#include <QTime>
#include "settings.h"

extern class Settings *settings;
extern char *muteInternalDirectoryTag;

//#define BWGRAPH_LINE_SEND       (1u<<0)
//#define BWGRAPH_LINE_RECV       (1u<<1)
//#define SETTING_FILTER          "LineFilter"
#define SETTING_STYLE           "GraphStyle"
#define DEFAULT_STYLE           GraphFrame::AreaGraph


/** Redraw graph every 1000ms **/
#define REFRESH_RATE    1000

/* Define the format used for displaying the date and time */
#define DATETIME_FMT  "yyyy MM dd hh:mm:ss"

/* Images used in the graph style drop-down */
#define IMG_AREA_GRAPH    ":/resources/graph-area.png"
#define IMG_LINE_GRAPH    ":/resources/graph-line.png"

// TRANSFER
extern long TotMessagesReceived;
qulonglong LastTotMessagesReceived=0L;
extern long TotMessagesToAll;
qulonglong LastTotMessagesToAll=0L;

// DOWNLOAD
qulonglong TotBytesDownloaded=0L; // bytes downloaded : share protocol, for our downloads.
qulonglong TotBytesReceived=0L; // bytes received, all protocols, not only for share protocol
qulonglong LastTotBytesDownloaded=0L;
qulonglong LastTotBytesReceived=0L;
qulonglong LastStoredTotBytesDownloaded;
qulonglong LastStoredTotBytesReceived;

// UPLOAD
qulonglong TotBytesUploaded=0L;
qulonglong TotBytesEmitted=0L; // bytes emitted, all protocols, not only for share protocol
qulonglong LastTotBytesUploaded=0L;
qulonglong LastTotBytesEmitted=0L;
qulonglong LastStoredTotBytesUploaded;
qulonglong LastStoredTotBytesEmitted;

// CONNEXIONS
// TIMES
QTime UpTime;
int UpDays;
// RECORDS


bool dayChange;
QMutex StatLock;
static int Timer=0;

/** Returns a formatted string with the correct size suffix. */
QString
totalCountToString(qreal total)
{
  /* Determine the correct size suffix */
  if (total < 1000) {
    /* Use no suffix */
    return QString("%1").arg(total, 0, 'f', 0);
  } else if (total < 1000000) {
    /* Use k suffix */
    return QString("%1 k").arg(total/1000.0, 0, 'f', 2);
  } else {
    /* Use M suffix */
    return QString("%1 M").arg(total/1000000.0, 0, 'f', 2);
  }
}
/** Constructor */
void Kommute::initStatisticPage()
{
   loadStatistics();
   UpDays=0;
   dayChange=false;
   UpTime.start();
   /* Create Bandwidth Graph related QObjects */
  
   /* Set header resize modes and initial section sizes */
   QHeaderView * _stheader = statsTreeWidget-> header();
   _stheader->resizeSection ( 0, 210 );

  /* Bind events to actions */
  createActions();

  /* Ask Mute to notify us about bandwidth updates */


  /* Initialize Sent/Receive data counters */
  reset();
  
  /* Hide Bandwidth Graph Settings frame */
  showSettingsFrame(false);
  showUploadsFrame(false);
  /* Load the previously saved settings */
  //loadSettings();

  
  QAbstractItemModel * model =statsTreeWidget->model();
  QModelIndex ind2;
  // set Times --> Session --> Since
  ind2=model->index(4,0).child(0,0).child(1,1);
  model->setData(ind2,QDateTime::currentDateTime()
			    .toString(DATETIME_FMT));

  /* Turn off opacity group on unsupported platforms */
#if defined(Q_WS_WIN)

#endif
  
#if defined(Q_WS_X11)

#endif
  Timer=startTimer(REFRESH_RATE);
}

void Kommute::loadStatistics()
{
 QString fullName (settings->getTempDirectory());
  fullName += "/";
  fullName += muteInternalDirectoryTag;
  fullName += "/statistics";
  QSettings  stats ( fullName, QSettings::IniFormat);
  // TRANSFER statistics
  stats.beginGroup("TRANSFER");
  LastTotMessagesReceived = stats.value("TotMessagesReceived", 0).toInt();
  LastTotMessagesToAll = stats.value("TotMessagesToAll", 0).toInt();
  stats.endGroup();
  // DOWNLOAD statistics
  stats.beginGroup("DOWNLOAD");
  LastStoredTotBytesDownloaded = stats.value("TotBytesDownloaded", 0).toInt();
  LastStoredTotBytesReceived = stats.value("TotBytesReceived", 0).toInt();
  stats.endGroup();
  // UPLOAD statistics
  stats.beginGroup("UPLOAD");
  LastStoredTotBytesUploaded = stats.value("TotBytesUploaded", 0).toInt();
  LastStoredTotBytesEmitted = stats.value("TotBytesEmitted", 0).toInt();
  stats.endGroup();
}

void Kommute::saveStatistics()
{
 QString fullName (settings->getTempDirectory());
  fullName += "/";
  fullName += muteInternalDirectoryTag;
  fullName += "/statistics";
  QSettings  stats ( fullName, QSettings::IniFormat);
  // TRANSFER statistics
  stats.beginGroup("TRANSFER");
  stats.setValue("TotMessagesReceived", TotMessagesReceived+LastTotMessagesReceived);
  stats.setValue("TotMessagesToAll", TotMessagesToAll+LastTotMessagesToAll);
  stats.endGroup();
  // DOWNLOAD statistics
  stats.beginGroup("DOWNLOAD");
  stats.setValue("TotBytesDownloaded", TotBytesDownloaded+LastTotBytesDownloaded);
  stats.setValue("TotBytesReceived", TotBytesReceived+LastTotBytesReceived);
  stats.endGroup();
  // UPLOAD statistics
  stats.beginGroup("UPLOAD");
  stats.setValue("TotBytesUploaded", TotBytesUploaded+LastTotBytesUploaded);
  stats.setValue("TotBytesEmitted", TotBytesEmitted+LastTotBytesEmitted);
  stats.endGroup();
// CONNEXIONS statistics
// TIMES statistics
// RECORDS


}

/** Default destructor 
StatisticPage::~StatisticPage()
{
  //delete _settings;
}*/

/**
 Custom event handler. Checks if the event is a bandwidth update event. If it
 is, it will add the data point to the history and updates the graph.*/

void
Kommute::gcustomEvent(QEvent *event)
{
  if (event->type() == CustomEventType::BandwidthEvent) {
    BandwidthEvent *bw = (BandwidthEvent *)event;
    updateGraph(bw->bytesRead(), bw->bytesWritten());
  }
}

void Kommute::timerEvent( QTimerEvent * )
{
  QAbstractItemModel * model =statsTreeWidget->model();
  QModelIndex ind1;
  
  // set UL:DL Ratio --> session ratio
  ind1=model->index(0,0).child(0,0).child(0,1);
  model->setData(ind1,(TotBytesEmitted/1024.0) / (TotBytesReceived/1024.0));
   //set UL:DL Ratio --> cumulative ratio
  ind1=model->index(0,0).child(0,0).child(1,1);
  model->setData(ind1,(LastStoredTotBytesEmitted/1024.0) / (LastStoredTotBytesReceived/1024.0));
  
  // set transfer --> message received --> total
  ind1=model->index(0,0).child(1,0).child(0,1);
  model->setData(ind1,totalCountToString(TotMessagesReceived));
  // set transfer --> message received --> total to ALL
  ind1=model->index(0,0).child(1,0).child(1,1);
  model->setData(ind1,totalCountToString(TotMessagesToAll));
  
  // set download --> session bytes
  ind1=model->index(1,0).child(0,0).child(0,1);
  model->setData(ind1,totalToString(TotBytesDownloaded/1024.0));
  // set received --> session bytes
  ind1=model->index(1,0).child(0,0).child(1,1);
  model->setData(ind1,totalToString(TotBytesReceived/1024.0));
  // set download --> Cumulative bytes
  ind1=model->index(1,0).child(1,0).child(0,1);
  model->setData(ind1,totalToString(LastStoredTotBytesDownloaded/1024.0));
  // set received --> Cumulative bytes
  ind1=model->index(1,0).child(1,0).child(1,1);
  model->setData(ind1,totalToString(LastStoredTotBytesReceived/1024.0));
  
  // set upload --> session bytes
  ind1=model->index(2,0).child(0,0).child(0,1);
  model->setData(ind1,totalToString(TotBytesUploaded/1024.0));
  // set emitted --> session bytes
  ind1=model->index(2,0).child(0,0).child(1,1);
  model->setData(ind1,totalToString(TotBytesEmitted/1024.0));  
    // set upload --> Cumulative bytes
  ind1=model->index(2,0).child(1,0).child(0,1);
  model->setData(ind1,totalToString(LastStoredTotBytesUploaded/1024.0));
  // set emitted --> Cumulative bytes
  ind1=model->index(2,0).child(1,0).child(1,1);
  model->setData(ind1,totalToString(LastStoredTotBytesEmitted/1024.0));
  
  // set Times --> Session --> Uptime
  ind1=model->index(4,0).child(0,0).child(0,1);
  if(! dayChange && (UpTime.elapsed()/1000 > 60*60*23)) dayChange=true;
  if( dayChange && (UpTime.elapsed() <10000)) {dayChange=false;UpDays++;}

  QTime elapsed;
  elapsed= QTime(0,0,0).addMSecs(UpTime.elapsed());
  if( UpDays>0)
      model->setData(ind1,QString(tr("%1 days ")).arg(UpDays)+elapsed.toString("hh:mm:ss"));
  else
      model->setData(ind1,elapsed.toString("hh:mm:ss"));
  //
  updateGraph(TotBytesDownloaded-LastTotBytesDownloaded,TotBytesUploaded-LastTotBytesUploaded);
  updateGraph2(TotBytesReceived-LastTotBytesReceived,TotBytesEmitted-LastTotBytesEmitted);
  LastTotBytesDownloaded=TotBytesDownloaded;
  LastTotBytesUploaded=TotBytesUploaded;
  LastTotBytesReceived=TotBytesReceived;
  LastTotBytesEmitted=TotBytesEmitted;
}

/**
 Binds events to actions
*/
void
Kommute::createActions()
{
  /* Downloads/Uploads Settings Frame :*/  
  connect(btnToggleSettings, SIGNAL(toggled(bool)),
      this, SLOT(showSettingsFrame(bool)));

  connect(btnReset, SIGNAL(clicked()),
      this, SLOT(reset()));

  connect(btnSaveSettings, SIGNAL(clicked()),
      this, SLOT(saveChanges()));

  connect(btnCancelSettings, SIGNAL(clicked()),
      this, SLOT(cancelChanges()));
  
  /* Incoming/Outgoing Settings Frame :*/    
  connect(btnToggleSettings2, SIGNAL(toggled(bool)),
      this, SLOT(showUploadsFrame(bool)));
      
  connect(btnReset2, SIGNAL(clicked()),
      this, SLOT(reset_received_emitted()));
      
  connect(btnSaveSettings2, SIGNAL(clicked()),
      this, SLOT(saveChanges()));
      
  connect(btnCancelSettings2, SIGNAL(clicked()),
      this, SLOT(cancelChanges()));
  

}

/**
 Adds new data to the graph
*/
void
Kommute::updateGraph(quint64 bytesRead, quint64 bytesWritten)
{
  /* Downloads/Uploads Graph only cares about kilobytes */
  frmGraph->addPoints(bytesRead/1024.0, bytesWritten/1024.0);
  
}

/**
 Adds new data to the graph
*/
void
Kommute::updateGraph2(quint64 bytesRead, quint64 bytesWritten)
{  
  /* Incoming/Outgoing Graph only cares about kilobytes */
  frmGraph2->addPoints(bytesRead/1024.0, bytesWritten/1024.0);
}

/**
 Loads the saved Bandwidth Graph settings
*/
void
Kommute::loadSettings()
{

  /* Set the line filter checkboxes accordingly */
  //uint filter = _settings->getBWGraphFilter();
  //ui.chkReceiveRate->setChecked(filter & BWGRAPH_REC);
  //ui.chkSendRate->setChecked(filter & BWGRAPH_SEND);
  
   /* Set whether we are plotting bandwidth as area graphs or not */
  /*int graphStyle = getSetting(SETTING_STYLE, DEFAULT_STYLE).toInt();
  if (graphStyle < 0 || graphStyle >= cmbGraphStyle->count()) {*/
  int graphStyle = DEFAULT_STYLE;
  //}
  cmbGraphStyle->setCurrentIndex(graphStyle);
  frmGraph->setGraphStyle((GraphFrame::GraphStyle)graphStyle);

  /* Set Downloads/Uploads graph frame settings */
  frmGraph->setShowCounters(chkReceiveRate->isChecked(),
                               chkSendRate->isChecked());
                               
  cmbGraphStyle2->setCurrentIndex(graphStyle);
  frmGraph2->setGraphStyle((GraphFrame::GraphStyle)graphStyle);                             
                               
  /* set Incoming/Outgoing graph frame settings */
  frmGraph2->setShowCounters(chkReceiveRate2->isChecked(),
                               chkSendRate2->isChecked());                             
                               
}

/** 
 Resets the log start time
*/
void
Kommute::reset()
{
  /* Reset the graph */
  frmGraph->resetGraph();
}

void
Kommute::reset_received_emitted()
{
  /* Reset the graph */
  frmGraph2->resetGraph();
}

/**
 Saves the Bandwidth Graph settings and adjusts the graph if necessary
*/
void
Kommute::saveChanges()
{
  /* Hide the settings frame and reset toggle button */
  showSettingsFrame(false);
  showUploadsFrame(false);
  
  /* Save the graph style */
  //getsetting(SETTING_STYLE, cmbGraphStyle->currentIndex());
  
  /* Save the line filter values */
  //_settings->setBWGraphFilter(BWGRAPH_REC, ui.chkReceiveRate->isChecked());
  //_settings->setBWGraphFilter(BWGRAPH_SEND, ui.chkSendRate->isChecked());

  /* Update the graph frame settings */
  frmGraph->setShowCounters(chkReceiveRate->isChecked(),
                               chkSendRate->isChecked());
  frmGraph->setGraphStyle((GraphFrame::GraphStyle)cmbGraphStyle->currentIndex());                             
                               
  /* Update the graph frame settings */
  frmGraph2->setShowCounters(chkReceiveRate2->isChecked(),
                               chkSendRate2->isChecked());
                               
  frmGraph2->setGraphStyle((GraphFrame::GraphStyle)cmbGraphStyle2->currentIndex());                               

  /* A change in window flags causes the window to disappear, so make sure
   * it's still visible. */
  showNormal();
}

/** 
 Simply restores the previously saved settings
*/
void 
Kommute::cancelChanges()
{
  /* Hide the settings frame and reset toggle button */
  showSettingsFrame(false);
  showUploadsFrame(false);

  /* Reload the settings */
  //loadSettings();
}

/** 
 Toggles the Settings pane on and off, changes toggle button text
*/
void
Kommute::showSettingsFrame(bool show)
{
  if (show) {
    frmSettings->setVisible(true);
    btnToggleSettings->setChecked(true);
    btnToggleSettings->setText(tr("Hide Settings"));
  } else {
    frmSettings->setVisible(false);
    btnToggleSettings->setChecked(false);
    btnToggleSettings->setText(tr("Show Settings"));
  }
}

void
Kommute::showUploadsFrame(bool show)
{
  if (show) {
    frmSettings_2->setVisible(true);
    btnToggleSettings2->setChecked(true);
    btnToggleSettings2->setText(tr("Hide Settings"));
  } else {
    frmSettings_2->setVisible(false);
    btnToggleSettings2->setChecked(false);
    btnToggleSettings2->setText(tr("Show Settings"));
  }
}

/** Returns a formatted string with the correct size suffix. */
QString
Kommute::totalToString(qreal total)
{
  /* Determine the correct size suffix */
  if (total < 1024) {
    /* Use KB suffix */
    return tr("%1 KIB").arg(total, 0, 'f', 2);
  } else if (total < 1048576) {
    /* Use MB suffix */
    return tr("%1 MIB").arg(total/1024.0, 0, 'f', 2);
  } else {
    /* Use GB suffix */
    return tr("%1 GIB").arg(total/1048576.0, 0, 'f', 2);
  }
}

