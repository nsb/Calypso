;Name "Kommute"
!include Library.nsh

; Define your application name
!define APPNAME "Kommute"
!define VERSION "0.24"
!define APPNAMEANDVERSION "${APPNAME} ${VERSION}"
!define QTBASE "c:\qt\2010.02.1"

; Main Install settings
Name "${APPNAMEANDVERSION}"
InstallDir "$PROGRAMFILES\Kommute"
InstallDirRegKey HKLM "Software\${APPNAME}" "Install_Dir"
OutFile "kommute-${VERSION}.exe"
BrandingText "${APPNAMEANDVERSION}"


;--------------------------------

; Modern interface settings
!include Sections.nsh
!include "MUI.nsh"


;Interface Settings
!define MUI_ABORTWARNING

# MUI defines
;!define MUI_ICON "${NSISDIR}\Contrib\Graphics\Icons\modern-install-blue.ico"
!define MUI_FINISHPAGE_NOAUTOCLOSE
!define MUI_LICENSEPAGE_RADIOBUTTONS
!define MUI_COMPONENTSPAGE_SMALLDESC
!define MUI_FINISHPAGE_LINK "Visit the Kommute forum for the latest news and support"
!define MUI_FINISHPAGE_LINK_LOCATION "http://kommute.sourceforge.net/forum/"
!define MUI_FINISHPAGE_RUN "$INSTDIR\Kommute.exe"
!define MUI_FINISHPAGE_SHOWREADME $INSTDIR\changelog.txt
!define MUI_FINISHPAGE_SHOWREADME_TEXT changelog.txt
!define MUI_FINISHPAGE_SHOWREADME_NOTCHECKED
;!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\modern-uninstall-blue.ico"
!define MUI_UNFINISHPAGE_NOAUTOCLOSE
!define MUI_LANGDLL_REGISTRY_ROOT HKLM
!define MUI_LANGDLL_REGISTRY_KEY ${REGKEY}
!define MUI_LANGDLL_REGISTRY_VALUENAME InstallerLanguage

;--------------------------------
;Configuration


  ;!insertmacro MUI_RESERVEFILE_SPECIALBITMAP
 
  LicenseLangString myLicenseData 1033 "license\license.txt"
  LicenseLangString myLicenseData 1031 "license\license-GER.txt"
  LicenseLangString myLicenseData 1036 "license\license-FR.txt"
  LicenseLangString myLicenseData 1055 "license\license-TR.txt"
  LicenseLangString myLicenseData 1049 "license\license-RU.txt"
  LicenseLangString myLicenseData 1040 "license\license-IT.txt"
  LicenseLangString myLicenseData 1053 "license\license.txt"
  LicenseLangString myLicenseData 1034 "license\license-SP.txt"
  LicenseLangString myLicenseData 1045 "license\license.txt"
  LicenseLangString myLicenseData 2052 "license\license.txt"
  LicenseLangString myLicenseData 1030 "license\license-DK.txt"
  LicenseLangString myLicenseData 1041 "license\license.txt"
  LicenseLangString myLicenseData 1042 "license\license.txt"
  
  LicenseData $(myLicenseData)

# Installer pages
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "$(myLicenseData)"
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

# Installer languages
!define MUI_LANGDLL_ALLLANGUAGES

