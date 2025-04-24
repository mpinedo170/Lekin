Function GetWindowsKey
  Push $R0
  Push $R1
  StrCpy $R0 "SOFTWARE\Microsoft\Windows NT\CurrentVersion"
  ReadRegStr $R1 HKLM $R0 CurrentVersion
  StrCmp $R1 "" 0 done

  ; we are not NT.
  StrCpy $R0 "SOFTWARE\Microsoft\Windows\CurrentVersion"
  ReadRegStr $R1 HKLM $R0 VersionNumber
  StrCmp $R1 "" 0 done

  MessageBox "MB_OK|MB_ICONEXCLAMATION" "Cannot determine Windows version, aborting."
  Quit
done:
  Pop $R1
  Exch $R0
FunctionEnd
