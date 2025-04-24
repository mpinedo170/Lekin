;NSIS Modern User Interface version 1.63

!include "MUI.nsh"
!include "LocalSys.nsh"
!include "WinMessages.nsh"
!include "GetWindowsKey.nsh"
!include "x64.nsh"

;--------------------------------
;Configuration

  ;Name and file
  !define FULL_NAME "LEKIN -- Scheduling System"
  !define REG_KEY_SOFTWARE "SOFTWARE\Feldman\${FULL_NAME}"
  !define REG_KEY_SOFTWARE_VITAL "${REG_KEY_SOFTWARE}\Vital"
  !define REG_KEY_UNINSTALL "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\${FULL_NAME}"

  Name "${FULL_NAME}"
  !define PRODUCT "LEKIN"
  !define VERSION "3.0"
  OutFile "..\Media\Lekin.exe"

  ;Default installation folder
  InstallDir "$PROGRAMFILES\${PRODUCT}"

  ;Get installation folder from registry if available
  InstallDirRegKey HKLM "${REG_KEY_SOFTWARE_VITAL}" "Program"

  !define MUI_NAME "${FULL_NAME}"
  !define MUI_ICON "..\Asm\Res\Lekin.ico"
  !define MUI_UNICON "Uninstall.ico"

  !define MUI_LICENSEPAGE
  !define MUI_DIRECTORYPAGE
  !define MUI_ABORTWARNING
  !define MUI_UNINSTALLER
  !define MUI_UNCONFIRMPAGE

;--------------------------------
;Pages

  !insertmacro MUI_PAGE_LICENSE "..\License.txt"
  !define MUI_PAGE_CUSTOMFUNCTION_SHOW lekin.DirectoryShow
  !insertmacro MUI_PAGE_DIRECTORY
  Page custom SetCompany
  !insertmacro MUI_PAGE_INSTFILES
  !insertmacro MUI_PAGE_FINISH

  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES

;--------------------------------
;Reserve Files

  ;Things that need to be extracted on first (keep these lines before any File command!)
  ;Only useful for BZIP2 compression

  ReserveFile "ioCompany.ini"
  !insertmacro MUI_RESERVEFILE_INSTALLOPTIONS

;--------------------------------
;Installer Sections

Section "" SecLekin
  SetShellVarContext all

  ; Set output path to the installation directory.
  SetOutPath "$INSTDIR"

  ClearErrors
  WriteUninstaller "$INSTDIR\Uninstall.exe"
  IfErrors Errors

  !define LOCAL_EXE_PATH "..\..\_output\Release"
  File "${LOCAL_EXE_PATH}\Asm.exe"
  File "${LOCAL_EXE_PATH}\Asm.hlp"
  File "..\Misc\IComp.exe"
  File "${LOCAL_EXE_PATH}\AFLib_R.dll"
  File "..\Readme.txt"
  File "${LOCAL_X86_PATH}\${X86_EXE}"
  File "${LOCAL_X64_PATH}\${X64_EXE}"

  SetOutPath "$INSTDIR\Alg"
  File "${LOCAL_EXE_PATH}\LSearch.exe"
  File "${LOCAL_EXE_PATH}\SB.exe"
  File "${LOCAL_EXE_PATH}\SBLS.exe"
  File "${LOCAL_EXE_PATH}\SBTWT.exe"
  File "..\Misc\Alkis\RunASB.bat"
  File "..\Misc\Alkis\A2Lekin.exe"
  File "..\Misc\Alkis\ASB.exe"
  File "..\Misc\Alkis\Lekin2A.exe"

  !include "Data.nsh"

  ${If} ${RunningX64}
	ExecWait "{$INSTDIR}\${X64_EXE} /q"
  ${Else}
	ExecWait "{$INSTDIR}\${X86_EXE} /q"
  ${EndIf}

  ReadRegStr $R0 HKLM "${REG_KEY_SOFTWARE_VITAL}" "Version"
  StrLen $R1 $R0
  StrCmp $R0 "3" NoKillAlgoriths
  IntCmp $R1 2 0 0 NoKillAlgoriths

  ; Delete algorithms, if the old version is less than 3 characters and is not equal to "3"
  DeleteRegKey HKLM "${REG_KEY_SOFTWARE}\Algorithms"

