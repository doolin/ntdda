# Microsoft Developer Studio Project File - Name="ntdda" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=ntdda - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "ntdda.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ntdda.mak" CFG="ntdda - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ntdda - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "ntdda - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "ntdda - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "DDAFORWINDOWS" /FAs /Fr /YX /FD /c
# SUBTRACT CPP /WX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib libxml.lib wsock32.lib htmlhelp.lib /nologo /subsystem:windows /profile /debug /machine:I386 /out:"bin/ntdda.exe"

!ELSEIF  "$(CFG)" == "ntdda - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "include/" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "DDAFORWINDOWS" /FR /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /fo"ntdda.res" /i "..\include\\" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib comctl32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib opengl32.lib glu32.lib glut32.lib libxmld.lib wsock32.lib htmlhelp.lib /nologo /subsystem:windows /debug /machine:I386 /out:"bin/ntddad.exe" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "ntdda - Win32 Release"
# Name "ntdda - Win32 Debug"
# Begin Group "numerics"

# PROP Default_Filter ""
# Begin Group "analysis"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\analysisdata.c
# End Source File
# Begin Source File

SOURCE=.\src\analysisdriver.c
# End Source File
# Begin Source File

SOURCE=.\src\combineddf.c
# End Source File
# Begin Source File

SOURCE=.\src\constants.c
# End Source File
# Begin Source File

SOURCE=.\src\contact.c
# End Source File
# Begin Source File

SOURCE=.\src\friction.c
# End Source File
# Begin Source File

SOURCE=.\src\ghssolver.c
# End Source File
# Begin Source File

SOURCE=.\src\gravity.c
# End Source File
# Begin Source File

SOURCE=.\src\loadmatrix.c
# End Source File
# Begin Source File

SOURCE=.\src\rockbolts.c
# End Source File
# Begin Source File

SOURCE=.\src\sparsestorage.c
# End Source File
# Begin Source File

SOURCE=.\src\timestep.c
# End Source File
# Begin Source File

SOURCE=.\src\transfercontact.c
# End Source File
# End Group
# Begin Group "geometry"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\blocks.c
# End Source File
# Begin Source File

SOURCE=.\src\geomdriver.c
# End Source File
# Begin Source File

SOURCE=.\src\geometrydata.c
# End Source File
# Begin Source File

SOURCE=.\src\lns.c
# End Source File
# End Group
# Begin Source File

SOURCE=.\src\utils.c
# End Source File
# End Group
# Begin Group "gui"

# PROP Default_Filter ""
# Begin Group "dialogs"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\win32gui\analysisdialog.c
# End Source File
# Begin Source File

SOURCE=.\src\apwedge.c
# End Source File
# Begin Source File

SOURCE=.\src\win32gui\drawdialog3.c
# End Source File
# Begin Source File

SOURCE=.\src\filebrowse.c
# End Source File
# Begin Source File

SOURCE=.\src\geomdlg.c
# End Source File
# Begin Source File

SOURCE=.\src\unitdialog.c
# End Source File
# End Group
# Begin Source File

SOURCE=.\src\win32gui\drawtitle.c
# End Source File
# Begin Source File

SOURCE=.\src\win32gui\dropfiles.c
# End Source File
# Begin Source File

SOURCE=.\src\handlehelp.c
# End Source File
# Begin Source File

SOURCE=.\src\win32gui\handlemainmenu.c
# End Source File
# Begin Source File

SOURCE=.\src\print.c
# End Source File
# Begin Source File

SOURCE=.\src\win32gui\profile.c
# End Source File
# Begin Source File

SOURCE=.\src\replay.c
# End Source File
# Begin Source File

SOURCE=.\src\win32gui\statusbar.c
# End Source File
# Begin Source File

SOURCE=.\src\win32gui\testpropsheet.c
# End Source File
# Begin Source File

SOURCE=.\src\win32gui\toolbar.c
# End Source File
# Begin Source File

SOURCE=.\src\visedhandle.c
# End Source File
# Begin Source File

SOURCE=.\src\win32gui\win32utils.c
# End Source File
# Begin Source File

SOURCE=.\src\win32gui\wingraph.c
# End Source File
# Begin Source File

SOURCE=.\src\win32gui\winmain.c
# End Source File
# End Group
# Begin Group "utilities"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\bolt.c
# End Source File
# Begin Source File

SOURCE=.\src\datalog.c
# End Source File
# Begin Source File