!insertmacro MUI_LANGUAGE English
!insertmacro MUI_LANGUAGE German
!insertmacro MUI_LANGUAGE French
!insertmacro MUI_LANGUAGE Italian
!insertmacro MUI_LANGUAGE Turkish
!insertmacro MUI_LANGUAGE Russian
!insertmacro MUI_LANGUAGE Swedish
!insertmacro MUI_LANGUAGE Spanish
!insertmacro MUI_LANGUAGE Polish
!insertmacro MUI_LANGUAGE SimpChinese
!insertmacro MUI_LANGUAGE Danish
!insertmacro MUI_LANGUAGE Japanese
!insertmacro MUI_LANGUAGE Korean


  ;Component-selection page
    ;Titles
    
    LangString sec_kommute ${LANG_ENGLISH} "Kommute (required)"
    LangString sec_link ${LANG_ENGLISH} "Take kommute-Links"
    LangString sec_startmenu ${LANG_ENGLISH} "Start Menu Shortcuts"
    LangString sec_shortcuts ${LANG_ENGLISH} "Shortcuts"
    LangString DESC_sec_kommute ${LANG_ENGLISH} "Installs the Kommute program files."
    LangString DESC_sec_link ${LANG_ENGLISH} "Registers Kommute to take kommute-Links."
    LangString DESC_sec_startmenu ${LANG_ENGLISH} "Create Start Menu Shortcuts"
    LangString DESC_sec_shortcuts ${LANG_ENGLISH} "Create Kommute shortcut icons."
    LangString LANGUAGEID ${LANG_ENGLISH} "1033"
    
    LangString sec_kommute ${LANG_GERMAN} "Kommute  Programmdateien(erforderlich)"
    LangString sec_link ${LANG_GERMAN} "kommute URL verknüpfung"
    LangString sec_startmenu ${LANG_GERMAN} "Start Menu Verknüpfung"
    LangString sec_shortcuts ${LANG_GERMAN} "Shortcuts"
    LangString DESC_sec_kommute ${LANG_GERMAN} "Installiert die erforderlichen Programmdateien."
    LangString DESC_sec_link ${LANG_GERMAN} "Registriert Kommute für die Annahme von kommute-Links."
    LangString DESC_sec_startmenu ${LANG_GERMAN} "Erstellt eine Start Menu Verknüpfung"
    LangString DESC_sec_shortcuts ${LANG_GERMAN} "Erstellt eine Kommute Verknüpfung als Desktop und/oder Schnellstart Icon."
    LangString LANGUAGEID ${LANG_GERMAN} "1031"
    
    LangString sec_kommute ${LANG_SPANISH} "Kommute archivos de programa"
    LangString sec_link ${LANG_SPANISH} "Capturar enlaces kommute-link"
    LangString sec_startmenu ${LANG_SPANISH} "Accesos directos del menú Inicio"
    LangString sec_shortcuts ${LANG_SPANISH} "Accesos directos"
    LangString DESC_sec_kommute ${LANG_SPANISH} "Instalar los archivos del programa"
    LangString DESC_sec_link ${LANG_SPANISH} "Registrar la asociación de los enlaces kommute."
    LangString DESC_sec_startmenu ${LANG_SPANISH} "Crear accesos directos del menú Inicio"
    LangString DESC_sec_shortcuts ${LANG_SPANISH} "Kommute Crear iconos de acceso directo."
    LangString LANGUAGEID ${LANG_SPANISH} "1034"

    LangString sec_kommute ${LANG_FRENCH} "Kommute (obligatoire)"
    LangString sec_link ${LANG_FRENCH} "Prendre les liens kommute"
    LangString sec_startmenu ${LANG_FRENCH} "Raccourcis du menu Démarrer"
    LangString sec_shortcuts ${LANG_FRENCH} "Raccourcis"
    LangString DESC_sec_kommute ${LANG_FRENCH} "Installe les fichiers du programme."
    LangString DESC_sec_link ${LANG_FRENCH} "Ajouter Kommute dans le registre pour prendre les liens kommute."
    LangString DESC_sec_startmenu ${LANG_FRENCH} "Crée les raccourcis du menu Démarrer"
    LangString DESC_sec_shortcuts ${LANG_FRENCH} "Crée une icône sur le bureau."
    LangString LANGUAGEID ${LANG_FRENCH} "1036"
    
    LangString sec_kommute ${LANG_ITALIAN} "Kommute Programmi"
    LangString sec_link ${LANG_ITALIAN} "Prendi gli kommute-Links"
    LangString sec_startmenu ${LANG_ITALIAN} "Start Menu Scorciatoie"
    LangString sec_shortcuts ${LANG_ITALIAN} "Scorciatoie"
    LangString DESC_sec_kommute ${LANG_ITALIAN} "Installa i Files del Programma"
    LangString DESC_sec_link ${LANG_ITALIAN} "Fai Prendere ad Kommute gli kommute-Links."
    LangString DESC_sec_startmenu ${LANG_ITALIAN} "Create Start Menu Shortcuts"
    LangString DESC_sec_shortcuts ${LANG_ITALIAN} "Create Kommute shortcut icons."
    LangString LANGUAGEID ${LANG_ITALIAN} "1040"
    
    LangString sec_kommute ${LANG_POLISH} "Kommute Pliki programu"
    LangString sec_link ${LANG_POLISH} "Skojarz linki kommute"
    LangString sec_startmenu ${LANG_POLISH} "Start Menu Shortcuts"
    LangString sec_shortcuts ${LANG_POLISH} "Shortcuts"
    LangString DESC_sec_kommute ${LANG_POLISH} "Zainstaluj g??wne pliki programu"
    LangString DESC_sec_link ${LANG_POLISH} "Skojarz linki kommute"
    LangString DESC_sec_startmenu ${LANG_POLISH} "Create Start Menu Shortcuts"
    LangString DESC_sec_shortcuts ${LANG_POLISH} "Create Kommute shortcut icons."
    LangString LANGUAGEID ${LANG_POLISH} "1045"
	
    LangString sec_kommute ${LANG_RUSSIAN} "Kommute "
    LangString sec_link ${LANG_RUSSIAN} "Ïåðåõâàòûâàòü kommute-Ëèíêè"
    LangString sec_startmenu ${LANG_RUSSIAN} "Start Menu Shortcuts"
    LangString sec_shortcuts ${LANG_RUSSIAN} "Shortcuts"
    LangString DESC_sec_kommute ${LANG_RUSSIAN} "Installs the Kommute program files."
    LangString DESC_sec_startmenu ${LANG_RUSSIAN} "Create Start Menu Shortcuts"
    LangString DESC_sec_shortcuts ${LANG_RUSSIAN} "Create Kommute shortcut icons."
    LangString LANGUAGEID ${LANG_RUSSIAN} "1049"
    
    LangString sec_kommute ${LANG_SWEDISH} "Kommute Program files"
    LangString sec_link ${LANG_SWEDISH} "kommute-links"
    LangString sec_startmenu ${LANG_SWEDISH} "Start Menu"
    LangString sec_shortcuts ${LANG_SWEDISH} "Shortcuts"
    LangString DESC_sec_kommute ${LANG_SWEDISH} "Installs the Kommute program files."
    LangString DESC_sec_link ${LANG_SWEDISH} "Take kommute-Links."
    LangString DESC_sec_startmenu ${LANG_SWEDISH} "Create Start Menu Shortcuts"
    LangString DESC_sec_shortcuts ${LANG_SWEDISH} "Create Kommute shortcut icons."
    LangString LANGUAGEID ${LANG_SWEDISH} "1053"
    
    LangString sec_kommute ${LANG_TURKISH} "Kommute Program Dosyalarý"
    LangString sec_link ${LANG_TURKISH} "kommute-Ba?lant?lar?n? Al"
    LangString sec_startmenu ${LANG_TURKISH} "Start Menu"
    LangString sec_shortcuts ${LANG_TURKISH} "Shortcut'lar"
    LangString DESC_sec_kommute ${LANG_TURKISH} "Program dosyalarýný kurar."
    LangString DESC_sec_link ${LANG_TURKISH} "Kommute'u kommute-Ba?lant?lar?n? almas? i?in kaydettirir."
    LangString DESC_sec_startmenu ${LANG_TURKISH} "Create Start Menu Shortcuts"
    LangString DESC_sec_shortcuts ${LANG_TURKISH} "Shortcut yap Start menu , Desktop veya Quicklaunchbar"
    LangString LANGUAGEID ${LANG_TURKISH} "1055"
    
    LangString sec_kommute ${LANG_SIMPCHINESE} "Kommute³ÌÐòÎÄ¼þ"
    LangString sec_link ${LANG_SIMPCHINESE} "»ñÈ¡kommuteÁ´½Ó"
    LangString sec_startmenu ${LANG_SIMPCHINESE} "¿ªÊ¼²Ëµ¥Ïî"
    LangString sec_shortcuts ${LANG_SIMPCHINESE} "¿ì½Ý·½Ê½"
    LangString DESC_sec_kommute ${LANG_SIMPCHINESE} "°²×°´Ë³ÌÐò"
    LangString DESC_sec_link ${LANG_SIMPCHINESE} "µã»÷Á´½ÓÀ´×¢²áKommute¡£"
    LangString DESC_sec_startmenu ${LANG_SIMPCHINESE} "Éú³É¿ªÊ¼²Ëµ¥Ïî¡£"
    LangString DESC_sec_shortcuts ${LANG_SIMPCHINESE} "Éú³ÉKommute¿ì½Ý·½Ê½Í¼±ê¡£"
    LangString LANGUAGEID ${LANG_SIMPCHINESE} "2052"
    
    LangString sec_kommute ${LANG_DANISH} "Programmer"
    LangString sec_link ${LANG_DANISH} "Overtag kommute-Links"
    LangString sec_startmenu ${LANG_DANISH} "Start Menu"
    LangString sec_shortcuts ${LANG_DANISH} "Shortcuts"
    LangString DESC_sec_kommute ${LANG_DANISH} "Installerer programfilen kommute.exe"
    LangString DESC_sec_link ${LANG_DANISH} "Registrerer kommute til at overtage kommute-Links."
    LangString DESC_sec_startmenu ${LANG_DANISH} "Create Start Menu Shortcuts"
    LangString DESC_sec_shortcuts ${LANG_DANISH} "Create Kommute shortcut icons."
    LangString LANGUAGEID ${LANG_DANISH} "1030"
    
    LangString sec_kommute ${LANG_JAPANESE} "Kommute Program files"
    LangString sec_link ${LANG_JAPANESE} "Take kommute-Links."
    LangString sec_startmenu ${LANG_JAPANESE} "Start Menu Shortcuts"
    LangString sec_shortcuts ${LANG_JAPANESE} "Shortcuts"
    LangString DESC_sec_kommute ${LANG_JAPANESE} "Installs the Kommute program files."
    LangString DESC_sec_link ${LANG_JAPANESE} "Registers Kommute to take kommute-Links."
    LangString DESC_sec_startmenu ${LANG_JAPANESE} "Create Start Menu Shortcuts"
    LangString DESC_sec_shortcuts ${LANG_JAPANESE} "Create Kommute shortcut icons."
    LangString LANGUAGEID ${LANG_JAPANESE} "1041"
        
    LangString sec_kommute ${LANG_KOREAN} "Kommute Program files"
    LangString sec_link ${LANG_KOREAN} "Take kommute-Links."
    LangString sec_startmenu ${LANG_KOREAN} "Start Menu Shortcuts"
    LangString sec_shortcuts ${LANG_KOREAN} "Shortcuts"
    LangString DESC_sec_kommute ${LANG_KOREAN} "Installs the Kommute program files."
    LangString DESC_sec_link ${LANG_KOREAN} "Registers Kommute to take kommute-Links."
    LangString DESC_sec_startmenu ${LANG_KOREAN} "Create Start Menu Shortcuts"
    LangString DESC_sec_shortcuts ${LANG_KOREAN} "Create Kommute shortcut icons."
    LangString LANGUAGEID ${LANG_KOREAN} "1042"


