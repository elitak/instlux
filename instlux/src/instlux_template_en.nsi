; filename: instlux.nsi
;
; Linux installer: Install linux boots from you hard disk, so that
;                  the system bios does not need to be changed.
;                  You can choose from different linux installers (different
;                  distributions and different media).
;                  For network installations, program sources will be downloaded
;                  through internet during the install process, so internet 
;                  connection should be provided.
;                  For CD installations, program sources will be downloaded
;                  from a CD, so that you should have a CD. That CD could be
;                  obtained from internet, magazines, ...
;
;                  In order to achieve that, grub for DOS is installed
;                  into your hard disc, as well as the needed kernel and drivers.
;                  Then, boot.ini is changed to include grub on the boot process.
;                  After that, the system will be rebooted so that you can install
;                  linux. Next time you boot windows, the boot.ini will be restored.
;
;
;    Copyright (C) 2005  Jordi Massaguer Pla

;    This program is free software; you can redistribute it and/or modify
;    it under the terms of the GNU General Public License as published by
;    the Free Software Foundation; either version 2 of the License, or
;    (at your option) any later version.
;
;    This program is distributed in the hope that it will be useful,
;    but WITHOUT ANY WARRANTY; without even the implied warranty of
;    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;    GNU General Public License for more details.
;
;    You should have received a copy of the GNU General Public License
;    along with this program; if not, write to the Free Software
;    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
;
; 
;--------------------------------
;Include Modern UI

  !include "MUI.nsh"

;--------------------------------
;General

  Name "NAME"
  AllowRootDirInstall true
  OutFile "..\bin\OUTFILE"
  Caption "CAPTION"

  ;Default installation folder
  !include LogicLib.nsh
  !include FileFunc.nsh ; GetRoot / un.GetRoot
  !insertmacro GetRoot
  !insertmacro un.GetRoot
  BGGradient f8e409  

  !define MUI_ICON "instlux.ico"
  !define MUI_UNICON "instlux.ico"
  !define MUI_HEADERIMAGE
  !define MUI_HEADERIMAGE_BITMAP "instlux_logo.bmp"

  !define MUI_WELCOMEPAGE_TITLE_3LINES 
  !define MUI_FINISHPAGE_TITLE_3LINES
  

;--------------------------------
;Interface Settings

  !define MUI_ABORTWARNING

;--------------------------------
;Pages

  !insertmacro MUI_PAGE_WELCOME
  !insertmacro MUI_PAGE_LICENSE "license_en.txt"
  !insertmacro MUI_PAGE_INSTFILES
  !insertmacro MUI_PAGE_FINISH    
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES
  
;--------------------------------
;Languages
 
  !insertmacro MUI_LANGUAGE "English"

;--------------------------------
; splash

 XPStyle on
 ;icon instlux_logo_icon2.ico
var /GLOBAL c
Function .onInit
	# the plugins dir is automatically deleted when the installer exits
	InitPluginsDir
	${GetRoot} $WINDIR $c
	StrCpy $INSTDIR "$c"
	File /oname=$PLUGINSDIR\splash.bmp "instlux_logo.bmp"
	#optional
	#File /oname=$PLUGINSDIR\splash.wav "C:\myprog\sound.wav"

	splash::show 1000 $PLUGINSDIR\splash

	Pop $0 ; $0 has '1' if the user closed the splash screen early,
			; '0' if everything closed normally, and '-1' if some error occurred.
FunctionEnd

InstallDir "$c\"

;-------------------------------

;Sections

Section ""
  var /GLOBAL MenuLSTFile
  SetOutPath $INSTDIR
  FileOpen $MenuLSTFile $c\menu.lst w
  FileWrite $MenuLSTFile "title MENU_TITLE $\r$\n"
  FileWrite $MenuLSTFile "kernel   (hd0,0)/distros/KERNEL APPEND_IDE$\r$\n"
  FileWrite $MenuLSTFile "initrd   (hd0,0)/distros/DRIVERS$\r$\n"
  ;FileWrite $MenuLSTFile "title MENU_TITLE (SATA or SCSI)$\r$\n"
  ;FileWrite $MenuLSTFile "kernel   (hd0,0)/distros/KERNEL APPEND_SATA$\r$\n"
  ;FileWrite $MenuLSTFile "initrd   (hd0,0)/distros/DRIVERS$\r$\n"
  FileSeek $MenuLSTFile 0 END
  FileClose $MenuLSTFile
  File "grldr"
  LIST_OF_FILES
  SetFileAttributes "$c\boot.ini" NORMAL
  SetFileAttributes "$c\boot.ini" SYSTEM|HIDDEN
  WriteINIStr $c\boot.ini "operating systems" "$c\grldr" '"BOOT_TITLE"' 
  WriteUninstaller "$SMSTARTUP\instlux-uninst.exe"
  SetRebootFlag true

SectionEnd

;-------------------------------------------
; uninstall section

;Uninstaller

Section "Uninstall"
  ${un.GetRoot} $WINDIR $c

  Delete /REBOOTOK "$c\menu.lst"
  Delete /REBOOTOK "$c\grldr"
  RMDir /REBOOTOK /r "$c\distros\OUTPATH"
  SetFileAttributes "$c\boot.ini" NORMAL
  SetFileAttributes "$c\boot.ini" SYSTEM|HIDDEN
  DeleteINIStr $c\boot.ini "operating systems" "$c\grldr"
  Delete /REBOOTOK "$SMSTARTUP\instlux-uninst.exe"
  
SectionEnd


