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

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <QtGui>
#include <QCloseEvent>
#include <QDockWidget>
#include <QTabWidget>
#include <QDesktopWidget>
#include <QPainter>
#include <QString>
#include <QtDebug>
#include <QIcon>
#include <QSystemTrayIcon>
#include <QMessageBox>
#include <QWhatsThis>
#include <QDesktopServices>
#include <QtGlobal>

#include "mutelayer.h"
#include "kommute.h"
#include "settings.h"
#include "searchview.h"
#include "connectionsthread.h"
#include "uploadsthread.h"
#include "searchthread.h"
#include "searchresultstabwidget.h"
#include "closesearchthreadqueue.h"
#include "closesearchthreadconsumer.h"
#include "connectionquality.h"
#include "searchresultsview.h"
#include "settingsgui.h"
#include "mutestopthread.h"
#include "fhelp.h"
#include "fabout.h"
#include "hashthread.h"
#include "kommuteversion.h"
#include "PluginsPage.h"
//#include "bwgraph/bwgraph.h"
#include "cache.h"
#include "KommuteLinkAnalyzer.h"
#include "webhostcachesdownload.h"

Kommute *pMainWin=NULL;
class Settings *settings;
HashThread *HT;
// webHostCaches object
WebHostCachesDownload *webHostCachesDownload=NULL;


Kommute::Kommute(QWidget *parent)
  : QMainWindow(parent)
{
    pMainWin=this;
    setCorner(Qt::TopLeftCorner, Qt::LeftDockWidgetArea);
    setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
    setCorner(Qt::TopRightCorner, Qt::RightDockWidgetArea);
    setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);

//     setAttribute(Qt::WA_DeleteOnClose);

    setupUi(this);
    searchTab->removeTab(0);
    //actionConnections->setCheckable(true);
    //actionConnections->setChecked(true);

    this->setWindowIcon(QIcon(QString::fromUtf8(":/kommute_logo/hi-16-app-kommute.png")));

    actionExit->setShortcut(tr("Ctrl+Q"));
    actionExit->setStatusTip(tr("Exit the application"));
    actionExit->setIcon(QIcon(":/resources/exit.png"));
    connect(actionExit, SIGNAL(triggered()), this, SLOT(quit()));

    actionClose->setShortcut(tr("Ctrl+W"));
    actionClose->setStatusTip(tr("Close the current tab"));
    actionClose->setEnabled(false);
    actionClose->setIcon(QIcon(":/resources/tab_remove.png"));
    connect(actionClose, SIGNAL(triggered()), this, SLOT(closeTab()));

    //Search Results View
    closetabButton->setEnabled(false);
    connect(closetabButton, SIGNAL(clicked()), this, SLOT(closeAllTabs()));

    downloadButton->setEnabled(false);

    //Set Icons for the menuHelp/menuSettings

    actionKommute_Handbook->setIcon(QIcon(":/resources/address-book-new.png"));
    actionWhat_s_this->setIcon(QIcon(":/resources/help-browser.png"));
    actionReport_Bug->setIcon(QIcon(":/resources/bug.png"));

    // this will show the setupwizard if it's needed
    settings = new Settings ();
    settings->loadSettings ();

    createTaskGraphPainterWidget();

    /*taskGraphDetailLayout = new QHBoxLayout();
    taskGraphDetailLayout->addWidget(taskGraphWidget);
    tab_2->setLayout(taskGraphDetailLayout);
    taskGraphDetailLayout->setSpacing( 0 );
    taskGraphDetailLayout->setMargin( 0 );*/

    initDownloadsPage();
    initConnectionsPage();
    initStatisticPage();
    initSharedFilesPage();
    
    loadStatistics();
    
    // Create the searchview
    searchView = new SearchView(searchViewDock);
    searchViewDock->setWidget(searchView);
    connect(searchView, SIGNAL(startSearch(QString, QString )), this, SLOT(newSearch(QString, QString )));
