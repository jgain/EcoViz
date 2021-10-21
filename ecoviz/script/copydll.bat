cls

:: x64 Debug
XCOPY /Y "%QT5DIR%\bin\Qt5Cored.dll"	 					"..\out\Debug\"
XCOPY /Y "%QT5DIR%\bin\Qt5Guid.dll" 						"..\out\Debug\"
XCOPY /Y "%QT5DIR%\bin\Qt5Widgetsd.dll" 					"..\out\Debug\"
XCOPY /Y "%QT5DIR%\bin\Qt5Opengld.dll" 					"..\out\Debug\"
XCOPY /Y "%QT5DIR%\bin\Qt5Chartsd.dll" 					"..\out\Debug\"
XCOPY /Y "..\windows_extern_packages\glew\glew32.dll" 				"..\out\Debug\"
XCOPY /Y "..\windows_extern_packages\sqlite3\sqlite3.dll" 			"..\out\Debug\"

:: x64 Release
XCOPY /Y "%QT5DIR%\bin\Qt5Core.dll"	 					"..\out\Release\"
XCOPY /Y "%QT5DIR%\bin\Qt5Gui.dll" 						"..\out\Release\"
XCOPY /Y "%QT5DIR%\bin\Qt5Widgets.dll" 					"..\out\Release\"
XCOPY /Y "%QT5DIR%\bin\Qt5Opengl.dll" 						"..\out\Release\"
XCOPY /Y "%QT5DIR%\bin\Qt5Charts.dll" 						"..\out\Release\"
XCOPY /Y "..\windows_extern_packages\glew\glew32.dll" 				"..\out\Release\"
XCOPY /Y "..\windows_extern_packages\sqlite3\sqlite3.dll" 			"..\out\Release\"