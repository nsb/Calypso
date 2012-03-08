set QTDIR=c:\qt\2010.02.1
set MINGW=%QTDIR%\mingw

set PATH=%QTDIR%\qt\bin;%QTDIR%\bin;%MINGW%\bin;%PATH%

cd ..

qmake kommute.pro

mingw32-make

cd src

lrelease src.pro

"c:\Program files\nsis\makensisw.exe" kommute.nsi