NoKillAlgoriths:
  ;Store install folder
  WriteRegStr HKLM "${REG_KEY_SOFTWARE_VITAL}" "Program" $INSTDIR
  WriteRegStr HKLM "${REG_KEY_SOFTWARE_VITAL}" "Algorithms" $INSTDIR\Alg

  StrCpy $R0 "$SMPROGRAMS\${FULL_NAME}"
  CreateDirectory "$R0"
  CreateShortcut "$R0\${PRODUCT}.lnk" "$INSTDIR\Asm.exe"
  CreateShortcut "$R0\${PRODUCT} Help.lnk" "$INSTDIR\Asm.hlp"

  DeleteRegValue HKLM "${REG_KEY_SOFTWARE_VITAL}" "License"
  ClearErrors
  ExecWait `"$INSTDIR\Asm.exe" /Register`
  ReadRegStr $R0 HKLM "${REG_KEY_SOFTWARE_VITAL}" "License"
  StrCmp $R0 "Demo" 0 RegisterErrors

  WriteRegStr HKLM "${REG_KEY_UNINSTALL}" DisplayIcon `"$INSTDIR\Asm.exe"`
  WriteRegStr HKLM "${REG_KEY_UNINSTALL}" DisplayName `${FULL_NAME}`
  WriteRegStr HKLM "${REG_KEY_UNINSTALL}" UninstallString `"$INSTDIR\Uninstall.exe"`
  WriteRegStr HKLM "${REG_KEY_SOFTWARE_VITAL}" "Version" "${VERSION}"
  Return

RegisterErrors:
  MessageBox MB_OK|MB_ICONSTOP "LEKIN was not successfully registered."
  Quit

Errors:
  MessageBox MB_OK|MB_ICONSTOP "Please ask your System Administrator to install LEKIN."
  Quit
SectionEnd

;--------------------------------
;Languages

  !insertmacro MUI_LANGUAGE "English"

;--------------------------------
;Language Strings

  ;Description
  LangString DESC_SecCopyUI ${LANG_ENGLISH} "Copying files..."

  ;Header
  LangString TEXT_IO_TITLE ${LANG_ENGLISH} "Call sign"
  LangString TEXT_IO_SUBTITLE ${LANG_ENGLISH} "Please enter your call sign."

  !insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${SecLekin} $(DESC_SecCopyUI)
  !insertmacro MUI_FUNCTION_DESCRIPTION_END

;--------------------------------
;Installer Functions

Function .onInit
  ;Extract InstallOptions INI Files
  !insertmacro MUI_INSTALLOPTIONS_EXTRACT "ioCompany.ini"
FunctionEnd

Function lekin.DirectoryShow
  Push $R0
  Push $R1
  ReadRegStr $R0 HKLM "${REG_KEY_SOFTWARE_VITAL}" "Program"
  StrCmp $R0 "" done

  FindWindow $R0 "#32770" "" $HWNDPARENT
  GetDlgItem $R1 $R0 1019 ; directory string
  SendMessage $R1 0xCF 1 0 ; EM_SETREADONLY

  GetDlgItem $R1 $R0 1001 ; browse button
  System::Call "user32::EnableWindow(i $R1, i 0) .s"

  GetDlgItem $R1 $R0 1006 ; comment
  SendMessage $R1 ${WM_SETTEXT} 0 "STR:Setup has found a previous installation of ${PRODUCT} in the following folder.$\n$\r$\n$\rTo install in a different folder, cancel Setup, uninstall ${PRODUCT} and run Setup again.$\n$\r$\n$\rClick Next to continue."

done:
  Pop $R1
  Pop $R0
FunctionEnd

Function SetCompany
  Push $R0
  Push $R1
  Push $R2

  ReadRegStr $R1 HKLM "${REG_KEY_SOFTWARE_VITAL}" "Company"
  StrCmp $R1 "" 0 CompanyReady

  Call GetWindowsKey
  Pop $R0
  ReadRegStr $R1 HKLM $R0 "RegisteredOrganization"
  ReadRegStr $R2 HKLM $R0 "RegisteredOwner"

  StrCmp $R1 "" R1Empty
  StrCmp $R2 "" CompanyReady
  StrCpy $R1 "$R1 - $R2"
  Goto CompanyReady

R1Empty:
  StrCpy $R1 $R2

CompanyReady:
  !insertmacro MUI_HEADER_TEXT "$(TEXT_IO_TITLE)" "$(TEXT_IO_SUBTITLE)"
  !insertmacro MUI_INSTALLOPTIONS_WRITE "ioCompany.ini" "Field 2" "State" $R1
  !insertmacro MUI_INSTALLOPTIONS_DISPLAY "ioCompany.ini"

  ;Read a value from an InstallOptions INI File
  !insertmacro MUI_INSTALLOPTIONS_READ $R0 "ioCompany.ini" "Field 2" "State"
  ClearErrors
  WriteRegStr HKLM "${REG_KEY_SOFTWARE_VITAL}" "Company" $R0
  IfErrors Errors

  Pop $R2
  Pop $R1
  Pop $R0
  Return

Errors:
  MessageBox MB_OK "Please ask your System Administrator to install LEKIN."
  Quit
FunctionEnd

;--------------------------------
;Uninstaller Section

Section "Uninstall"
  SetShellVarContext all
  ExecWait `"$INSTDIR\Asm.exe" /Unregister /Embedding`

  RMDir /r "$INSTDIR"
  RMDir /r "$SMPROGRAMS\${FULL_NAME}"
  DeleteRegKey HKLM "${REG_KEY_SOFTWARE}"
  DeleteRegKey HKLM "${REG_KEY_UNINSTALL}"
SectionEnd
