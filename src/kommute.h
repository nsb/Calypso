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
#ifndef KOMMUTE_H
#define KOMMUTE_H

#include "ui_kommute.h"
#include "bwgraph/bwgraph.h"

#include <QSystemTrayIcon>
#include <QStandardItemModel>
#include <QEvent>
#include <QSortFilterProxyModel>
#include <QScrollArea>


class QAction;
class QEvent;
class DLListDelegate;
class SFListDelegate;
class ConnectionsListDelegate;

class Kommute : public QMainWindow, private Ui::MainWindow
{
    Q_OBJECT

public:
    Kommute(QWidget *parent = 0);
    ~Kommute();
    void customEvent(QEvent*);
    void hideEvent(QHideEvent*);
    void reloadList();
    //void initMultiview();
    void initConnectionsPage();
    void initDownloadsPage();
    void initSharedFilesPage();
    void loadStatistics();
    void saveStatistics();
    void initStatisticPage();
    void initPluginsPage();
    void handleDownloadRequest(const QString& url);
    inline bool stopping() {return stopRunning;};
    static bool checkIfAutorunWithWindows();
    static void updateAutorunRegistrySetting( bool flag );

protected:
    void timerEvent(QTimerEvent*);
    /** Called to deliver a bandwidth update event . */
    void gcustomEvent(QEvent *event);




signals:
    void connectionCountChanged(int);
    void currentConnectionAttempt( QString, int );
    void numberOfUploadsChanged( int );
    void visible(bool);

public slots:
    void downloadFileRequested( QString, unsigned long, QString, QString ,int,int,int,int);
    int addItem(const QString& symbol, const QString& name, qlonglong fileSize,
        double progress, const QString& priority, double dlspeed,
        const QString& sources, const QString& status, const QString& routequality,
        const QString& host, const QString& hash, const QString& ptr,
        qlonglong completed, qlonglong remaining);
    void delItem(int row);
    void editItem(int row, int column, QVariant data);
    void updateProgress(int value);

    double getProgress(int row, QStandardItemModel *model);
    double getSpeed(int row, QStandardItemModel *model);
    QString getFileName(int row, QStandardItemModel *model);
    QString getStatus(int row, QStandardItemModel *model);
    QString getRouteQuality(int row, QStandardItemModel *model);
    QString getHost(int row, QStandardItemModel *model);
    QString getHash(int row, QStandardItemModel *model);
    QString getPtr(int row, QStandardItemModel *model);
    qlonglong getFileSize(int row, QStandardItemModel *model);
    qlonglong getTransfered(int row, QStandardItemModel *model);
    qlonglong getRemainingTime(int row, QStandardItemModel *model);

    int getRowFromHash(QString hash) const;
    //QString getHashFromRow(unsigned int row) const;
    void showDetails(const QModelIndex &index);
    //void updateFileSizeAndProgress(QString hash);
    void detailsSelection();

    //SharedFilesPage:
    int addSFItem(QString sfsymbol, QString sfname, qlonglong sffileSize, QString sfhash);
    void delSFItem(int row);
    void editSFItem(int row, int column, QVariant data);
    QString getSFFileName(int row, QStandardItemModel *model);
    qlonglong getSFFileSize(int row, QStandardItemModel *model);
    QString getSFHash(int row, QStandardItemModel *model);

    void filterRegExpChanged();
    void clearfilter();
    void fileListRegExpChanged(QTreeWidgetItem * item, int column);
    void toggleclearButton();