//    connect(searchView, SIGNAL(visible(bool)), searchViewDockAction, SLOT(setChecked(bool)));



    // Because stopping a searchThread takes a long time, we put finished searchThreads
    // in a searchThreadQueue, and creates a background thread that deletes them when
    // they are finished
    searchThreadQueue = new CloseSearchThreadQueue();
    searchThreadConsumer = new CloseSearchThreadConsumer( searchThreadQueue );

    // Create the status bar
    statusBar()->showMessage( "" );

    // Connection quality monitor
    connQuality = new ConnectionQuality();
    connect(this, SIGNAL(connectionCountChanged(int )),
                         this, SLOT(connectionQuality(int )));

    //statusBar()->addPermanentWidget(new QLabel(tr("")));
    statusBar()->addPermanentWidget(connQuality);

    // show current connection attempt
    connect(this, SIGNAL(currentConnectionAttempt(QString, int )),
                         this, SLOT(showConnectionAttempt(QString, int )));

    /* Create all the dialogs of which we only want one instance */
    _bandwidthGraph = new BandwidthGraph();

    // start Settings Gui if needed
    connect(action_Configure_Kommute, SIGNAL(triggered()), this, SLOT(showSettingsGui()));
    connect(searchViewDockAction, SIGNAL(triggered()), this, SLOT(toggleSearch()));

    // show the tray Icon
    setupTrayIcon();

    // Create the download for downloading of the WebHostCaches
    webHostCachesDownload = new WebHostCachesDownload( settings->getWebHostCachesSetting(), settings->getDownloadWebHostCachesSetting() );
    // if we want to reload the WebHostCachesList on every start
    if ( settings->getEnableWebHostCache() || settings->getReloadWebHostCachesSetting() )
    {
      webHostCachesDownload->start();
/*
      while ( webHostCachesDownload->isRunning() )
      {
        qApp->processEvents();
      }
      settings->setWebHostCachesSetting( webHostCachesDownload->getWebHostCaches() );
*/
    }

    // Start MUTE filesharing
    startFileSharing();
    reloadList();
    startCache();
    char *virtualAddress = MuteLayer::getLocalVirtualAddress();
    setWindowTitle(QString("Kommute %1 - ").arg(kommuteVersion())+virtualAddress+"["+MuteLayer::getMnemonic( virtualAddress )+"]");
    delete [] virtualAddress;

    // we don't plan to stop the mute core now
    stopRunning = false;
    // start a hash thread:
    HT=new HashThread(this);
    HT->start();
}

Kommute::~Kommute()
{
    delete HT;
    delete settings;
    delete trayMenu;
    delete DLListModel;
    delete searchThreadQueue;
    delete searchView;
    delete _bandwidthGraph;
}

#define AUTORUN_WIN_KEY "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run"
#define KOMMUTE_KEY "kommute.exe"


bool Kommute:: checkIfAutorunWithWindows()
{

	if ( settings )
		return settings->getChkAutorunWithWin();
	return false;
}

void Kommute::updateAutorunRegistrySetting ( bool flag )
{
	QSettings autoKey( AUTORUN_WIN_KEY, QSettings::NativeFormat);
	QString kKey = autoKey.value(KOMMUTE_KEY).toString();
	QString CurrentPath = QDir::currentPath()+QDir::separator()+KOMMUTE_KEY;

	QMessageBox msb;

	if ( !flag )
	{

		if((!kKey.isEmpty())) { autoKey.remove(KOMMUTE_KEY);}

	}
	else
	{
		if((kKey.isEmpty())||( kKey != CurrentPath )) autoKey.setValue (KOMMUTE_KEY,CurrentPath);

	}


}



void Kommute::handleDownloadRequest(const QString& url){

    KommuteLinkAnalyzer analyzer (url);

    if (!analyzer.isValid ())
        return;

    QVector<KommuteLinkData> linkList;
    analyzer.getFileInformation (linkList);

    for (int i = 0, n = linkList.size (); i < n; ++i)
    {
        const KommuteLinkData& linkData = linkList[i];
        downloadFileRequested(linkData.getName (), linkData.getSize ().toInt (),
            linkData.getHash (), "", -1, -1, -1, -1);
    }
}

