# Microsoft Developer Studio Project File - Name="console" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=console - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "console.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "console.mak" CFG="console - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "console - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "console - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "console - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib libopt.lib libxml.lib wsock32.lib /nologo /subsystem:console /machine:I386 /out:"Release/dda.exe"

!ELSEIF  "$(CFG)" == "console - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "../include" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib liboptd.lib libxmld.lib wsock32.lib /nologo /subsystem:console /debug /machine:I386 /out:"Debug/dda.exe" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "console - Win32 Release"
# Name "console - Win32 Debug"
# Begin Group "src"

# PROP Default_Filter ""
# Begin Group "analysis"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\src\analysisdriver.c
# End Source File
# Begin Source File

SOURCE=..\src\blocks.c
# End Source File
# Begin Source File

SOURCE=..\src\combineddf.c
# End Source File
# Begin Source File

SOURCE=..\src\contact.c
# End Source File
# Begin Source File

SOURCE=..\src\ddamemory.c
# End Source File
# Begin Source File

SOURCE=..\src\ghssolver.c
# End Source File
# Begin Source File

SOURCE=..\src\gravity.c
# End Source File
# Begin Source File

SOURCE=..\src\loadmatrix.c
# End Source File
# Begin Source File

SOURCE=..\src\rockbolts.c
# End Source File
# Begin Source File

SOURCE=..\src\utils.c
# End Source File
# End Group
# Begin Group "geometry"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\src\blockreader.c
# End Source File
# Begin Source File

SOURCE=..\src\geomdriver.c
# End Source File
# Begin Source File

SOURCE=..\src\lns.c
# End Source File
# End Group
# Begin Source File

SOURCE=..\src\analysisreader.c
# End Source File
# Begin Source File

SOURCE=..\src\analysisreader2.c
# End Source File
# Begin Source File

SOURCE=..\src\ddamain.c
# End Source File
# Begin Source File

SOURCE=..\src\errorhandler.c
# End Source File
# Begin Source File

SOURCE=..\src\exportfig.c
# End Source File
# Begin Source File

SOURCE=..\src\geomreader.c
# End Source File
# Begin Source File

SOURCE=..\src\geomreader2.c
# End Source File
# Begin Source File

SOURCE=..\src\inputfiles.c
# End Source File
# Begin Source File

SOURCE=..\src\postprocess.c
# End Source File
# Begin Source File

SOURCE=..\src\sparsestorage.c
# End Source File
# Begin Source File

SOURCE=..\src\testcode.c
# End Source File
# Begin Source File

SOURCE=..\src\timestep.c
# End Source File
# End Group
# Begin Group "blas"

# PROP Default_Filter ""
# End Group
# Begin Group "blaslapack"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\src\dasum.c
# End Source File
# Begin Source File

SOURCE=..\src\daxpy.c
# End Source File
# Begin Source File

SOURCE=..\src\dcopy.c
# End Source File
# Begin Source File

SOURCE=..\src\ddot.c
# End Source File
# Begin Source File

SOURCE=..\src\dgemm.c
# End Source File
# Begin Source File

SOURCE=..\src\dgemv.c
# End Source File
# Begin Source File

SOURCE=..\src\dger.c
# End Source File
# Begin Source File

SOURCE=..\src\dgetf2.c
# End Source File
# Begin Source File

SOURCE=..\src\dgetrf.c
# End Source File
# Begin Source File

SOURCE=..\src\dgetri.c
# End Source File
# Begin Source File

SOURCE=..\src\display.c
# End Source File
# Begin Source File

SOURCE=..\src\dlabad.c
# End Source File
# Begin Source File

SOURCE=..\src\dlacon.c
# End Source File
# Begin Source File

SOURCE=..\src\dlacpy.c
# End Source File
# Begin Source File

SOURCE=..\src\dlamch.c
# End Source File
# Begin Source File

SOURCE=..\src\dlange.c
# End Source File
# Begin Source File

SOURCE=..\src\dlansy.c
# End Source File
# Begin Source File

SOURCE=..\src\dlaqsy.c
# End Source File
# Begin Source File

SOURCE=..\src\dlassq.c
# End Source File
# Begin Source File

SOURCE=..\src\dlaswp.c
# End Source File
# Begin Source File

SOURCE=..\src\dlatrs.c
# End Source File
# Begin Source File

SOURCE=..\src\dlauu2.c
# End Source File
# Begin Source File

SOURCE=..\src\drscl.c
# End Source File
# Begin Source File

SOURCE=..\src\dscal.c
# End Source File
# Begin Source File

SOURCE=..\src\dswap.c
# End Source File
# Begin Source File

SOURCE=..\src\dsymv.c
# End Source File
# Begin Source File

SOURCE=..\src\dsyrk.c
# End Source File
# Begin Source File

SOURCE=..\src\dtrmm.c
# End Source File
# Begin Source File

SOURCE=..\src\dtrmv.c
# End Source File
# Begin Source File

SOURCE=..\src\dtrsm.c
# End Source File
# Begin Source File

SOURCE=..\src\dtrsv.c
# End Source File
# Begin Source File

SOURCE=..\src\dtrti2.c
# End Source File
# Begin Source File

SOURCE=..\src\dtrtri.c
# End Source File
# Begin Source File

SOURCE=..\src\idamax.c
# End Source File
# Begin Source File

SOURCE=..\src\ilaenv.c
# End Source File
# Begin Source File

SOURCE=..\src\inpoly.c
# End Source File
# Begin Source File

SOURCE=..\src\loghtml.c
# End Source File
# Begin Source File

SOURCE=..\src\lsame.c
# End Source File
# Begin Source File

SOURCE=..\src\xerbla.c
# End Source File
# End Group
# Begin Source File

SOURCE=..\src\analysisdata.c
# End Source File
# Begin Source File

SOURCE=..\src\analysisddaml.c
# End Source File
# Begin Source File

SOURCE=..\src\blockhandler.c
# End Source File
# Begin Source File

SOURCE=..\src\congrad.c
# End Source File
# Begin Source File

SOURCE=..\src\consoleinterface.c
# End Source File
# Begin Source File

SOURCE=..\src\ddadlist.c
# End Source File
# Begin Source File

SOURCE=..\src\eqlex.c
# End Source File
# Begin Source File

SOURCE=..\src\geomddaml.c
# End Source File
# Begin Source File

SOURCE=..\src\geometrydata.c
# End Source File
# Begin Source File

SOURCE=..\src\norms.c
# End Source File
# End Target
# End Project
