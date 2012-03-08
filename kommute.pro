TEMPLATE = subdirs

SUBDIRS += \
        src/MUTE/crypto src

tr1.path  = $$INSTALLDIR/usr/share/apps/kommute/translations
tr1.commands = cd src;lrelease src.pro;cd ..
INSTALLS += tr1

translate.path  = $$INSTALLDIR/usr/share/apps/kommute/translations
translate.files = src/translations/*.qm
INSTALLS += translate

appli.path = $$INSTALLDIR/usr/bin
appli.files = src/kommute
INSTALLS += appli

applnk.path  = $$INSTALLDIR/usr/share/applnk
applnk.files = src/kommute.desktop
INSTALLS += applnk

gapps.path  = $$INSTALLDIR/usr/share/gnome/apps
gapps.files = src/kommute.desktop
INSTALLS += gapps

ico16.path  = $$INSTALLDIR/usr/share/icons/hicolor/16x16/apps
ico16.files = src/resources/hi16-app-kommute.png
INSTALLS += ico16

ico32.path  = $$INSTALLDIR/usr/share/icons/hicolor/32x32/apps
ico32.files = src/resources/hi32-app-kommute.png
INSTALLS += ico32

ico64.path  = $$INSTALLDIR/usr/share/icons/hicolor/64x64/apps
ico64.files = src/resources/hi64-app-kommute.png
INSTALLS += ico64

ico128.path  = $$INSTALLDIR/usr/share/icons/hicolor/128x128/apps
ico128.files = src/resources/hi128-app-kommute.png
INSTALLS += ico128

doc.path  = $$INSTALLDIR/usr/share/doc/kommute/
doc.files = src/doc/*.html src/doc/*.png
INSTALLS += doc

doc_common.path  = $$INSTALLDIR/usr/share/doc/kommute/common/
doc_common.files = src/doc/common/*
INSTALLS += doc_common

