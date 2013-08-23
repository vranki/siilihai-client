# This file is used to generate Windows installer for Siilihai.
# Use makensis.exe from Nullsoft Installer creator.

outFile "siilihai-win32-latest.exe"
 
installDir $PROGRAMFILES\Siilihai

Page directory
Page instfiles
 
section
    setOutPath $INSTDIR
    File siilihai-client.exe
    File *.dll
    File *.ico

    writeUninstaller "$INSTDIR\uninstall.exe"
 
    createShortCut "$SMPROGRAMS\Siilihai.lnk" "$INSTDIR\siilihai-client.exe" "$INSTDIR\siilis_icon_16.ico"

    createDirectory "$INSTDIR\platforms"
    setOutPath "$INSTDIR\platforms"
    file "qwindows.dll"
sectionEnd
 
section "uninstall"
    delete "$INSTDIR\uninstall.exe"
    delete "$INSTDIR\*.exe"
    delete "$INSTDIR\*.dll"
    delete "$INSTDIR\*.ico"
    delete "$INSTDIR\platforms\*.*"
    RMDir "$INSTDIR\platforms"
    RMDir $INSTDIR

    delete "$SMPROGRAMS\Siilihai.lnk"
sectionEnd