void Kommute::quit()
{
    // show that we want to stop
    stopRunning = true;
    // show exiting message and wait 5 seconds
    //statusBar()->showMessage(tr("Exiting application..."), 5000);
    /*
        5 seconds causes an error at my box when kde thought the app hang and want to kill it
        Even if you deny this, the app won't close anymore
    */
    // show exiting message
    statusBar()->showMessage(tr("Exiting application..."));
    // suppress searches
    while(searchTab->count()  > 0)
    {
	QWidget * oldWidget= searchTab->widget(0);
	searchTab->removeTab(0);
	delete oldWidget;
    }
    // stop web host cache
    webHostCachesDownload->stop();
    // stop HashThread
    HT->stop();
    // we are going to exit already, so we can't exit again
    actionTrayExit->setEnabled(false);
    actionExit->setEnabled(false);
    // stop the mute core
    stopCache();
    stopFileSharing();
    // save cumulative statistics
    saveStatistics();
    // stop the mute core
    MuteStopThread *muteStopThread = new MuteStopThread();
    // stop web host cache download
    delete webHostCachesDownload;
    // wait till the mute core isn't running anymore. wait 15 seconds max.
    QTime time0;
    time0.start();
    while ( muteStopThread->isRunning() && time0.elapsed() <15000)
    {
        qApp->processEvents();
        //Debug Message here
    }
    delete muteStopThread;
    // remove the tray icon
    removeTrayIcon();
    close();
    qApp->quit();
}

void Kommute::setupTrayIcon()
{
    /**
    @todo: Why doesn't the resource path work here? :/resources/hi24-app-kommute.png
    */
    // create the pixmap
    QPixmap pixmap = QPixmap(":/kommute_logo/hi-16-app-kommute.png");
    // create the menu
    trayMenu = new QMenu( QCoreApplication::instance()->applicationName() );
    // add a horizontal line to the menu
    trayMenu->addSeparator();
    // add the BandWidth Graph menu entry
    //actionTrayBandWidthGraph = trayMenu->addAction( tr("Bandwidth Graph"));
    //actionTrayBandWidthGraph->setIcon(QIcon(":/resources/graph.png"));
    //connect( actionTrayBandWidthGraph, SIGNAL( triggered() ), _bandwidthGraph, SLOT(showWindow()));
	// add the Settings menu entry
    actionTrayOptions = trayMenu->addAction( tr("Options"));
    actionTrayOptions->setIcon(QIcon(":/resources/misc16.png"));
    connect( actionTrayOptions, SIGNAL( triggered() ), this, SLOT( showSettingsGui() ));
    // add a horizontal line to the menu
    trayMenu->addSeparator();
    // add actions to the menu
    actionTrayRestore = trayMenu->addAction( tr("Restore from Tray"));
    actionTrayRestore->setIcon(QIcon(":/resources/restorewindow.png"));
    // if kommute is visible, disable the actionTrayRestore
    // until a valid method is implemented here, set to false as standard
    actionTrayRestore->setEnabled( false );
    connect( actionTrayRestore, SIGNAL( triggered() ), this, SLOT( restoreKommute() ));
    // add a horizontal line to the menu
    trayMenu->addSeparator();
    // add the close menu entry
    actionTrayExit = trayMenu->addAction( tr("Quit Kommute"));
    actionTrayExit->setIcon(QIcon(":/resources/exit2.png"));
    connect( actionTrayExit, SIGNAL( triggered() ), this, SLOT( quit() ));
    // build the trayicon
    //trayIcon = new QSystemTrayIcon( pixmap, QCoreApplication::instance()->applicationName(), menu, this );
    trayIcon = new QSystemTrayIcon( pixmap, this );
    trayIcon->setContextMenu( trayMenu );
    trayIcon->setToolTip(QCoreApplication::instance()->applicationName());
    //trayrestore with mouse click
    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this,
            SLOT(toggleVisibility(QSystemTrayIcon::ActivationReason)));
    // show the icon
    trayIcon->show();
}

void Kommute::restoreKommute()
{
    // disable the actionTrayRestore, because the window is restored already
    actionTrayRestore->setEnabled( false );
    // give the window the titlebar back, so it's visible in the taskbar
    setWindowFlags(Qt::Window);
    // and show the window like it was before
    setWindowState(windowState() & ~Qt::WindowMinimized | Qt::WindowActive);
    show();
}