; The stuff to install
Section $(sec_kommute) sec_kommute


  SectionIn RO
  
  ; Set output path to the installation directory.
  SetOutPath $INSTDIR
  
  ; Put file there

  File /oname=$INSTDIR\kommute.exe "release\kommute.exe" 

  File /r "doc"
  File /r "*.qm"
  File /r  ${QTBASE}\qt\qt_*.qm
  File /r "changelog.txt"


!insertmacro InstallLib DLL NOTSHARED NOREBOOT_PROTECTED ${QTBASE}\mingw\bin\mingwm10.dll $SYSDIR\mingwm10.dll $SYSDIR
!insertmacro InstallLib DLL NOTSHARED NOREBOOT_PROTECTED ${QTBASE}\mingw\bin\libgcc_s_dw2-1.dll $SYSDIR\libgcc_s_dw2-1.dll $SYSDIR
!insertmacro InstallLib DLL NOTSHARED NOREBOOT_PROTECTED ${QTBASE}\qt\bin\QtCore4.dll $SYSDIR\QtCore4.dll $SYSDIR
!insertmacro InstallLib DLL NOTSHARED NOREBOOT_PROTECTED ${QTBASE}\qt\bin\QtGui4.dll $SYSDIR\QtGui4.dll $SYSDIR
!insertmacro InstallLib DLL NOTSHARED NOREBOOT_PROTECTED ${QTBASE}\qt\bin\QtNetwork4.dll $SYSDIR\QtNetwork4.dll $SYSDIR
!insertmacro InstallLib DLL NOTSHARED NOREBOOT_PROTECTED ${QTBASE}\qt\bin\QtXml4.dll $SYSDIR\QtXml4.dll $SYSDIR
!insertmacro InstallLib DLL NOTSHARED NOREBOOT_PROTECTED ${QTBASE}\qt\bin\QtScript4.dll $SYSDIR\QtScript4.dll $SYSDIR
!insertmacro InstallLib DLL NOTSHARED NOREBOOT_PROTECTED ${QTBASE}\qt\bin\QtOpenGL4.dll $SYSDIR\QtOpenGL4.dll $SYSDIR  
  
  ; Write the installation path into the registry
  WriteRegStr HKLM SOFTWARE\Kommute "Install_Dir" "$INSTDIR"
  
  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Kommute" "DisplayName" "Kommute"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Kommute" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Kommute" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Kommute" "NoRepair" 1
  WriteUninstaller "uninstall.exe"
  
