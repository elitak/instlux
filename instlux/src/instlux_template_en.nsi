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
; Author: Jordi Massaguer i Pla <jordimassaguerpla _ yahoo _ es>
; Contributions: Using "GetRoot" instead of "c:\" has been adapted from the project "goodbye microsoft" from Robert Millan
;                Using a license for each language has been contributed by Daniel Pedigo
;                Detecting whether it is running on a Windows16 bits or Windows32 bits has been contributed by Daniel Pedigo
;                Installing grub.exe and configuring boot.ini on windows98 has been contributed by Daniel Pedigo
;                A way for grub to load the kernel even when its not (0,0) has been contributed by Daniel Pedigo
;				 Daniel Gollub and Sascha Sommer has contributed with some other improvements
;
;    Copyright (C) 2005  Jordi Massaguer i Pla

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
  !insertmacro MUI_PAGE_LICENSE $(license)
  !insertmacro MUI_PAGE_INSTFILES
  !insertmacro MUI_PAGE_FINISH    
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES
  
;--------------------------------
;Languages
 
  !insertmacro MUI_LANGUAGE "English"
  LicenseLangString license ${LANG_ENGLISH} "translations/english/license_english.txt"
  !insertmacro MUI_RESERVEFILE_LANGDLL

;--------------------------------
; splash

 XPStyle on
 ;icon instlux_logo_icon2.ico
var c
var ConfigSYS
var MenuLSTFile
var HitMeFile
var Resolution
var LangParam

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
	!insertmacro MUI_LANGDLL_DISPLAY
	
FunctionEnd

InstallDir "$c\"

;-------------------------------

;Sections

Section "Install"
  SetOutPath $INSTDIR
#   Get Windows Version
  GetVersion::WindowsName
  Pop $R0
  StrCmp $R0 '95' lbl_Win9x
  StrCmp $R0 '95 OSR2' lbl_Win9x
  StrCmp $R0 '98' lbl_Win9x
  StrCmp $R0 '98 SE' lbl_Win9x
  StrCmp $R0 'NT' lbl_WinNT
  StrCmp $R0 '2000' lbl_WinNT
  StrCmp $R0 'XP' lbl_WinNT
  StrCmp $R0 'XP x64' lbl_WinNT
  StrCmp $R0 'Server 2003' lbl_WinNT
  StrCmp $R0 'Server 2003 R2' lbl_WinNT
  StrCmp $R0 'Vista' lbl_WinVista lbl_Error

  
  lbl_Error:
    MessageBox MB_OK "This operating system is not currently supported by the installer."
	Quit
  lbl_Win9x:
    File "grub.exe"
	Rename "$c\config.sys" "$c\config-bak.sys"
	
	FileOpen $ConfigSYS "$c\config.sys" w
	FileWrite $ConfigSYS "[menu]$\r$\n"
    FileWrite $ConfigSYS "menuitem=Windows , Start Windows$\r$\n"
    FileWrite $ConfigSYS "menuitem=BOOT_TITLE , BOOT_TITLE$\r$\n$\r$\n"
    FileWrite $ConfigSYS "[Windows]$\r$\n"
    FileWrite $ConfigSYS "shell=io.sys$\r$\n$\r$\n"
    FileWrite $ConfigSYS "[BOOT_TITLE]$\r$\n"
    FileWrite $ConfigSYS "install=grub.exe"
	FileSeek $ConfigSYS 0 END
    FileClose $ConfigSYS
	
  Goto lbl_Common
  
  lbl_WinNT:
    File "grldr"
    SetFileAttributes "$c\boot.ini" NORMAL
    WriteINIStr "$c\boot.ini" "boot loader" "timeout" "30"
    WriteINIStr "$c\boot.ini" "operating systems" "$c\grldr" '"BOOT_TITLE"'
    SetFileAttributes "$c\boot.ini" SYSTEM|HIDDEN
  Goto lbl_Common

; ********** From Debian-Installer Loader - Robert Millan ****************
  lbl_WinVista:
    File /oname=$c\grldr.mbr "grldr.mbr"
    File /oname=$c\grldr "grldr"
    ReadRegStr $0 HKLM "Software\Linux\Linux-Installer Loader" "bootmgr"
    ${If} $0 == ""
      nsExec::ExecToStack '"bcdedit" /create /d "BOOT_TITLE" /application bootsector'
      Pop $0
      ${If} $0 != 0
        StrCpy $0 bcdedit.exe
        MessageBox MB_OK "Exec Error" ; TODO: translate error string!
        Quit
      ${Endif}
      Pop $0 ; "The entry {id} was successfully created"
      StrCpy $0 $0 38 10
      ; $0 holds the boot id.  Write it down, both for installer idempotency
      ; and for uninstaller.
      WriteRegStr HKLM "Software\Linux\Linux-Installer Loader" "bootmgr" "$0"
    ${Endif}
    nsExec::Exec '"bcdedit" /set $0 device boot'
    nsExec::Exec '"bcdedit" /set $0 path \grldr.mbr'
    nsExec::Exec '"bcdedit" /displayorder $0 /addlast'
  Goto lbl_Common
  
  lbl_Common:
  FileOpen $HitMeFile $c\instlux_hitme.txt a 
  FileWrite $HitMeFile "This file was created by instlux."
  FileSeek $HitMeFile 0 END
  FileClose $HitMeFile