void Kommute::toggleVisibility(QSystemTrayIcon::ActivationReason e)
{
    if(e == QSystemTrayIcon::Trigger || e == QSystemTrayIcon::DoubleClick){
        if(isHidden()){
             // disable the actionTrayRestore, because the window is restored already
			actionTrayRestore->setEnabled( false );
            setWindowFlags(Qt::Window);
            setWindowState(windowState() & ~Qt::WindowMinimized | Qt::WindowActive);
            show();

            if(isMinimized()){
                if(isMaximized()){
                    showMaximized();
                }else{
                    showNormal();
                }
            }
            raise();
            activateWindow();
	  extern bool newRelease;
	  if(newRelease)
          {
	    QMessageBox::information(NULL,tr("New Release !")
		,tr("A new Release is available.\nYou should go to http://sourceforge.net/projects/kommute/files and download the last release."));
	    newRelease=false;
          }
        }else{
            hide();
        }
    }
}

void Kommute::show()
{

    if (!this->isVisible()) {
        QMainWindow::show();
    } else {
        QMainWindow::activateWindow();
        setWindowState(windowState() & ~Qt::WindowMinimized | Qt::WindowActive);
        QMainWindow::raise();
    }
}


void Kommute::removeTrayIcon()
{
    // remove the icon
    trayIcon->hide();
}

void Kommute::startFileSharing()
{


    // Seed for the random generator
    QString seed = settings->getSeedSetting();
    // dir were the settings for mute should be
    QString settingsDir = settings->getSettingsDir();
    // port mute should listen
    int portNumber = settings->getPortNumberSetting();

    // TODO: It should be possible to specify portnumber as commandline arg.
    // from feivelda (Andy): I think it isn't necessary anymore to set the portnumber as command line argument
    // it's set in settingsGui already


     if ( MuteLayer::muteLayerStart(seed, settingsDir, portNumber)) {
        //Debug Message here
     }

     // Start the connections monitoring thread


     connectionsThread = new ConnectionsThread (this);
     int value = settings->getMaxConnectionCountSetting();
     MuteLayer::muteLayerSetMaxNumberOfConnections(value);
     value = settings->getTargetConnectionCountSetting();
     MuteLayer::muteLayerSetTargetNumberOfConnections(value);

     connectionsThread->start();

    // Start the uploads monitoring thread
    uploadsThread = new UploadsThread(this);
    uploadsThread->start();
}

void Kommute::stopFileSharing()
{
    // Stop all active searches
     stopSearches();

    // Stop all active downloads
    downloadsTreeView->stopAll();

    // Stop the uploads monitoring thread
     uploadsThread->stop();
     delete uploadsThread;

    // Stop the connections monitoring thread

    connectionsThread->stop();
    delete connectionsThread;

    searchThreadConsumer->stop();
    delete searchThreadConsumer;


}

void Kommute::newSearch( QString address, QString searchTerms )
{
    // Start a new search and add a new searchTab to the searchView

    // If we a searching in a specific host, prepend the host name to searchtab caption
    QString tabText;
    if( QString::compare( address, "ALL" ) != 0 )
        tabText = QString("[%1] %2")
                          .arg( MuteLayer::getMnemonic( address ) )
                          .arg( searchTerms );
    else
        tabText = searchTerms;

    SearchResultsView *srView = new SearchResultsView(0, address, searchTerms, searchThreadQueue);

    searchTab->addTab( srView, searchTerms );

    connect(srView, SIGNAL(numberOfSearchResultsChanged(QWidget*, int, int)),
            searchTab, SLOT(updateNumSearchResults(QWidget*, int, int )));
    searchTab->setCurrentWidget( srView );
    searchTab->setTabIcon(searchTab->currentIndex(), QIcon(":/resources/cancel.png") );
    SearchThread *sThread = new SearchThread( srView , address, searchTerms );
    srView->attachSearchThread( sThread );
    connect(srView, SIGNAL(downloadFileRequested(QString, unsigned long, QString, QString,int,int,int,int)),
        this, SLOT(downloadFileRequested(QString, unsigned long, QString, QString,int,int,int,int)));
    connect(srView, SIGNAL(downloadFileRequested(QString, unsigned long, QString, QString ,int,int,int,int)),
        this, SLOT(addFriendToSearchView( QString, unsigned long, QString, QString )));
    connect(downloadButton, SIGNAL(clicked()), srView, SLOT(downloadSelectedItems()));

    connect(sThread, SIGNAL(started()), srView, SLOT(searchStarted()));
    connect(sThread, SIGNAL(finished()), srView, SLOT(searchStopped()));
    connect(sThread, SIGNAL(terminated()), srView, SLOT(searchStopped()));
    connect(srView, SIGNAL(grabFriendRequested(QString, unsigned long, QString, QString )),
        this, SLOT(addFriendToSearchView( QString, unsigned long, QString, QString )));
    sThread->start();
    on_actionSearch_activated();
    actionClose->setEnabled(true);
    closetabButton->setEnabled(true);
    downloadButton->setEnabled(true);

}