SectionEnd

Section $(sec_link) sec_link
    SectionIn 1 2
    DeleteRegKey HKCR "kommute"
    WriteRegStr HKEY_CLASSES_ROOT kommute "" "URL: kommute Protocol"
    WriteRegStr HKEY_CLASSES_ROOT kommute "URL Protocol" ""
    WriteRegStr HKEY_CLASSES_ROOT kommute\shell "" open
    WriteRegStr HKEY_CLASSES_ROOT kommute\DefaultIcon "" $INSTDIR\Kommute.exe,1
    WriteRegStr HKEY_CLASSES_ROOT kommute\shell\open\command "" '"$INSTDIR\kommute.exe" "%1"'    
    DeleteRegKey HKLM "SOFTWARE\Classes\kommute"       
    WriteRegStr HKEY_LOCAL_MACHINE SOFTWARE\Classes\kommute "" "URL: kommute Protocol"
    WriteRegStr HKEY_LOCAL_MACHINE SOFTWARE\Classes\kommute "URL Protocol" ""
    WriteRegStr HKEY_LOCAL_MACHINE SOFTWARE\Classes\kommute\shell "" open
    WriteRegStr HKEY_LOCAL_MACHINE SOFTWARE\Classes\kommute\DefaultIcon "" $INSTDIR\kommute.exe
    WriteRegStr HKEY_LOCAL_MACHINE SOFTWARE\Classes\kommute\shell\open\command "" '"$INSTDIR\kommute.exe" "%1"'