    /** Called when this dialog is to be displayed */
    void show();
    // ConnectionsPage :
    int addCLItem(QString cicon, QString address, QString port, qlonglong sent, QString queued, QString dropped );
    void delCLItem(int row);
    void editCLItem(int row, int column, QVariant data);
    void setLogInfo(QString info, QColor color=QApplication::palette().color(QPalette::WindowText));

private slots:
    void newSearch(QString, QString );
    void closeTab();
    void closeAllTabs();
    void closeEvent( QCloseEvent *event );
    bool event( QEvent *event );
    void connectionQuality( int );
    void showSettingsGui();
    void toggleSearch();
    void toggleMultiview();
    void restoreKommute();
    void toggleVisibility(QSystemTrayIcon::ActivationReason e);
    void addFriendToSearchView(QString, unsigned long, QString, QString);
    void on_actionKommute_Handbook_activated();
    void on_action_About_Kommute_activated();
    void on_actionWhat_s_this_activated();
    void on_actionReport_Bug_activated();
    void on_actionConnections_activated();
    void on_actionTransfers_activated();
    void on_actionSearch_activated();
    void on_actionStatistics_activated();
    void on_actionSharedfiles_activated();
    void on_actionPlugins_activated();
    void on_actionColor_activated();
    void on_actionToolBar_icon_only_activated();
    void on_actionToolBar_Text_with_icon_activated();
    void quit();
    // ConnectionsPage :
    void showConnectDialog();
    void showConnectionAttempt( QString, int );
    void addHostcp();
    void on_actionClearLog_triggered();
    void displayInfoLogMenu(const QPoint& pos);
    void connectionstreeViewCostumPopupMenu( QPoint point );
    //StatisticsPage :
    /** Adds new data to the graph */
    void updateGraph(quint64 bytesRead, quint64 bytesWritten);
    /** Adds new data to the graph */
    void updateGraph2(quint64 bytesRead, quint64 bytesWritten);
    /** Called when settings button is toggled */
    void showSettingsFrame(bool show);
    /** Called when the settings button is toggled */
    void showUploadsFrame(bool show);
    /** Called when the user saves settings */
    void saveChanges();
    /** Called when the user cancels changes settings */
    void cancelChanges();
    /** Called when the reset button is pressed */
    void reset();
    void reset_received_emitted();
    // Called when double click on downloading file.
    void changeDownloadState(const QModelIndex& index);
    void changeEvent (QEvent* event);
    // set new painter widget.
    void setTaskGraphPainterWidget (const QModelIndex& index);
    void openFile (const QModelIndex & index);

private:
    void startFileSharing();
    void stopFileSharing();
    void stopSearches();
    void setupTrayIcon();
    void removeTrayIcon();

    void createTaskGraphPainterWidget();
    //QHBoxLayout *taskGraphDetailLayout;
    //QScrollArea *taskGraphWidget;

    void initConnectionList ();
    void initDownloadList ();
    void retranslateDownloadList ();
    void initSFList ();
    void initSFTreeWidget ();
    void retranslateSFList ();
    void retranslate ();

    class SearchView *searchView;
    class ConnectionsThread *connectionsThread;
    class UploadsThread *uploadsThread;
    class CloseSearchThreadQueue *searchThreadQueue;
    class CloseSearchThreadConsumer *searchThreadConsumer;
    class ConnectionQuality *connQuality;
    class NumberofConnections *numofConnections;
    class QSystemTrayIcon *trayIcon;

    // actions for systray icon
    class QAction *actionTrayExit;
    class QAction *actionTrayRestore;
    class QAction *actionTrayOptions;
    class QAction *actionTrayBandWidthGraph;


    bool stopRunning;
    // multiview elements
    //    class UploadsList *uploadsList;
    class QStandardItemModel *connectionsModel;

    QStandardItemModel *DLListModel;
    QItemSelectionModel *selection;
    DLListDelegate *DLDelegate;
    qlonglong fileSize;
    double progress;
    double dlspeed;
    QString status, icon, name, routequality, host, hash, ptr;
    qlonglong completed, remaining;

    // ShareFilesPage:
    QStandardItemModel *SFListModel;
    SFListDelegate *SFDelegate;
    qlonglong sffileSize;
    QString sfname, sfhash;
    QSortFilterProxyModel *proxyModel;


    // ConnectionsPage :
    QStandardItemModel *ConnectionsListModel;
    ConnectionsListDelegate *CLDelegate;

    /** Define the popup menus for the Context menu */
    QMenu* contextMnu;
    /** Defines the actions for the context menu */
    QAction* addhostAct;

    //StatisticsPage :
    /** Create and bind actions to events **/
    void createActions();
    /** Loads the saved Bandwidth Graph settings */
    void loadSettings();
    /** Returns a formatted string representation of total. */
    QString totalToString(qreal total);

    QMenu *trayMenu;

    /** A BandwidthGraph object which handles monitoring RetroShare bandwidth usage */
    BandwidthGraph* _bandwidthGraph;
};

#endif