void Kommute::addFriendToSearchView( QString name, unsigned long size, QString hash, QString host )
{
    searchView->addFriend( host );
}

void Kommute::stopSearches()
{
    // TODO: implement this
}

void Kommute::connectionQuality( int connections )
{
    connQuality->setQuality( connections );
}

/*void Kommute::showConnectionAttempt( QString attemptAddress, int attemptPort )
{
    statusBar()->showMessage( "Trying to connect to " + attemptAddress + ":" + QString::number( attemptPort ) + "...", 5000 );
}*/

void Kommute::closeTab()
{
    QWidget *w = searchTab->widget(searchTab->currentIndex());
    searchTab->removeTab(searchTab->currentIndex());
    if( searchTab->count() == 0 )
    {
        actionClose->setEnabled(false);
        closetabButton->setEnabled(false);
    }
    delete w;
}

//Close all search tabs actived
void Kommute::closeAllTabs()
{
    searchTab->closeAllTabs();
    actionClose->setEnabled(false);
    closetabButton->setEnabled(false);
    downloadButton->setEnabled(false);
}

/**
    method quit was replaced by closeEvent because closeEvent is called every time, not only when
    clicked to quit. the quit function wasn't called when kommute was closed by the X
*/
void Kommute::closeEvent( QCloseEvent *event )
{
    // if we are stopping already, don't do anything
    if ( stopRunning )
    {
        event->ignore();
        return;
    }
    else
    {
        // Compute size and position of the pixmap to be grabbed:
        QRect g = trayIcon->geometry();
        int desktopWidth  = qApp->desktop()->width();
        int desktopHeight = qApp->desktop()->height();
        int tw = g.width();
        int th = g.height();
        int w = desktopWidth / 4;
        int h = desktopHeight / 9;
        int x = g.x() + tw/2 - w/2;               // Center the rectange in the systray icon
        int y = g.y() + th/2 - h/2;
        if ( x < 0 )                 x = 0;       // Move the rectangle to stay in the desktop limits
        if ( y < 0 )                 y = 0;
        if ( x + w > desktopWidth )  x = desktopWidth - w;
        if ( y + h > desktopHeight ) y = desktopHeight - h;

        // Grab the desktop and draw a circle arround the icon:
        QPixmap shot = QPixmap::grabWindow( QApplication::desktop()->winId(),  x,  y,  w,  h );
        QPainter painter( &shot );
        const int MARGINS = 6;
        const int WIDTH   = 3;
        int ax = g.x() - x - MARGINS -1;
        int ay = g.y() - y - MARGINS -1;
        painter.setPen(  QPen( Qt::red,  WIDTH ) );
        painter.drawArc( ax,  ay,  tw + 2*MARGINS,  th + 2*MARGINS,  0,  16*360 );
        painter.end();

        // Associate source to image and show the dialog:
//        QMimeSourceFactory::defaultFactory()->setPixmap( "systray_shot",  shot );
        //QMessageBox(QMessageBox::information,
	QMessageBox msgBox( QMessageBox::Information,
	    tr( "Docking in System Tray" ),
            tr( "<p>Closing the main window will keep Kommute running in the system tray. "
            "Use <b>Quit</b> from the <b>Kommute</b> menu to quit the application.</p>"),
            QMessageBox::Ok,this  );
	msgBox.setIconPixmap(shot);
	msgBox.exec();
        hide();
        event->ignore();
    }
}

void Kommute::hideEvent( QHideEvent *event )
{
    // if the window is hidden, we must be able to open it again
    actionTrayRestore->setEnabled( true );
    event->accept();
}

bool Kommute::event(QEvent *event)
{
    // if the event is WindowStateChange and
    if( QEvent::WindowStateChange == event->type() )
    {
        // the event is minimized
        if ( event->spontaneous() && isMinimized() )
        {
            // we should hide the window, so it's minimized
            hide();
            // and remove the WindowTitle. So it's also removed from taskbar
            setWindowFlags(Qt::FramelessWindowHint);
            // the event was consumed
            return true;
        }
    }
    // the event wasn't consomued, so give it back
    QWidget::event(event);
    return false;
}

