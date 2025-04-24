; AddSharedDLL
;
; Increments a shared DLLs reference count.
; Use by passing one item on the stack (the full path of the DLL).
;
; Usage:
;   Push $SYSDIR\myDll.dll
;   Call AddSharedDLL
;

Function AddSharedDLL
  Exch $R1
  Push $R0
  ReadRegDword $R0 HKLM Software\Microsoft\Windows\CurrentVersion\SharedDLLs $R1
  IntOp $R0 $R0 + 1
  WriteRegDWORD HKLM Software\Microsoft\Windows\CurrentVersion\SharedDLLs $R1 $R0
  Pop $R0
  Pop $R1
FunctionEnd
