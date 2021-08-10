To use the Visual Studio projet on Windows, you must :
- Install Qt5 (including QtCharts) and set the environment variable "QTDIR64" with the path to the Qt5 installation folder as value
- Launch the "copydll.bat" script located in the "script" folder
- Open the Visual Studio solution and install (with the NuGet package manager) the following libraries :
	* boost
	* boost_serialization-vc142 ("-vc142" for Visual Studio 2019)
	* glm