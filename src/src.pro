
TEMPLATE = app
TARGET = kommute
CONFIG += qt gui uic qrc uitools
CONFIG += release
RESOURCES = resources.qrc
QT += network xml script opengl

FORMS += searchresultsview.ui \
        kommute.ui \
        searchview.ui \
        setupwizard.ui \
        settingsgui.ui \
        listdialog.ui \
        fhelp.ui \
        fabout.ui \
        connect/ConnectDialog.ui \
        properities/CommentsDialog.ui \
        properities/DetailsDialog.ui \
        bwgraph/bwgraph.ui

SOURCES += main.cpp \
           kommute.cpp \
           searchview.cpp\
           mutelayer.cpp \
           ConnectionsPage.cpp \
           connectionsthread.cpp \
           addlogevent.cpp \
           ConnectionsListDelegate.cpp \
           connectioncountevent.cpp \
           connectionattemptevent.cpp \
           connectionevent.cpp \
           connectionquality.cpp \
           closesearchthreadqueue.cpp \
           closesearchthreadconsumer.cpp \
           searchthread.cpp \
           searchresultevent.cpp \
           searchresultstabwidget.cpp \
           searchresultsview.cpp \
           searchresultstable.cpp \
           searchresultfilter.cpp \
           searchresultfiltermodel.cpp \
           downloadthread.cpp \
           download.cpp \
           Downloaddefs.cpp \
           downloadstatusevent.cpp \
           downloadfileevent.cpp \
           downloadstable.cpp \
           DLListDelegate.cpp \
           DownloadsPage.cpp \
           KommuteApplication.cpp \
           kommutecustomevent.cpp \
           Kommutedefs.cpp \
           kommuteutils.cpp \
           kommuteversion.cpp \
           KommuteLinkAnalyzer.cpp \
           keygenthread.cpp \
           webhostcachesdownload.cpp \
           settingsgui.cpp \
           settingshelper.cpp \
           listdialog.cpp \
           mutestopthread.cpp \
           fhelp.cpp \
           fabout.cpp \
           hashthread.cpp \
           graphframe.cpp \
           PluginsPage.cpp \
           PluginManager.cpp \
           PluginManagerWidget.cpp \
           StatisticPage.cpp \
           SharedFilesPage.cpp \
           sharedfileslist.cpp \
           sharedfilesevent.cpp \
           settings.cpp \
           setupwizard.cpp \
           taskGraphPainterWidget.cpp \
           SFListDelegate.cpp \
           uploadsthread.cpp \
           uploadscountevent.cpp \
           uploadevent.cpp \
           uploadlistitem.cpp \
           uploadslist.cpp \
           uploaddefs.cpp \
           Utils.cpp \
           xprogressbar.cpp \
           connect/ConnectDialog.cpp \
           properities/CommentsDialog.cpp \
           properities/DetailsDialog.cpp \
           bwgraph/bwgraph.cpp \
           ksettings.cpp \
           kwindow.cpp \
	   cache.cpp \
	   MUTE/fileShare.cpp \
	   MUTE/ChannelReceivingThread.cpp MUTE/ChannelReceivingThreadManager.cpp \
	   MUTE/ConnectionMaintainer.cpp MUTE/LocalAddressReceiver.cpp \
	   MUTE/MessageIDTracker.cpp MUTE/messageRouter.cpp \
	   MUTE/OutboundChannelManager.cpp MUTE/ServerThread.cpp \
	   MUTE/StopSignalThread.cpp MUTE/FinishedSignalThread.cpp \
	   MUTE/sha1.cpp MUTE/MimeTyper.cpp \
	   minorGems/util/SettingsManager.cpp \
	   minorGems/network/NetworkFunctionLocks.cpp \
	   minorGems/network/p2pParts/OutboundChannel.cpp \
	   minorGems/util/printUtils.cpp \
	   minorGems/util/stringUtils.cpp \
	   minorGems/util/StringBufferOutputStream.cpp \
	   minorGems/network/SocketManager.cpp \
	   MUTE/common/CryptoUtils.cpp \
	   minorGems/network/p2pParts/HostCatcher.cpp \
	   minorGems/network/web/URLUtils.cpp \
	   minorGems/network/p2pParts/protocolUtils.cpp \
	   MUTE/layers/secureStream/SecureStreamFactory.cpp \
	   minorGems/network/web/WebClient.cpp \
	   MUTE/common/AESEncryptor.cpp MUTE/common/AESDecryptor.cpp \
	   MUTE/layers/secureStream/SecureOutputStream.cpp MUTE/layers/secureStream/SecureInputStream.cpp minorGems/formats/encodingUtils.cpp \
	   MUTE/kblimiter.cpp  applog.cpp


