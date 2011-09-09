# This file is used to generate Windows installer for Siilihai.
# Use makensis.exe from Nullsoft Installer creator.

outFile "Install-Siilihai.exe"
 
installDir $PROGRAMFILES\Siilihai

Page directory
Page instfiles
 
section
    setOutPath $INSTDIR
    File siilihai.exe
    File *.dll
#    File /r data

    writeUninstaller "$INSTDIR\uninstall.exe"
 
    createShortCut "$SMPROGRAMS\Siilihai.lnk" "$INSTDIR\siilihai.exe"
sectionEnd
 
section "uninstall"
    delete "$INSTDIR\uninstall.exe"
    delete "$INSTDIR\*.exe"
    delete "$INSTDIR\*.dll"
#    RMDir /r "$INSTDIR\data"
    RMDir $INSTDIR

    delete "$SMPROGRAMS\Siilihai.lnk"
sectionEnd

