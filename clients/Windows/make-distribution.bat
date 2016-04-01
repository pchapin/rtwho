@echo off

rem This script could use more error checking, etc.

set CRT_ASSEMBLIES=c:\Program Files\Microsoft Visual Studio 9.0\VC\redist\x86

cd Distribution
rem The following delete command is dangerous. What if the cd above fails?
rem del /Q *.*
copy ..\..\..\Release\RTWho.exe .
copy ..\..\RTWconfig.cfg .
copy "%XERCESCROOT%\bin\xerces-c_3_0.dll" .
copy "%CRT_ASSEMBLIES%\Microsoft.VC90.CRT" .

set CRT_ASSEMBLIES=

zip -r RTWho-0.0.zip *