void Kommute::showSettingsGui()
{
    // create a settingsGui class
    SettingsGui *settingsGui = new SettingsGui();
    // show it modal
    settingsGui->exec();
    settings->loadSettings();


}

void Kommute::toggleSearch()
{
	searchViewDock->setVisible(!searchViewDock->isVisible());
}

void Kommute::on_actionConnections_activated()
{
	stackPages->setCurrentWidget(page_Connections);
	/*actionConnections->setCheckable(true);
	actionConnections->setChecked(true);

    actionTransfers->setChecked(false);
	actionSearch->setChecked(false);
    actionStatistics->setChecked(false);
    actionSharedfiles->setChecked(false);*/
}

void Kommute::on_actionTransfers_activated()
{
    if(layoutWidget->parentWidget() != splitter)
    {
	//layoutWidget2->setParent(NULL);
	layoutWidget->setParent(splitter);
	//layoutWidget2->setParent(splitter);
    }
	stackPages->setCurrentWidget(page_Transfers);
	/*actionTransfers->setCheckable(true);
	actionTransfers->setChecked(true);

	actionConnections->setChecked(false);
	actionSearch->setChecked(false);
    actionStatistics->setChecked(false);
    actionSharedfiles->setChecked(false);*/
}

void Kommute::on_actionSearch_activated()
{
    frame_x->setParent(NULL);
    if(settings->getSearchWithDownloads())
    {
	QLayout *ql=page_Search->layout();
	ql->addWidget(layoutWidget);
	layoutWidget->setParent(splitter_search);
    }
    else
    if(layoutWidget->parentWidget() != splitter)
    {
	layoutWidget->setParent(NULL);
    }
    stackPages->setCurrentWidget(page_Search);
    /*actionSearch->setCheckable(true);
    actionSearch->setChecked(true);

    actionTransfers->setChecked(false);
    actionConnections->setChecked(false);
    actionStatistics->setChecked(false);
    actionSharedfiles->setChecked(false);*/
}

void Kommute::on_actionStatistics_activated()
{
	stackPages->setCurrentWidget(page_Stats);
	/*actionStatistics->setCheckable(true);
	actionStatistics->setChecked(true);

	actionTransfers->setChecked(false);
	actionSearch->setChecked(false);
	actionConnections->setChecked(false);
	actionSharedfiles->setChecked(false);*/
}

void Kommute::on_actionSharedfiles_activated()
{
	stackPages->setCurrentWidget(page_Sharedfiles);
	/*actionSharedfiles->setCheckable(true);
	actionSharedfiles->setChecked(true);

	actionTransfers->setChecked(false);
	actionSearch->setChecked(false);
	actionConnections->setChecked(false);
	actionStatistics->setChecked(false);*/
}

void Kommute::on_actionPlugins_activated()
{
	stackPages->setCurrentWidget(page_Plugins);
	/*actionSharedfiles->setCheckable(true);
	actionSharedfiles->setChecked(true);

	actionTransfers->setChecked(false);
	actionSearch->setChecked(false);
	actionConnections->setChecked(false);
	actionStatistics->setChecked(false);*/
}

void Kommute::toggleMultiview()
{
}

void Kommute::on_actionReport_Bug_activated()
{
	QDesktopServices::openUrl (QUrl("http://sourceforge.net/tracker/?func=add&group_id=146890&atid=766694"));
}

void Kommute::on_actionWhat_s_this_activated()
{
	QWhatsThis::enterWhatsThisMode () ;
}

void Kommute::on_actionKommute_Handbook_activated()
{
 FHelp *fh;
 QString nomfic("index.html");
 QString path("/usr/share/doc/kommute");
 QStringList paths;
 char cfic[500];
 struct stat buf;
 QString locale = QLocale::system().name();
 char loc[20];
  strcpy(loc, locale.toLocal8Bit().data());
  sprintf(cfic,"/usr/share/doc/kommute/index_%s.html",loc);
  if ( ! stat(cfic, &buf))
  {
     sprintf(cfic,"index_%s.html",locale.toLocal8Bit().data());
      nomfic=cfic;
  }
  else
  {
   char * finloc=strchr(loc,'_');
    if (finloc) finloc[0]=0;
    sprintf(cfic,"/usr/share/doc/kommute/index_%s.html",loc);
    if ( ! stat(cfic, &buf))
    {
      sprintf(cfic,"index_%s.html",loc);
      nomfic=cfic;
    }
    else
    {
      sprintf(cfic,"doc/index_%s.html",loc);
      if ( ! stat(cfic, &buf))
      {
        sprintf(cfic,"index_%s.html",loc);
        nomfic=cfic;
      }
      else
      {
        nomfic="index.html";
      }
    }
  }
  fh=new FHelp;
  paths << "." <<"doc" <<"/usr/share/doc/kommute";
  fh->ui.TBHelp->setSearchPaths(paths);
  fh->ui.TBHelp->setSource(nomfic);
  fh->showMaximized();
}