# TODO add all avaliable languages
  StrCmp $LANGUAGE ${LANG_ENGLISH} 0 +2
    StrCpy $LangParam "en"
  StrCmp $LANGUAGE ${LANG_DUTCH} 0 +2
    StrCpy $LangParam "da"
  StrCmp $LANGUAGE ${LANG_FRENCH} 0 +2
    StrCpy $LangParam "fr"
  StrCmp $LANGUAGE ${LANG_GERMAN} 0 +2
    StrCpy $LangParam "de"
#  StrCmp $LANGUAGE ${LANG_KOREAN} 0 +2
#     StrCpy $LangParam "cr"
#  StrCmp $LANGUAGE ${LANG_RUSSIAN} 0 +2
#    StrCpy $LangParam "ru"
  StrCmp $LANGUAGE ${LANG_SPANISH} 0 +2
    StrCpy $LangParam "es"
  StrCmp $LANGUAGE ${LANG_SWEDISH} 0 +2
    StrCpy $LangParam "se"
#  StrCmp $LANGUAGE ${LANG_TRADCHINESE} 0 +2
#    StrCpy $LangParam "zh_TW"
#  StrCmp $LANGUAGE ${LANG_SIMPCHINESE} 0 +2
#    StrCpy $LangParam "zh_CN"
#  StrCmp $LANGUAGE ${LANG_SLOVAK} 0 +2
#    StrCpy $LangParam "sk"

  # http://nsis.sourceforge.net/System_plug-in <--- requires
  System::Call 'user32::GetSystemMetrics(i 0) i .r0'
  System::Call 'user32::GetSystemMetrics(i 1) i .r1'

  # 64k |  0x311    0x314    0x317    0x31A
  IntCmp $0 640 is640 isUnknow 0
  IntCmp $0 800 is800
  IntCmp $0 1024 is1024
  IntCmp $0 1280 is1280 0 is1280 

  isUnknow:
    StrCpy $Resolution "normal"
  Goto writeMenu 
     	
  is640:
    StrCpy $Resolution "0x311"
  Goto writeMenu  

  is800:
    StrCpy $Resolution "0x314"
  Goto writeMenu

  is1024:
    StrCpy $Resolution "0x317"
  Goto writeMenu  

  is1280:
    StrCpy $Resolution "0x31A"
  Goto writeMenu  


  writeMenu:
  FileOpen $MenuLSTFile $c\menu.lst a
  FileWrite $MenuLSTFile "hiddenmenu $\r$\n"
  FileWrite $MenuLSTFile "timeout 0 $\r$\n"

  FileWrite $MenuLSTFile "title MENU_TITLE $\r$\n"
  FileWrite $MenuLSTFile "find --set-root /instlux_hitme.txt$\r$\n"
  FileWrite $MenuLSTFile "kernel   /distros/KERNEL KRNL_APPEND lang=$LangParam vga=$Resolution$\r$\n"
  FileWrite $MenuLSTFile "initrd   /distros/DRIVERS$\r$\n"
  FileSeek $MenuLSTFile 0 END
  FileClose $MenuLSTFile
  LIST_OF_FILES
  WriteUninstaller "$SMSTARTUP\instlux-uninst.exe"
  SetRebootFlag true
SectionEnd

;-------------------------------------------
; uninstall section

;Uninstaller

Section "Uninstall"
  ${un.GetRoot} $WINDIR $c

  #   Get Windows Version
  GetVersion::WindowsName
  Pop $R6
  StrCmp $R6 '95' lbl_Win9x
  StrCmp $R6 '95 OSR2' lbl_Win9x
  StrCmp $R6 '98' lbl_Win9x
  StrCmp $R6 '98 SE' lbl_Win9x
  StrCmp $R6 'NT' lbl_WinNT
  StrCmp $R6 '2000' lbl_WinNT
  StrCmp $R6 'XP' lbl_WinNT
  StrCmp $R6 'XP x64' lbl_WinNT
  StrCmp $R6 'Server 2003' lbl_WinNT
  StrCmp $R6 'Server 2003 R2' lbl_WinNT
  StrCmp $R6 'Vista' lbl_WinVista
  
  lbl_Win9x:
    Delete /REBOOTOK "$c\grub.exe"
	Delete /REBOOTOK "$c\config.sys"
	Rename "$c\config-bak.sys" "$c\config.sys"
  Goto lbl_Finish

  lbl_WinNT:
    Delete /REBOOTOK "$c\grldr"
    SetFileAttributes "$c\boot.ini" NORMAL
    DeleteINIStr "$c\boot.ini" "operating systems" "$c\grldr"
    SetFileAttributes "$c\boot.ini" SYSTEM|HIDDEN
  Goto lbl_Finish

  lbl_WinVista:
     ReadRegStr $0 HKLM "Software\Linux\Linux-Installer Loader" "bootmgr"
     ${If} $0 != ""
       nsExec::Exec '"bcdedit" /delete $0'
       Pop $0
       ${If} $0 != 0
         StrCpy $0 bcdedit.exe
         MessageBox MB_OK "Exec Error"; TODO: translate error string!
       ${Endif}
       DeleteRegKey HKLM "Software\Linux\Linux-Installer Loader"
    ${Endif}
  Goto lbl_Finish

  lbl_Finish:
    RMDir /REBOOTOK /r "$c\distros\OUTPATH"
    Delete /REBOOTOK "$c\menu.lst"
    Delete /REBOOTOK "$c\instlux_hitme.txt"
    Delete /REBOOTOK "$SMSTARTUP\instlux-uninst.exe"
  
SectionEnd