SectionEnd

; Optional section (can be disabled by the user)
Section $(sec_startmenu) sec_startmenu

  CreateDirectory "$SMPROGRAMS\Kommute"
  CreateShortCut "$SMPROGRAMS\Kommute\$(^UninstallLink).lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
  CreateShortCut "$SMPROGRAMS\Kommute\Kommute.lnk" "$INSTDIR\Kommute.exe" "" "$INSTDIR\Kommute.exe" 0
  
SectionEnd

SectionGroup $(sec_shortcuts) sec_shortcuts
Section Desktop SEC0001

  CreateShortCut "$DESKTOP\Kommute.lnk" "$INSTDIR\Kommute.exe" "" "$INSTDIR\Kommute.exe" 0
  
SectionEnd

Section Quicklaunchbar SEC0002
  

  CreateShortCut "$QUICKLAUNCH\Kommute.lnk" "$INSTDIR\Kommute.exe" "" "$INSTDIR\Kommute.exe" 0
  
SectionEnd
SectionGroupEnd   

;--------------------------------
;Descriptions

!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${sec_kommute} $(DESC_sec_kommute)
    !insertmacro MUI_DESCRIPTION_TEXT ${sec_link} $(DESC_sec_link)
    !insertmacro MUI_DESCRIPTION_TEXT ${sec_startmenu} $(DESC_sec_startmenu)
    !insertmacro MUI_DESCRIPTION_TEXT ${sec_shortcuts} $(DESC_sec_shortcuts)
