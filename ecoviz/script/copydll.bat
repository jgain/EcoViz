cls

:: x64 Debug

XCOPY /Y "..\windows_extern_packages\glew\glew32.dll" 				"..\out\Debug\"
XCOPY /Y "..\windows_extern_packages\sqlite3\sqlite3.dll" 			"..\out\Debug\"

:: x64 Release

XCOPY /Y "..\windows_extern_packages\glew\glew32.dll" 				"..\out\Release\"
XCOPY /Y "..\windows_extern_packages\sqlite3\sqlite3.dll" 			"..\out\Release\"

:: Deploy
%QTDIR64%\bin\windeployqt.exe --release ..\Out\Release\ecoviz.exe
%QTDIR64%\bin\windeployqt.exe ..\Out\Debug\ecoviz.exe