SOURCE=.\include\Dda.rc
# End Source File
# Begin Source File

SOURCE=.\src\ddadlist.c
# End Source File
# Begin Source File

SOURCE=.\src\ddamemory.c
# End Source File
# Begin Source File

SOURCE=.\src\inpoly.c
# End Source File
# Begin Source File

SOURCE=.\src\joint.c
# End Source File
# Begin Source File

SOURCE=.\src\loadpoint.c
# End Source File
# Begin Source File

SOURCE=.\include\resource.h
# End Source File
# Begin Source File

SOURCE=.\src\timehistory.c
# End Source File
# End Group
# Begin Group "graphics"

# PROP Default_Filter "*.ico *.bmp"
# Begin Source File

SOURCE=.\resources\blockmaterials.ico
# End Source File
# Begin Source File

SOURCE=.\resources\contact.ico
# End Source File
# Begin Source File

SOURCE=.\resources\dda.ico
# End Source File
# Begin Source File

SOURCE=.\resources\ddawin.bmp
# End Source File
# Begin Source File

SOURCE=.\resources\earthquake.ico
# End Source File
# Begin Source File

SOURCE=.\resources\execute.ico
# End Source File
# Begin Source File

SOURCE=.\resources\flag.ico
# End Source File
# Begin Source File

SOURCE=.\resources\geom.ico
# End Source File
# Begin Source File

SOURCE=.\src\graphics.c
# End Source File
# Begin Source File

SOURCE=.\resources\gravity.ico
# End Source File
# Begin Source File

SOURCE=.\resources\greenledoff.ico
# End Source File
# Begin Source File

SOURCE=.\resources\greenledon.ico
# End Source File
# Begin Source File

SOURCE=.\resources\jointtype.ico
# End Source File
# Begin Source File

SOURCE=.\resources\loadpoint.ico
# End Source File
# Begin Source File

SOURCE=.\resources\logoff.ico
# End Source File
# Begin Source File

SOURCE=.\resources\logon.ico
# End Source File
# Begin Source File

SOURCE=.\resources\redledoff.ico
# End Source File
# Begin Source File

SOURCE=.\resources\redledon.ico
# End Source File
# Begin Source File

SOURCE=.\resources\refresh.ico
# End Source File
# Begin Source File

SOURCE=.\resources\rockbolt.ico
# End Source File
# Begin Source File

SOURCE=.\resources\stopwatch.ico
# End Source File
# Begin Source File

SOURCE=.\resources\Toolbar.bmp
# End Source File
# Begin Source File

SOURCE=.\resources\yellowledoff.ico
# End Source File
# Begin Source File

SOURCE=.\resources\yellowledon.ico
# End Source File
# End Group
# Begin Group "testcode"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\testcode.c
# End Source File
# End Group
# Begin Group "I/O"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\analysisddaml.c
# End Source File
# Begin Source File

SOURCE=.\src\analysisreader.c
# End Source File
# Begin Source File

SOURCE=.\src\blockhandler.c
# End Source File
# Begin Source File

SOURCE=.\src\ddaml.c
# End Source File
# Begin Source File

SOURCE=.\src\display.c
# End Source File
# Begin Source File

SOURCE=.\src\geomddaml.c
# End Source File
# Begin Source File

SOURCE=.\src\geomreader.c
# End Source File
# Begin Source File

SOURCE=.\src\inputfiles.c
# End Source File
# Begin Source File

SOURCE=.\src\loghtml.c
# End Source File
# Begin Source File

SOURCE=.\src\postprocess.c
# End Source File
# End Group
# Begin Source File

SOURCE=.\include\analysisdata.h
# End Source File
# Begin Source File

SOURCE=.\src\contacts.c
# End Source File
# Begin Source File

SOURCE=.\include\contacts.h
# End Source File
# Begin Source File

SOURCE=.\src\dda.c
# End Source File
# Begin Source File

SOURCE=.\include\dda.h
# End Source File
# Begin Source File

SOURCE=.\src\dda_error.c
# End Source File
# Begin Source File

SOURCE=.\include\geometrydata.h
# End Source File
# Begin Source File

SOURCE=.\include\gravity.h
# End Source File
# Begin Source File

SOURCE=.\include\postprocess.h
# End Source File
# Begin Source File

SOURCE=.\include\timehistory.h
# End Source File
# Begin Source File

SOURCE=.\include\utils.h
# End Source File
# End Target
# End Project
