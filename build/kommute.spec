Summary: MUTE-net GUI for KDE
Name: kommute
Version: 0.2
Release: 1
License: GPL
Group: Networking/File transfer
URL: http://kommute.sourceforge.net/
Distribution: PCLinuxOS
Packager: Gary Whetstone <node45_55@yahoo.com>
Source1: http://downloads.sourceforge.net/kommute/kommute-%{version}-%{release}.tar.gz
BuildRoot: /var/tmp/%{name}-root

%description
Kommute is a KDE file sharing client using the anonymous file sharing MUTE network.

%prep

cd $RPM_BUILD_DIR
/bin/gzip -dc $RPM_SOURCE_DIR/kommute-%{version}-%{release}.tar.gz | tar -xvvf -
STATUS=$?
if [ $STATUS -ne 0 ]; then
  exit $STATUS

fi

%build

cd $RPM_BUILD_DIR/kommute/kommute
qmake
make

%install
cd $RPM_BUILD_DIR/kommute/kommute
cd src
lrelease src.pro
cd ..
qmake
make INSTALL_ROOT="$RPM_BUILD_ROOT" install

%files
/usr/share/
/usr/bin/


%post