win32{
SOURCES += minorGems/network/win32/HostAddressWin32.cpp
SOURCES += minorGems/system/win32/MutexLockWin32.cpp
SOURCES += minorGems/system/win32/TimeWin32.cpp
SOURCES += minorGems/system/win32/BinarySemaphoreWin32.cpp
SOURCES += minorGems/system/win32/ThreadWin32.cpp
SOURCES += minorGems/network/win32/SocketWin32.cpp
SOURCES += minorGems/network/win32/SocketServerWin32.cpp
SOURCES += minorGems/network/win32/SocketClientWin32.cpp
SOURCES += minorGems/io/file/win32/PathWin32.cpp
SOURCES += minorGems/io/win32/TypeIOWin32.cpp
	RC_FILE = resources/kommute_win.rc
    LIBS += -lwsock32
LIBS += MUTE/crypto/Release/libcrypto.a
    DEFINES += WIN_32
}
unix{
SOURCES += minorGems/network/linux/HostAddressLinux.cpp
SOURCES += minorGems/system/unix/TimeUnix.cpp
SOURCES += minorGems/system/linux/BinarySemaphoreLinux.cpp
SOURCES += minorGems/system/linux/ThreadLinux.cpp
SOURCES += minorGems/network/linux/SocketLinux.cpp
SOURCES += minorGems/network/linux/SocketServerLinux.cpp
SOURCES += minorGems/network/linux/SocketClientLinux.cpp
SOURCES += minorGems/io/file/linux/PathLinux.cpp
SOURCES += minorGems/io/linux/TypeIOLinux.cpp
QMAKE_LFLAGS += -g
QMAKE_CFLAGS = 
QMAKE_CXXFLAGS = 
LIBS += MUTE/crypto/libcrypto.a
}

HEADERS += kommute.h \
           applog.h \
           mutelayer.h \
           addlogevent.h \
           cache.h \
           connectioncountevent.h \
           connectionattemptevent.h \
           connectionevent.h \
           connectionquality.h \
           ConnectionsListDelegate.h \
           connectionsthread.h \
           closesearchthreadqueue.h \
           closesearchthreadconsumer.h \
           downloadthread.h \
           download.h \
		   Downloaddefs.h \
           downloadstatusevent.h \
           downloadfileevent.h \
           downloadstable.h \
           DLListDelegate.h \
           fhelp.h \
           fabout.h \
           graphframe.h \
           hashthread.h \
           KommuteApplication.h \
		   Kommutedefs.h \
           kommuteutils.h \
           kommuteversion.h \
		   KommuteLinkAnalyzer.h \
           keygenthread.h \
           ksettings.h \
           kwindow.h \
           listdialog.h \
           linetypes.h \
           misc.h \
           mutestopthread.h \
           searchthread.h \
           searchview.h \
           searchresultevent.h \
           searchresultstabwidget.h \
           searchresultsview.h \
           searchresultstable.h \
           searchresultfilter.h \
           searchresultfiltermodel.h \
           SearchListDelegate.h \
           settings.h \
           settingsgui.h \
           settingshelper.h \
           setupwizard.h \
           sharedfileslist.h \
           sharedfilesevent.h \
           SFListDelegate.h \
           taskGraphPainterWidget.h \
           uploadsthread.h \
           uploadscountevent.h \
           uploadevent.h \
           uploadlistitem.h \
           uploadslist.h \
           uploaddefs.h \
           Utils.h \
           PluginsPage.h \
           PluginManager.h \
           PluginManagerWidget.h \
           webhostcachesdownload.h \
           xprogressbar.h \
           connect/ConnectDialog.h \
           resources/kommute_win.rc.h \
           properities/CommentsDialog.h \
           properities/DetailsDialog.h \
           bwgraph/bwgraph.h \
	   MUTE/fileShare.h MUTE/StopSignalThread.h MUTE/FinishedSignalThread.h \
	   MUTE/sha1.h MUTE/MimeTyper.h \
	   MUTE/ChannelReceivingThread.h MUTE/ChannelReceivingThreadManager.h \
	   MUTE/ConnectionMaintainer.h MUTE/LocalAddressReceiver.h \
	   MUTE/MessageIDTracker.h MUTE/messageRouter.h \
	   MUTE/OutboundChannelManager.h MUTE/ServerThread.h \
	   minorGems/util/SettingsManager.h \
	   MUTE/kblimiter.h \
	   plugins/PluginInterface.h

TRANSLATIONS += translations/kommute_de.ts \
                translations/kommute_da.ts \
                translations/kommute_es.ts \
                translations/kommute_it.ts \
                translations/kommute_pt.ts \
                translations/kommute_tr.ts \
                translations/kommute_fr.ts \
                translations/kommute_bs.ts \
                translations/kommute_pl.ts \
                translations/kommute_sv.ts \
                translations/kommute_ru.ts \
                translations/kommute_nl.ts \
                translations/kommute_zh_CN.ts \

isEmpty(QMAKE_LRELEASE) {

    win32:QMAKE_LRELEASE = $$[QT_INSTALL_BINS]\lrelease.exe

    else:QMAKE_LRELEASE = $$[QT_INSTALL_BINS]/lrelease

}

updateqm.input = TRANSLATIONS
updateqm.output = ${QMAKE_FILE_PATH}/${QMAKE_FILE_BASE}.qm
updateqm.commands = $$QMAKE_LRELEASE ${QMAKE_FILE_IN} -qm ${QMAKE_FILE_PATH}/${QMAKE_FILE_BASE}.qm
updateqm.CONFIG += no_link

QMAKE_EXTRA_COMPILERS += updateqm

PRE_TARGETDEPS += compiler_updateqm_make_all