!insertmacro MUI_FUNCTION_DESCRIPTION_END     

;--------------------------------

; Uninstaller

Section "Uninstall"
  
  ; Remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Kommute"
  DeleteRegKey HKLM SOFTWARE\Kommute

  ; Remove files and uninstaller
  Delete $INSTDIR\Kommute.exe
  Delete $INSTDIR\MUTE.log
  RMDir /r $INSTDIR\doc
  RMDir /r $INSTDIR\translations
  Delete $INSTDIR\uninstall.exe

  ; Remove shortcuts, if any
  Delete "$SMPROGRAMS\Kommute\*.*"
  ; Remove desktop shortcut 
  Delete "$DESKTOP\Kommute.lnk"
  ; Remove Quicklaunch shortcut 
  Delete "$QUICKLAUNCH\Kommute.lnk"

  ; Remove directories used
  RMDir "$SMPROGRAMS\Kommute"
  RMDir "$INSTDIR"

SectionEnd

Function .onInit

    !insertmacro MUI_LANGDLL_DISPLAY

FunctionEnd

# Installer Language Strings
# TODO Update the Language Strings with the appropriate translations.

LangString ^UninstallLink ${LANG_ENGLISH} "Uninstall"
LangString ^UninstallLink ${LANG_GERMAN} "Deinstallieren"
LangString ^UninstallLink ${LANG_SPANISH} "Desinstalar"
LangString ^UninstallLink ${LANG_FRENCH} "Désinstaller"
LangString ^UninstallLink ${LANG_POLISH} "Odinstaluj"
LangString ^UninstallLink ${LANG_RUSSIAN} "???????"
LangString ^UninstallLink ${LANG_SWEDISH} "Avinstallera"
LangString ^UninstallLink ${LANG_ITALIAN} "Disinstalla"
LangString ^UninstallLink ${LANG_TURKISH} "Kaldýr"
LangString ^UninstallLink ${LANG_SIMPCHINESE} "Ð¶ÔØ"
LangString ^UninstallLink ${LANG_DANISH} "Afinstaller Kommute"
LangString ^UninstallLink ${LANG_JAPANESE} "Uninstall Kommute"
LangString ^UninstallLink ${LANG_KOREAN} "Uninstall Kommute"
