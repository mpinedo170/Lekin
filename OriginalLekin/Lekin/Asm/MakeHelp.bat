@echo off
REM -- First make map file from Microsoft Visual C++ generated Resource.h
echo // MAKEHELP.BAT generated Help Map file.  Used by Asm.HPJ. >"Hlp\Asm.hm"
echo. >>"Hlp\Asm.hm"
echo // Commands (ID_*) >>"Hlp\Asm.hm"
makehm ID_,HID_,0x10000 Resource.h >>"Hlp\Asm.hm"
echo. >>"Hlp\Asm.hm"
echo // Prompts (IDP_*) >>"Hlp\Asm.hm"
makehm IDP_,HIDP_,0x30000 Resource.h >>"Hlp\Asm.hm"
echo. >>"Hlp\Asm.hm"
echo // Resources (IDR_*) >>"Hlp\Asm.hm"
makehm IDR_,HIDR_,0x20000 Resource.h >>"Hlp\Asm.hm"
echo. >>"Hlp\Asm.hm"
echo // Dialogs (IDD_*) >>"Hlp\Asm.hm"
makehm IDD_,HIDD_,0x20000 Resource.h >>"Hlp\Asm.hm"
echo. >>"Hlp\Asm.hm"
echo // Frame Controls (IDW_*) >>"Hlp\Asm.hm"
makehm IDW_,HIDW_,0x50000 Resource.h >>"Hlp\Asm.hm"
REM -- Make help for Project Asm

%1\Helper.exe

echo Copying the actual Help file because hcrtf.exe is not present in the system
copy Hlp\Final\Asm.hlp %1
exit 0

echo Building Win32 Help files
start /wait hcrtf -x "Hlp\Asm.hpj"
echo.
move /Y "Hlp\Asm.hlp" %1
ren %1\Asm.hlp Asm.hlp
echo.


