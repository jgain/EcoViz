To use the Visual Studio projet on Windows, you must :
- Install Qt5 (including QtCharts) and set the environment variable "QT5DIR" with the path to the Qt5 installation folder as value
- Launch the "copydll.bat" script located in the "script" folder
- Open the Visual Studio solution and install (with the NuGet package manager) the following libraries :
	* boost
	* boost_serialization-vc142 ("-vc142" for Visual Studio 2019)
	* glm
- In the project properties, go in "Debugging" and set (for both Debug and Release configuration) :
	* "Command Arguments" to "..\..\data\test"
	* "Working Directory" to "$(ProjectDir)\out"