UBUNTU INSTALLATION:
--------------------
These are requirements for Ubuntu. In each case, I've listed where to get the software 
for Ubuntu 18.10. For newer versions of Ubuntu you might not need all the PPAs.

CMake 2.8.7+, make, automake 1.9, pkg-config, QT5.7+, Boost1.49+, SQLite3

sudo apt-get install cmake automake1.9 pkg-config libglm-dev qtbase5-dev libqt5charts5-dev libboost-all-dev libglew-dev qtcreator sqlite3 sqlitebrowser

To install a particular version of g++ (in this case v7, note that version 9 does not work):

sudo add-apt-repository ppa:ubuntu-toolchain-r/test
sudo apt install gcc-8 g++-8
then make sure to set the correct version in the build script (buildecoviz.sh)

Compiling and Executing
-----------------------

There is a build script that you can run from the ecoviz subdirectory called buildecoviz.sh. 

Alternatively, once all the requirements are running, create a subdirectory called build (or
anything starting with build - you can have multiple build directories), switch
into it, and run

cmake <options> ..

Some useful options
-DCMAKE_CXX_COMPILER=g++-4.8          (to force a specific compiler)
-DCMAKE_BUILD_TYPE=Debug|Release|RelWithDebInfo  (to select build type, only choose one option here)
-DCMAKE_CXX_FLAGS_DEBUG="-gdwarf-3"   (helps older versions of GDB interpret debug symbols)
-DCMAKE_CXX_FLAGS_RELWITHDEBINFO="-O3 -DNDEBUG -g -gdwarf-3" (similar)
-DCMAKE_CXX_FLAGS="-fPIC" (if the compilation complains about position independent code)

Then run make to build. cmake options are sticky. 

The executable is ./viz/ecoviz. The system must be run from the build directory because there are some relative paths.

To run the visualizer with test data execute: ./viz/ecoviz ../../data/test

GUI
---
The visualizer is divided horizontally into two scenes (these happen at the moment to be identical but that need not necessarily be the case). Vertically there are four panels:
1) The transect view at the top, which is initially grey becuase no transect has been selected. Once a transect has been selected (see below) the right mouse button can be used to pan and the middle scroll wheel to zoom in and out.
2) Next, a 3D scene view. This can be rotated by holding down the right mouse button and moving the cursor. Zooming is with the mouse wheel. Double click with the right mouse button to change the focal point on the terrain. In order to select a transect click using <ctrl><leftmoustbutton> on two points of the terrain. These determine the start and end point of the transect, which will now appear in the transect window. To widen and narrow the transect use <ctl><mousewheel>. 
3) Below this is the timeline media bar with controls for backtracking a time-step, play/pause, and advancing a time-step.
4) The final window shows a graph with a bar that corresponds to the current timestep. There are no direct interactions possible in this window at the moment.

The are also two view panels accessible from the view menu: the first controls the terrain viewing and the second controls the display of plants, such as selecting which species are currently displayed. I suggest ignoring these for the moment, except perhaps to experiment with the smoothing level in the plant view.