void Kommute::on_action_About_Kommute_activated()
{
 FAbout f2;
 QStringList paths;
  paths << "." <<"./doc" <<"/usr/share/doc/kommute";
  f2.TB_GPL->setSearchPaths(paths);
  f2.TB_GPL->setSource(QString("common/gpl-license.html"));
  f2.exec();
}

void Kommute::on_actionColor_activated()
{
 QString standardSheet = "{background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1,stop:0 <color1>, stop:1 <color2>);}";
 QString actualSheet = qApp->styleSheet();
 QColor stop1 = QColorDialog::getColor(Qt::white);
 if (stop1.isValid())
 {
   QColor stop2 = QColorDialog::getColor(Qt::black);
   if (stop2.isValid())
   {
     actualSheet.replace(QRegExp(".QWidget[^\\{]*\\{[^\\}]*\\}"),"");
     actualSheet.replace(QRegExp("QToolBar[^\\{]*\\{[^\\}]*\\}"),"");
     actualSheet.replace(QRegExp("QMenuBar[^\\{]*\\{[^\\}]*\\}"),"");
     actualSheet.replace(QRegExp("QStatusBar[^\\{]*\\{[^\\}]*\\}"),"");
     QString widgetSheet = ".QWidget" + standardSheet.replace("<color1>", stop1.name()).replace("<color2>", stop2.name());
     QString toolSheet = "QToolBar" + standardSheet.replace("<color1>", stop1.name()).replace("<color2>", stop2.name());
     QString menuSheet = "QMenuBar" + standardSheet.replace("<color1>", stop1.name()).replace("<color2>", stop2.name());
     QString statusbarSheet = "QStatusBar" + standardSheet.replace("<color1>", stop1.name()).replace("<color2>", stop2.name());
     QString newSheet=actualSheet + widgetSheet + toolSheet + menuSheet + statusbarSheet;
     qApp->setStyleSheet( newSheet );

    QFile file( settings->getSettingsDir() + "custom.qss");
    file.open(QFile::WriteOnly);
    file.write( newSheet.toLatin1());
    QString styleSheet = QLatin1String(file.readAll());

    settings->setSheetName( "custom");
    settings->saveSettings();
   }
 }
}

void Kommute::createTaskGraphPainterWidget()
{
    //taskGraphWidget = new QScrollArea();
    taskGraphWidget->setWidgetResizable(true);
    taskGraphWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    taskGraphWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    taskGraphWidget->viewport()->setBackgroundRole(QPalette::NoRole);
    taskGraphWidget->setFrameStyle(QFrame::NoFrame);
    taskGraphWidget->setFocusPolicy(Qt::NoFocus);
}

void Kommute::on_actionToolBar_icon_only_activated()
{
  toolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);
}

void Kommute::on_actionToolBar_Text_with_icon_activated()
{
  toolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
}

void Kommute::changeEvent (QEvent* event)
{
    if (event->type () == QEvent::LanguageChange)
    {
        // apply the new language
        // all designed forms have retranslateUi() method
        retranslateUi (this);

        // everything related to translation not included in ui.
        retranslate ();
    }

    QWidget::changeEvent (event);
}

void Kommute::retranslate ()
{
    initConnectionList ();

    initDownloadList ();
    retranslateDownloadList();

    initSFList ();
    retranslateSFList ();

    // connection quality.
    connQuality->retranslate ();

    // system tray.
    actionTrayOptions->setText (tr ("Options"));
    actionTrayRestore->setText (tr("Restore from Tray"));
    actionTrayExit->setText (tr("Quit Kommute"));
}

